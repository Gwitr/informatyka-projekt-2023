#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include <exception>  // std::exception
#include <string>     // std::string
#include <memory>     // std::unique_ptr

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

const int SCREEN_WIDTH = 256, SCREEN_HEIGHT = 256;

class sdl_error : public std::exception
{
    const char *m_what;
public:
    sdl_error(const char *what)
        : m_what{what} {}
    
    const char *what() const throw()
    {
        return m_what;
    }
};

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

int main(int argc, char **argv)
{
    SDL_Window *window;
    SDL_Renderer *renderer;

    sdlCall(SDL_Init)(SDL_INIT_EVERYTHING);
    sdlCall(IMG_Init)(IMG_INIT_JPG | IMG_INIT_PNG);

    sdlCall(SDL_CreateWindowAndRenderer)(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);

    SDL_Texture *tex = sdlCall(IMG_LoadTexture)(renderer, "paddle.png");
    int w, h, access;
    Uint32 format;
    sdlCall(SDL_QueryTexture)(tex, &format, &access, &w, &h);

    std::unique_ptr<const std::string> title;
    uint64_t lastFrameTicks = sdlCall(SDL_GetTicks64)();
    float deltaTime = 0.016f;
    bool running = true;
    while (running) {
        // Handle events
        SDL_Event event;
        while (sdlCall(SDL_PollEvent)(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            }
        }

        // Lock to 60 FPS
        uint64_t currentFrameTicks = sdlCall(SDL_GetTicks64)();
        deltaTime = (currentFrameTicks - lastFrameTicks) / 1000.0f;
        if (deltaTime < 0.016f) {
            sdlCall(SDL_Delay)(16 - 1000 * deltaTime);
        }
        deltaTime = (sdlCall(SDL_GetTicks64)() - lastFrameTicks) / 1000.0f;
        lastFrameTicks = currentFrameTicks;

        SDL_Rect srcrect = { 0, 0, w, h };
        SDL_Rect dstrect = { 20, 20, w, h*2 };
        sdlCall(SDL_RenderClear)(renderer);
        sdlCall(SDL_RenderCopy)(renderer, tex, &srcrect, &dstrect);
        sdlCall(SDL_RenderPresent)(renderer);

        // Do this weird thing with std::unique_ptr to ensure that SDL never ends up with a stale pointer
        auto newTitle = std::make_unique<std::string>("FPS: " + std::to_string((int)(1.0f / deltaTime)));
        sdlCall(SDL_SetWindowTitle)(window, newTitle->c_str());
        title = std::move(newTitle);
    }

    sdlCall(SDL_DestroyTexture)(tex);
    sdlCall(IMG_Quit)();
    sdlCall(SDL_Quit)();

    return 0;
}
