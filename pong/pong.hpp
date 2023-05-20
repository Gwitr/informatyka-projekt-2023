#ifndef GAMES_PONG_PONG_HPP
#define GAMES_PONG_PONG_HPP

#include "../scene.hpp"
#include "object.hpp"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

class scoreboard : public object
{
    TTF_Font *const m_font;
    int m_score1 = 0, m_score2 = 0;
    void update_score_text();
public:
    scoreboard(SDL_Renderer *renderer, TTF_Font *font, int startX, int startY);
    scoreboard(const scoreboard &) = delete;  // Make sure to forbid copying as I'm too lazy to implement it
    scoreboard(scoreboard &&other);
    virtual void draw() const override;
    ~scoreboard();
    void addPoint(int player);
    virtual bool can_collide() const override;
};

class pong_scene final : public scene
{
    TTF_Font *m_font;
    SDL_Texture *m_tex1, *m_tex2;
    std::vector<std::unique_ptr<object>> m_objects;
    scoreboard *m_scores;

public:
    pong_scene(scenes &scenes, SDL_Renderer *renderer, bool hockeyMode);
    ~pong_scene();
    void update(float deltaTime) override;
    void draw() const override;
    void on_event(const SDL_Event &event) override;
};

extern uint32_t givePointEventType;

#endif  // GAMES_PONG_PONG_HPP
