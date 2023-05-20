#ifndef GAMES_UTILS_HPP
#define GAMES_UTILS_HPP

#include <exception>    // std::exception
#include <string_view>  // std::string_view
#include <vector>       // std::vector

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

inline const int SCREEN_WIDTH = 1080, SCREEN_HEIGHT = 810;

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


class sdl_error : public std::exception
{
    const char *m_what;
public:
    sdl_error(const char *what);
    const char *what() const throw();
};

void update_text(SDL_Renderer *renderer, TTF_Font *font, std::string_view newText, SDL_Color color, SDL_Texture *&textOut, int &textWidth, int &textHeight);

SDL_Texture *render_text(SDL_Renderer *renderer, TTF_Font *font, std::string_view newText, SDL_Color color);
SDL_Texture *render_text(SDL_Renderer *renderer, TTF_Font *font, std::string_view newText, SDL_Color color, int &textWidth, int &textHeight);

template<typename ReturnType, typename ...ArgTypes>
constexpr auto sdlCall(ReturnType (*func)(ArgTypes...))
{
    // NOTE: I decided against perfect forwarding as it prevents automatic casts for some reason
    return [func](ArgTypes... args)
    {
        SDL_ClearError();
        if constexpr (std::is_same_v<ReturnType, void>) {
            func(args ...);
            const char *err = SDL_GetError();
            if (err[0] != 0)
                throw sdl_error(err);

        } else {
            decltype(auto) value = func(args ...);
            const char *err = SDL_GetError();
            if (err[0] != 0)
                throw sdl_error(err);
            return value;
        }
    };
}

#endif  // GAMES_UTILS_HPP
