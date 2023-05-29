#include "pong.hpp"
#include <cmath>

uint32_t givePointEventType;  // The definition for the extern givePointEventType variable

// An object that represents the hole in which the ball has to go into in hockey mode
class goal : public object
{
    const int m_width, m_holeSize;   // Customizable width and hole size, these properties hold those parameters
public:
    // The constructor -- it takes the x (no y, as the object spans the whole screen), the width, and the hole size
    goal(SDL_Renderer *renderer, int x, int width, int holeSize)
        : object{renderer, NULL, x, 0}, m_width{width}, m_holeSize{holeSize}
    {
        // A bit of a hack, we override the texture size to be the size of the bounding box of the goal
        m_texWidth = width;
        m_texHeight = SCREEN_HEIGHT;
    }

    // This function returns a list of axis-aligned bounding boxes for this object
    virtual std::vector<SDL_Rect> get_collision_areas() const override
    {
        // We have 2 bounding boxes -- one for the top part of the goal, and one for the bottom. That way, the ball can go through
        // the middle
        return std::vector {
            SDL_Rect { (int)m_x, (int)m_y, m_width, (SCREEN_HEIGHT - m_holeSize) / 2 },
            SDL_Rect { (int)m_x, (int)(m_y + (SCREEN_HEIGHT + m_holeSize) / 2), m_width, (SCREEN_HEIGHT - m_holeSize) / 2 }
        };
    }

    // We change the drawing logic of a default object. A goal object doesn't have a texture, it just draws black rectangles
    virtual void draw() const override
    {
        // Set current color to black
        sdlCall(SDL_SetRenderDrawColor)(m_renderer, 0, 0, 0, 255);
        // Draw top rectangle
        SDL_Rect rect = { (int)m_x, (int)m_y, m_width, (SCREEN_HEIGHT - m_holeSize) / 2 };
        SDL_RenderFillRect(m_renderer, &rect);
        // Draw bottom rectangle
        rect.y += (SCREEN_HEIGHT + m_holeSize) / 2;
        SDL_RenderFillRect(m_renderer, &rect);
    }
};

// Object that represents an in-game paddle
class paddle : public object {
    // Which keys are used to control this particular paddle
    const int m_upKey, m_downKey;
    const float m_speed;  // Paddle speed parameter

public:
    // The current vertical speed of the paddle, used to calculate ball deflection angle
    float m_verticalSpeed = 0.0f;

    // The constructor. It takes in the starting location of the paddle, its speed, as well as its controls.
    paddle(SDL_Renderer *renderer, SDL_Texture *tex, int startX, int startY, float speed, int upKey, int downKey)
        : object{renderer, tex, startX, startY}, m_upKey{upKey}, m_downKey{downKey}, m_speed{speed}
    {}

    // The update function is overriden, as the paddle has behavior that must run each frame.
    virtual void update(float deltaTime, const std::vector<std::unique_ptr<object>> &others) override
    {
        float prevY = m_y;
        float movement = 0.0f;

        // Updating the displacement of the paddle based on the keys currently held by the user
        if (m_keys[m_upKey])
            movement = -m_speed * deltaTime;
        if (m_keys[m_downKey])
            movement = m_speed * deltaTime;

        // Apply the evaluated displacement
        m_y += movement;

        // For every single object
        for (auto &other : others) {
            // If this object is not a goal object, then ignore it
            goal *goalpost = dynamic_cast<goal*>(other.get());
            if (!goalpost)
                continue;
            // If it is a goal object, then check if we overlap with it
            if (aabb_overlap_all(get_collision_areas(), goalpost->get_collision_areas()))
            {
                // If we do, then cancel our movement
                m_y -= movement;
                break;
            }
        }

        // Clamp the Y value
        if (m_y > m_maxY - m_texHeight)
            m_y = m_maxY - m_texHeight;
        if (m_y < 0)
            m_y = 0;

        // Figure out our current speed
        m_verticalSpeed = (m_y - prevY) / deltaTime;
    }
};

// An object representing a ball
class ball : public object
{
    // The (constant!) speed of the ball
    const float m_speed;
    // The current direction vector of the ball
    float m_dirX, m_dirY;

    // List of objects that we're currently colliding with
    std::vector<object*> m_collided;

public:
    // Constructor for the ball; we simply set the starting x and y positions, as well as the constant speed and starting direction
    ball(SDL_Renderer *renderer, SDL_Texture *tex, int startX, int startY, float speed)
        : object{renderer, tex, startX, startY}, m_speed{speed}, m_dirX{1.0f}, m_dirY{0.0f}
    {}

    // Reset requires custom logic for the ball -- we also need to reset its direction
    virtual void reset() override
    {
        object::reset();  // Call the original reset function
        // Reset the velocity of the ball to its original value
        m_dirX = 1.0f;
        m_dirY = 0.0f;
    }

