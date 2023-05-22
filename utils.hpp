#ifndef GAMES_UTILS_HPP
#define GAMES_UTILS_HPP

// This file contains various utility functions

#include <exception>    // std::exception
#include <string_view>  // std::string_view
#include <vector>       // std::vector

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

const int SCREEN_WIDTH = 1080, SCREEN_HEIGHT = 810;

// This function calculates if 2 axis-aligned rectangles overlap
bool aabb_overlap(const SDL_Rect &rect1, const SDL_Rect &rect2);

// This function does what the previous function did, but for a list of rectangles. It's a template function, because of the `auto` argument, and so it has to be
// in the header file. The function assumes that the passed type is an iterator via duck typing, not through concepts, despite us having access to those via C++20
// (mostly because the ranges library is very esoteric)
bool aabb_overlap_all(const auto &rects)
{
    for (const SDL_Rect &rect1 : rects)
        for (const SDL_Rect &rect2 : rects) {
            if (&rect1 == &rect2) // Make sure that the rectangles checked are not the exact same object
                continue;
            // Delegate the hard work to the other function
            if (aabb_overlap(rect1, rect2)) {
                return true;
            }
        }
    // No rectangles overlapped after all
    return false;
}

// Helper function to overlap two lists of rectangles, as if they were one list. Unbelieveable that C++ STILL doesn't feature simple collection concatenation.
bool aabb_overlap_all(const auto &rectsList1, const auto &rectsList2)
{
    std::vector<SDL_Rect> rects;
    rects.insert(rects.end(), std::begin(rectsList1), std::end(rectsList1));
    rects.insert(rects.end(), std::begin(rectsList2), std::end(rectsList2));
    return aabb_overlap_all(rects);
}

// A class that describes an error thrown by SDL
class sdl_error : public std::exception
{
    const char *m_what;
public:
    sdl_error(const char *what);
    const char *what() const throw();
};

// Helper function to "update" a texture containing text with some new text
void update_text(SDL_Renderer *renderer, TTF_Font *font, std::string_view newText, SDL_Color color, SDL_Texture *&textOut, int &textWidth, int &textHeight);

// Helper functions to render text into a texture
SDL_Texture *render_text(SDL_Renderer *renderer, TTF_Font *font, std::string_view newText, SDL_Color color);
// This one also lets you get the texture's size without having to manually call SDL_QueryTexture
SDL_Texture *render_text(SDL_Renderer *renderer, TTF_Font *font, std::string_view newText, SDL_Color color, int &textWidth, int &textHeight);

// This is a template function, and so it has to be defined in the header. Its purpose is to wrap around SDL functions, adding exception support to them, so that
// you do not have to constantly check return values and error states like you would in C. This does that for you, automatically.
template<typename ReturnType, typename ...ArgTypes>
constexpr auto sdlCall(ReturnType (*func)(ArgTypes...))  // We declare the function as constexpr, as it's fully possible to evaluate at compile-time -- all it does is return a lambda.
{
    // NOTE: I decided against perfect forwarding as it prevents automatic casts for some reason (A design choice that I would swiftly give up on, but I don't have time to fix)
    // This lambda function captures the `func` function pointer by value (so, that variable will be accessible inside its scope). It takes a variable number of parameters, depending
    // on the parameters that the function pointer takes
    return [func](ArgTypes... args)
    {
        // First, we clear any error. This sets the error string to "".
        SDL_ClearError();
        // C++ is a bit annoying, sadly -- void is an incomplete type, and so we can't have a variable of that type, and so we need 2 cases for when the function returns void, and when it doesn't.
        // Thankfully, we don't have to work with SFINAE, as `if constexpr` allows us to easily check types
        if constexpr (std::is_same_v<ReturnType, void>) {
            // Call the original SDL function with the specified arguments
            func(args ...);
            // Collect the current SDL error
            const char *err = SDL_GetError();
            if (err[0] != 0)  // If it's not empty,
                throw sdl_error(err);  // then throw it as an exception.

        } else {
            decltype(auto) value = func(args ...);  // Call the original SDL function with the specified arguments, and collect its result
            // Same as above
            const char *err = SDL_GetError();
            if (err[0] != 0)
                throw sdl_error(err);
            return value;  // Return the result of the SDL function call
        }
    };
}

#endif  // GAMES_UTILS_HPP
