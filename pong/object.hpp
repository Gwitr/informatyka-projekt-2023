#ifndef GAMES_OBJECT_HPP
#define GAMES_OBJECT_HPP

#include <unordered_map>     // std::unordered_map
#include <vector>            // std::vector
#include <memory>            // std::unique_ptr
#include "../utils.hpp"
#include <iostream>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

// The base class for every object in the pong game.
class object {
protected:
    // All of these are protected -- they're accesible in the classes that inherit from this.

    // The current renderer, as well as the texture of this object
    SDL_Renderer *const m_renderer;  // FIXME: This probably shouldn't be here
    SDL_Texture *m_texture;

    // The texture size for this object
    int m_texWidth, m_texHeight;

    // The on-screen position of this object
    float m_x, m_y;

    // The starting position for this object (we reset to it)
    const int m_startX, m_startY;

    // The screen dimensions (FIXME: This probably shouldn't be here)
    int m_maxX, m_maxY;

    // A hashmap of every single key, mapped to whether it is pressed or not (FIXME: This definitely shouldn't be here)
    std::unordered_map<int, bool> m_keys;
public:
    // The constructor for an object. It takes in the current renderer, texture, as well as the starting position.
    object(SDL_Renderer *renderer, SDL_Texture *tex, int startX, int startY);
    // This doesn't do much in particular, but it has to be declared virtual to allow for safe polymorphism
    virtual ~object();

    virtual bool can_collide() const;  // A subclass can override this function and change when the object is tangible
    virtual void reset();              // A public function that lets reset the object to its starting state. A child class should override it if it holds any extra state to be reset.
    virtual void keyDown(int key);     // A function that is called when a key is pressed down on the keyboard
    virtual void keyUp(int key);       // A function that is called when a key is released on the keyboard
    // A function that's called to draw the object. This class provides a default interpretation, but subclasses requiring more complex rendering capabilities
    // may override it.
    virtual void draw() const;
    virtual void update(float deltaTime, const std::vector<std::unique_ptr<object>> &others);  // A function called every frame. A subclass can decide to do anything here

    virtual std::vector<SDL_Rect> get_collision_areas() const;  // A function that returns a list of collision areas for this object. By default, it's just 1 area, that being the bounding box of the texture.
};

#endif  // GAMES_OBJECT_HPP