    // The ball has behavior that must be ran each frame, and so, update is overriden
    virtual void update(float deltaTime, const std::vector<std::unique_ptr<object>> &others) override
    {
        // Move along the velocity vector
        m_x += m_dirX * m_speed * deltaTime;
        m_y += m_dirY * m_speed * deltaTime;
        // For every single object
        for (auto &obj : others) {
            // If said object is actually us, then ignore it
            if (obj.get() == this)
                continue;
            // If said object is incapable of collisions, then ignore it (Hack! This should be handled by get_collision_areas() returning an empty vector)
            if (!obj->can_collide())
                continue;

            // If the ball overlaps with that object...
            if (aabb_overlap_all(get_collision_areas(), obj->get_collision_areas())) {
                // And if the object isn't featured in the list of currently overlapping object...
                if (std::find(m_collided.begin(), m_collided.end(), obj.get()) == m_collided.end()) {
                    // ...then add it to that list,
                    m_collided.push_back(obj.get());
                    // and undo our X displacement for this frame, while also bouncing,
                    m_x -= m_dirX * m_speed * deltaTime;
                    m_dirX = -m_dirX;

                    // and if it's a paddle that we're colliding with (HACK)...
                    paddle *p = dynamic_cast<paddle*>(obj.get());
                    if (p != NULL) {
                        // ...then bounce from paddle. (not physically accurate in the slightest)
                        m_dirY += p->m_verticalSpeed * 0.003f;
                        float length = sqrtf(m_dirX * m_dirX + m_dirY * m_dirY);
                        m_dirX /= length;
                        m_dirY /= length;
                    }

                    break;
                }

            } else if (auto iter = std::find(m_collided.begin(), m_collided.end(), obj.get()); iter != m_collided.end()) {
                // This only runs if we're not colliding with an object but it's present in the colliding list; in that case, we remove it from the list
                m_collided.erase(iter);
            }
        }

        // Bounce off the top and bottom
        if (m_y > m_maxY - m_texHeight) {
            m_y = m_maxY - m_texHeight;
            m_dirY = -m_dirY;
        }
        if (m_y < 0) {
            m_y = 0;
            m_dirY = -m_dirY;
        }

        // Give points if the ball went off the side
        if (m_x < 0) {
            SDL_Event user_event;
            user_event.type = givePointEventType;
            user_event.user.data1 = (void*)1;
            // HACK! We should have a way to send events between objects, not this!
            sdlCall(SDL_PushEvent)(&user_event);
        }
        if (m_x > m_maxX - m_texWidth) {
            SDL_Event user_event;
            user_event.type = givePointEventType;
            user_event.user.data1 = (void*)0;
            // HACK! We should have a way to send events between objects, not this!
            sdlCall(SDL_PushEvent)(&user_event);
        }
    }
};

// Implementations of scoreboard methods

void scoreboard::update_score_text()
{
    // We just delegate to the update_text(...) helper function, formatting the score nicely
    update_text(m_renderer, m_font, std::to_string(m_score1) + " - " + std::to_string(m_score2), { 0, 0, 0, 255 }, m_texture, m_texWidth, m_texHeight);
}

// The constructor for a scoreboard object
scoreboard::scoreboard(SDL_Renderer *renderer, TTF_Font *font, int startX, int startY)
    : object{renderer, NULL, startX, startY}, m_font{font}  // Set the font used
{
    // Make sure to render the SDL_Texture for "0 - 0", as we will use it for drawing later
    update_score_text();
}

// The move constructor for a scoreboard object. Permits the moving of this object
scoreboard::scoreboard(scoreboard &&other)
    : object{other} /* Use the move constructor for object */, m_font{other.m_font} /* and just copy the font ID */
{
    // The new object (us) will copy the score from the old object (them)
    m_score1 = other.m_score1;
    m_score2 = other.m_score2;
    // We took ownership of the SDL_Texture, make sure the old object cannot use or deallocate it on accident anymore
    other.m_texture = NULL;
}

// A function to draw the scoreboard (it's a bit different, as the scoreboard is the only object anchored to its center. Also kind of a hack)
void scoreboard::draw() const
{
    // Draw the text anchored to the middle
    SDL_Rect srcrect = { 0, 0, m_texWidth, m_texHeight };
    SDL_Rect dstrect = { (int)(m_x - m_texWidth / 2.0f), (int)m_y, m_texWidth, m_texHeight };
    sdlCall(SDL_RenderCopy)(m_renderer, m_texture, &srcrect, &dstrect);
}

scoreboard::~scoreboard()
{
    // We own the texture, so make sure to destroy it (if we actually own it tho don't destroy it if it's NULL)
    if (m_texture)
        SDL_DestroyTexture(m_texture);
}

