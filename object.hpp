#ifndef GAMES_OBJECT_HPP
#define GAMES_OBJECT_HPP

#include <unordered_map>     // std::unordered_map
#include <vector>            // std::vector
#include <memory>            // std::unique_ptr
#include "utils.hpp"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

class object {
protected:
    SDL_Renderer *const m_renderer;
    SDL_Texture *m_texture;

    int m_texWidth, m_texHeight;

    float m_x, m_y;

    const int m_startX, m_startY;

    int m_maxX, m_maxY;

    std::unordered_map<int, bool> m_keys;
public:
    object(SDL_Renderer *renderer, SDL_Texture *tex, int startX, int startY);
    virtual ~object();

    virtual bool can_collide() const;
    virtual void reset();
    bool aabb_overlap(const object &other) const;
    virtual void keyDown(int key);
    virtual void keyUp(int key);
    virtual void draw() const;
    virtual void update(float deltaTime, const std::vector<std::unique_ptr<object>> &others);
};

#endif  // GAMES_OBJECT_HPP
