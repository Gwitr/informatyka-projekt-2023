#include "pong.hpp"
#include <cmath>

uint32_t givePointEventType;

class goal : public object
{
    const int m_width, m_holeSize;
public:
    goal(SDL_Renderer *renderer, int x, int width, int holeSize)
        : object{renderer, NULL, x, 0}, m_width{width}, m_holeSize{holeSize}
    {
        m_texWidth = width;
        m_texHeight = SCREEN_HEIGHT;
    }

    virtual std::vector<SDL_Rect> get_collision_areas() const override
    {
        return std::vector {
            SDL_Rect { (int)m_x, (int)m_y, m_width, (SCREEN_HEIGHT - m_holeSize) / 2 },
            SDL_Rect { (int)m_x, (int)(m_y + (SCREEN_HEIGHT + m_holeSize) / 2), m_width, (SCREEN_HEIGHT - m_holeSize) / 2 }
        };
    }

    virtual void draw() const override
    {
        sdlCall(SDL_SetRenderDrawColor)(m_renderer, 0, 0, 0, 255);
        SDL_Rect rect = { (int)m_x, (int)m_y, m_width, (SCREEN_HEIGHT - m_holeSize) / 2 };
        SDL_RenderFillRect(m_renderer, &rect);
        rect.y += (SCREEN_HEIGHT + m_holeSize) / 2;
        SDL_RenderFillRect(m_renderer, &rect);
    }
};

class paddle : public object {
    const int m_upKey, m_downKey;
    const float m_speed;

public:
    float m_verticalSpeed = 0.0f;

    paddle(SDL_Renderer *renderer, SDL_Texture *tex, int startX, int startY, float speed, int upKey, int downKey)
        : object{renderer, tex, startX, startY}, m_upKey{upKey}, m_downKey{downKey}, m_speed{speed}
    {}

    virtual void update(float deltaTime, const std::vector<std::unique_ptr<object>> &others) override
    {
        float prevY = m_y;

        float movement = 0.0f;

        if (m_keys[m_upKey])
            movement = -m_speed * deltaTime;
        if (m_keys[m_downKey])
            movement = m_speed * deltaTime;
        
        m_y += movement;

        for (auto &other : others) {
            goal *goalpost = dynamic_cast<goal*>(other.get());
            if (!goalpost)
                continue;
            if (aabb_overlap_all(get_collision_areas(), goalpost->get_collision_areas()))
            {
                m_y -= movement;
                break;
            }
        }

        // Clamp the Y value
        if (m_y > m_maxY - m_texHeight)
            m_y = m_maxY - m_texHeight;
        if (m_y < 0)
            m_y = 0;
        
        m_verticalSpeed = (m_y - prevY) / deltaTime;
    }
};

class ball : public object
{
    const float m_speed;
    float m_dirX, m_dirY;

    std::vector<object*> m_collided;

public:
    ball(SDL_Renderer *renderer, SDL_Texture *tex, int startX, int startY, float speed)
        : object{renderer, tex, startX, startY}, m_speed{speed}, m_dirX{1.0f}, m_dirY{0.0f}
    {}

    virtual void reset() override
    {
        object::reset();
        m_dirX = 1.0f;
        m_dirY = 0.0f;
    }

    virtual void update(float deltaTime, const std::vector<std::unique_ptr<object>> &others) override
    {
        // Attempt to move
        m_x += m_dirX * m_speed * deltaTime;
        m_y += m_dirY * m_speed * deltaTime;
        for (auto &obj : others) {
            if (obj.get() == this)
                continue;
            if (!obj->can_collide())
                continue;

            if (aabb_overlap_all(get_collision_areas(), obj->get_collision_areas())) {
                if (std::find(m_collided.begin(), m_collided.end(), obj.get()) == m_collided.end()) {
                    m_collided.push_back(obj.get());
                    m_x -= m_dirX * m_speed * deltaTime;
                    m_dirX = -m_dirX;

                    paddle *p = dynamic_cast<paddle*>(obj.get());
                    if (p != NULL) {
                        // Bounce from paddle (not physically accurate in the slightest)
                        m_dirY += p->m_verticalSpeed * 0.003f;
                        float length = sqrtf(m_dirX * m_dirX + m_dirY * m_dirY);
                        m_dirX /= length;
                        m_dirY /= length;
                    }

                    break;
                }

            } else if (auto iter = std::find(m_collided.begin(), m_collided.end(), obj.get()); iter != m_collided.end()) {
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
            sdlCall(SDL_PushEvent)(&user_event);
        }
        if (m_x > m_maxX - m_texWidth) {
            SDL_Event user_event;
            user_event.type = givePointEventType;
            user_event.user.data1 = (void*)0;
            sdlCall(SDL_PushEvent)(&user_event);
        }
    }
};


void scoreboard::update_score_text()
{
    update_text(m_renderer, m_font, std::to_string(m_score1) + " - " + std::to_string(m_score2), { 0, 0, 0, 255 }, m_texture, m_texWidth, m_texHeight);
}

scoreboard::scoreboard(SDL_Renderer *renderer, TTF_Font *font, int startX, int startY)
    : object{renderer, NULL, startX, startY}, m_font{font}
{
    update_score_text();
}

scoreboard::scoreboard(scoreboard &&other)
    : object{other}, m_font{other.m_font}
{
    m_score1 = other.m_score1;
    m_score2 = other.m_score2;
    other.m_texture = NULL;
}

void scoreboard::draw() const
{
    // Draw the text anchored to the middle
    SDL_Rect srcrect = { 0, 0, m_texWidth, m_texHeight };
    SDL_Rect dstrect = { (int)(m_x - m_texWidth / 2.0f), (int)m_y, m_texWidth, m_texHeight };
    sdlCall(SDL_RenderCopy)(m_renderer, m_texture, &srcrect, &dstrect);
}

scoreboard::~scoreboard()
{
    // We own the texture, so make sure to destroy it
    SDL_DestroyTexture(m_texture);
}

void scoreboard::addPoint(int player)
{
    if (player == 0)
        ++m_score1;
    else
        ++m_score2;

    update_score_text();
}

bool scoreboard::can_collide() const
{
    return false;
}


pong_scene::pong_scene(scenes &scenes, SDL_Renderer *renderer, bool hockeyMode)
    : scene{scenes, renderer}
{
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
    sdlCall(SDL_DestroyTexture)(m_tex1);
    sdlCall(SDL_DestroyTexture)(m_tex2);

    sdlCall(TTF_CloseFont)(m_font);
}

void pong_scene::update(float deltaTime)
{
    for (auto &object : m_objects)
        object->update(deltaTime, m_objects);
}

void pong_scene::draw() const
{
    sdlCall(SDL_SetRenderDrawColor)(m_renderer, 255, 255, 255, 255);
    sdlCall(SDL_RenderClear)(m_renderer);

    for (auto &object : m_objects)
        object->draw();
}

void pong_scene::on_event(const SDL_Event &event)
{
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            m_scenes.pop_scene();
        } else {
            for (auto &object : m_objects) {
                object->keyDown(event.key.keysym.sym);
            }
        }

    } else if (event.type == SDL_KEYUP) {
        for (auto &object : m_objects)
            object->keyUp(event.key.keysym.sym);

    } else if (event.type == givePointEventType) {
        // Reset the world
        for (auto &object : m_objects)
            object->reset();

        // Update the score
        m_scores->addPoint((intptr_t)event.user.data1);
    }
}
