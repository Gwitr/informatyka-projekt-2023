#ifndef GAMES_UTILS_HPP
#define GAMES_UTILS_HPP

#include <exception>         // std::exception
#include <string_view>       // std::string_view

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

inline const int SCREEN_WIDTH = 1080, SCREEN_HEIGHT = 810;

class sdl_error : public std::exception
{
    const char *m_what;
public:
    sdl_error(const char *what);
    const char *what() const throw();
};

void update_text(SDL_Renderer *renderer, TTF_Font *font, std::string_view newText, SDL_Color color, SDL_Texture *&textOut, int &textWidth, int &textHeight);

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
