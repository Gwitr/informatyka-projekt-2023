#ifndef GAMES_OBJECT_HPP
#define GAMES_OBJECT_HPP

#include <unordered_map>     // std::unordered_map
#include <vector>            // std::vector
#include <memory>            // std::unique_ptr
#include "utils.hpp"
#include <iostream>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

bool aabb_overlap(const SDL_Rect &rect1, const SDL_Rect &rect2);

bool aabb_overlap_all(const auto &rects)
{
    for (const SDL_Rect &rect1 : rects)
        for (const SDL_Rect &rect2 : rects) {
            if (&rect1 == &rect2)
                continue;
            if (aabb_overlap(rect1, rect2)) {
                return true;
            }
        }
    return false;
}

bool aabb_overlap_all(const auto &rectsList1, const auto &rectsList2)
{
    std::vector<SDL_Rect> rects;
    rects.insert(rects.end(), std::begin(rectsList1), std::end(rectsList1));
    rects.insert(rects.end(), std::begin(rectsList2), std::end(rectsList2));
    return aabb_overlap_all(rects);
}

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
    virtual void keyDown(int key);
    virtual void keyUp(int key);
    virtual void draw() const;
    virtual void update(float deltaTime, const std::vector<std::unique_ptr<object>> &others);

    virtual std::vector<SDL_Rect> get_collision_areas() const;
};

#endif  // GAMES_OBJECT_HPP
