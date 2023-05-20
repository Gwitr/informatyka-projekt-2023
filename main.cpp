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
#include "scene.hpp"
#include "ui.hpp"
#include "pong/pong.hpp"

class menu_scene final : public scene
{
    ui::widget_list m_widgets;

    TTF_Font *m_font;
public:
    menu_scene(scenes &scenes, SDL_Renderer *renderer)
        : scene{scenes, renderer}, m_widgets{renderer}
    {
        m_font = sdlCall(TTF_OpenFont)("Terminus.ttf", 32);

        const auto [windowW, windowH] = m_scenes.window_dimensions();

        auto &text1 = m_widgets.add_widget<ui::text>(
            windowW / 2, windowH / 2, 
            m_font, "Play Pong",
            SDL_Color { 255, 255, 255, 255 }, SDL_Color { 255, 0, 0, 255 }
        );
        text1.bind_mouse_click([this]()
        {
            m_scenes.push_scene<pong_scene>(false);
        });

        auto &text2 = m_widgets.add_widget<ui::text>(
            windowW / 2, windowH / 2 + text1.get_bounding_box().h + 8,
            m_font, "Play Hockey",
            SDL_Color { 255, 255, 255, 255 }, SDL_Color { 255, 0, 0, 255 }
        );
        text2.bind_mouse_click([this]()
        {
            m_scenes.push_scene<pong_scene>(true);
        });
    }

    ~menu_scene()
    {
        sdlCall(TTF_CloseFont)(m_font);
    }

    void draw() const override
    {
        sdlCall(SDL_SetRenderDrawColor)(m_renderer, 0, 0, 0, 255);
        sdlCall(SDL_RenderClear)(m_renderer);
        m_widgets.draw();
    }

    void update(float) override { }

    virtual void on_event(const SDL_Event &event) override
    {
        m_widgets.on_event(event);
    }
};

int main()
{
    sdlCall(SDL_Init)(SDL_INIT_EVERYTHING);
    sdlCall(IMG_Init)(IMG_INIT_JPG | IMG_INIT_PNG);
    sdlCall(TTF_Init)();
    givePointEventType = sdlCall(SDL_RegisterEvents)(1);

    {
        scenes sceneStack{SCREEN_WIDTH, SCREEN_HEIGHT, "Bouncy games"};
        sceneStack.push_scene<menu_scene>();
        sceneStack.mainloop();
    }

    sdlCall(TTF_Quit)();
    sdlCall(IMG_Quit)();
    sdlCall(SDL_Quit)();

    return 0;
}
