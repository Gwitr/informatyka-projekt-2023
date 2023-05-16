// g++ -O2 -Wall -Wextra -pedantic -std=c++20 -o main main.cpp object.cpp utils.cpp -lSDL2 -lSDL2_image -lSDL2_ttf -lm

#include <cstdint>           // uint32_t, intptr_t
#include <cmath>             // sqrtf
#include <algorithm>         // std::find
#include <iostream>          // std::cout
#include <string>            // std::string
#include <memory>            // std::unique_ptr
#include <vector>            // std::vector
#include <initializer_list>  // std::initializer_list

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "utils.hpp"
#include "object.hpp"

uint32_t givePointEventType;

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
        (void)others;

        float prevY = m_y;

        if (m_keys[m_upKey])
            m_y -= m_speed * deltaTime;
        if (m_keys[m_downKey])
            m_y += m_speed * deltaTime;

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

            if (aabb_overlap(*obj)) {
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

class scoreboard : public object
{
    TTF_Font *const m_font;

    int m_score1 = 0, m_score2 = 0;

    void update_score_text()
    {
        update_text(m_renderer, m_font, std::to_string(m_score1) + " - " + std::to_string(m_score2), { 0, 0, 0, 255 }, m_texture, m_texWidth, m_texHeight);
    }

public:
    scoreboard(SDL_Renderer *renderer, TTF_Font *font, int startX, int startY)
        : object{renderer, NULL, startX, startY}, m_font{font}
    {
        update_score_text();
    }

    scoreboard(const scoreboard &) = delete;  // Make sure to forbid copying as I'm too lazy to implement it
    scoreboard(scoreboard &&other)
        : object{other}, m_font{other.m_font}
    {
        m_score1 = other.m_score1;
        m_score2 = other.m_score2;
        other.m_texture = NULL;
    }

    virtual void draw() const override
    {
        // Draw the text anchored to the middle
        SDL_Rect srcrect = { 0, 0, m_texWidth, m_texHeight };
        SDL_Rect dstrect = { (int)(m_x - m_texWidth / 2.0f), (int)m_y, m_texWidth, m_texHeight };
        sdlCall(SDL_RenderCopy)(m_renderer, m_texture, &srcrect, &dstrect);
    }

    ~scoreboard()
    {
        // We own the texture, so make sure to destroy it
        SDL_DestroyTexture(m_texture);
    }

    void addPoint(int player)
    {
        if (player == 0)
            ++m_score1;
        else
            ++m_score2;

        update_score_text();
    }

    virtual bool can_collide() const override
    {
        return false;
    }
};

int main()
{
    SDL_Window *window;
    SDL_Renderer *renderer;

    sdlCall(SDL_Init)(SDL_INIT_EVERYTHING);
    sdlCall(IMG_Init)(IMG_INIT_JPG | IMG_INIT_PNG);
    sdlCall(TTF_Init)();
    {
        TTF_Font *font = sdlCall(TTF_OpenFont)("Terminus.ttf", 32);

        sdlCall(SDL_CreateWindowAndRenderer)(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
        sdlCall(SDL_SetWindowTitle)(window, "Bouncy games");

        givePointEventType = sdlCall(SDL_RegisterEvents)(1);

        SDL_Texture *tex1 = sdlCall(IMG_LoadTexture)(renderer, "paddle.png");
        SDL_Texture *tex2 = sdlCall(IMG_LoadTexture)(renderer, "ball.png");

        std::vector<std::unique_ptr<object>> objects;
        // Add player paddles to the game
        objects.emplace_back(new paddle{renderer, tex1, 25, SCREEN_HEIGHT / 2 - 64, 300.0f, SDLK_q, SDLK_a});
        objects.emplace_back(new paddle{renderer, tex1, SCREEN_WIDTH - 25 - 32, SCREEN_HEIGHT / 2 - 64, 300.0f, SDLK_o, SDLK_l});

        // Add ball to the game
        objects.emplace_back(new ball{renderer, tex2, SCREEN_WIDTH / 2 - 16, SCREEN_HEIGHT / 2 - 16, 300.0f});

        // Add scoreboard to the game
        auto &scores = dynamic_cast<scoreboard&>(*objects.emplace_back(new scoreboard{renderer, font, SCREEN_WIDTH / 2, 0}));

        uint64_t lastFrameTicks = sdlCall(SDL_GetTicks64)();
        float deltaTime = 0.016f;
        bool running = true;

        sdlCall(SDL_SetRenderDrawColor)(renderer, 255, 255, 255, 255);

        while (running) {
            // Handle events
            SDL_Event event;
            while (sdlCall(SDL_PollEvent)(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;

                } else if (event.type == SDL_KEYDOWN) {
                    for (auto &object : objects)
                        object->keyDown(event.key.keysym.sym);

                } else if (event.type == SDL_KEYUP) {
                    for (auto &object : objects)
                        object->keyUp(event.key.keysym.sym);

                } else if (event.type == givePointEventType) {
                    // Reset the world
                    for (auto &object : objects)
                        object->reset();

                    // Update the score
                    scores.addPoint((intptr_t)event.user.data1);
                }
            }

            // Update
            for (auto &object : objects)
                object->update(deltaTime, objects);

            // Render
            sdlCall(SDL_RenderClear)(renderer);
            for (auto &object : objects)
                object->draw();
            sdlCall(SDL_RenderPresent)(renderer);

            // Lock to 60 FPS
            uint64_t currentFrameTicks = sdlCall(SDL_GetTicks64)();
            deltaTime = (currentFrameTicks - lastFrameTicks) / 1000.0f;
            if (deltaTime < 0.016f) {
                sdlCall(SDL_Delay)(16 - 1000 * deltaTime);
            }
            deltaTime = (sdlCall(SDL_GetTicks64)() - lastFrameTicks) / 1000.0f;
            lastFrameTicks = currentFrameTicks;
        }

        sdlCall(SDL_DestroyTexture)(tex1);
        sdlCall(SDL_DestroyTexture)(tex2);

        sdlCall(TTF_CloseFont)(font);
    }

    sdlCall(TTF_Quit)();
    sdlCall(IMG_Quit)();
    sdlCall(SDL_Quit)();

    return 0;
}