// Add a point to a player
void scoreboard::addPoint(int player)
{
    // No need for an array when there's just 2 players. I think. Not sure if this is best practice
    if (player == 0)
        ++m_score1;
    else
        ++m_score2;

    // Make sure to change the displayed text afterwards!
    update_score_text();
}

// Hacky way to exclude the scoreboard from acting as an object the ball can deflect from
bool scoreboard::can_collide() const
{
    return false;
}


// Implementations of functions for a pong_scene object

pong_scene::pong_scene(scenes &scenes, SDL_Renderer *renderer, bool hockeyMode)
    : scene{scenes, renderer}
{
    // Load the font and the textures used
    m_font = sdlCall(TTF_OpenFont)("Terminus.ttf", 32);
    m_tex1 = sdlCall(IMG_LoadTexture)(m_renderer, "paddle.png");
    m_tex2 = sdlCall(IMG_LoadTexture)(m_renderer, "ball.png");

    // Add player paddles to the game
    m_objects.emplace_back(new paddle{m_renderer, m_tex1, 25, SCREEN_HEIGHT / 2 - 64, 300.0f, SDLK_q, SDLK_a});
    m_objects.emplace_back(new paddle{m_renderer, m_tex1, SCREEN_WIDTH - 25 - 32, SCREEN_HEIGHT / 2 - 64, 300.0f, SDLK_o, SDLK_l});

    if (hockeyMode) {
        // Add more player paddles to the game
        m_objects.emplace_back(new paddle{m_renderer, m_tex1, 250, SCREEN_HEIGHT / 2 - 64, 300.0f, SDLK_w, SDLK_s});
        m_objects.emplace_back(new paddle{m_renderer, m_tex1, SCREEN_WIDTH - 250 - 32, SCREEN_HEIGHT / 2 - 64, 300.0f, SDLK_i, SDLK_k});
        m_objects.emplace_back(new paddle{m_renderer, m_tex1, 350, SCREEN_HEIGHT / 2 - 64, 300.0f, SDLK_e, SDLK_d});
        m_objects.emplace_back(new paddle{m_renderer, m_tex1, SCREEN_WIDTH - 350 - 32, SCREEN_HEIGHT / 2 - 64, 300.0f, SDLK_u, SDLK_j});
        // Add goals to the game
        m_objects.emplace_back(new goal{m_renderer, 0, 64, 320});
        m_objects.emplace_back(new goal{m_renderer, SCREEN_WIDTH - 64, 64, 320});
    }

    // Add ball to the game
    m_objects.emplace_back(new ball{m_renderer, m_tex2, SCREEN_WIDTH / 2 - 16, SCREEN_HEIGHT / 2 - 16, 300.0f});

    // Add scoreboard to the game
    m_scores = dynamic_cast<scoreboard*>(m_objects.emplace_back(new scoreboard{m_renderer, m_font, SCREEN_WIDTH / 2, 0}).get());
}

pong_scene::~pong_scene()
{
    // Free all the resources
    sdlCall(SDL_DestroyTexture)(m_tex1);
    sdlCall(SDL_DestroyTexture)(m_tex2);

    sdlCall(TTF_CloseFont)(m_font);
}

void pong_scene::update(float deltaTime)
{
    // For every object that exists
    for (auto &object : m_objects)
        // Update it
        object->update(deltaTime, m_objects);
}

void pong_scene::draw() const
{
    // Clear the screen
    // FIXME: This function should be able to tell its owner that it needn't draw any scenes below
    sdlCall(SDL_SetRenderDrawColor)(m_renderer, 255, 255, 255, 255);
    sdlCall(SDL_RenderClear)(m_renderer);

    // For every object that exists
    for (auto &object : m_objects)
        // Draw it
        object->draw();
}

void pong_scene::on_event(const SDL_Event &event)
{
    // If a key is pressed
    if (event.type == SDL_KEYDOWN) {
        // And that key is escape
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            // Then exit from the pong game, back into the main menu (or whatever other scene was underneath)
            m_scenes.pop_scene();
        } else {
            // We don't know what to do with this key, but the objects might, so for every object that exists
            for (auto &object : m_objects) {
                // Call its keyDown function
                object->keyDown(event.key.keysym.sym);
            }
        }

    // If a key is released
    } else if (event.type == SDL_KEYUP) {
        // Notify every object of the event
        for (auto &object : m_objects)
            object->keyUp(event.key.keysym.sym);

    // If a point is gained (HACK)
    } else if (event.type == givePointEventType) {
        // Reset the world
        for (auto &object : m_objects)
            object->reset();

        // Update the score
        m_scores->addPoint((intptr_t)event.user.data1);
    }
}
