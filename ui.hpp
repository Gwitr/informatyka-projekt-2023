#ifndef GAMES_UI_HPP
#define GAMES_UI_HPP

#include <vector>
#include <memory>
#include <string_view>
#include <algorithm>
#include <functional>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

namespace ui
{
    class widget
    {
        std::vector<std::function<void()>> m_onClickHandlers;
    protected:
        SDL_Renderer *const m_renderer;

        int m_x, m_y;
        int m_w, m_h;

        SDL_Color m_bg;

    public:
        widget(SDL_Renderer *renderer, int x, int y, int w, int h, SDL_Color bg);
        widget(const widget &) = delete;
        virtual ~widget();

        void bind_mouse_click(std::function<void()> handler);
        void on_event(const SDL_Event &event);
        SDL_Rect get_bounding_box() const;
        virtual void draw() const = 0;
    };

    class text : public widget
    {
        SDL_Texture *m_tex = nullptr;
        SDL_Color m_fg;

    public:
        text(SDL_Renderer *renderer, int x, int y, TTF_Font *font, std::string_view text, SDL_Color fg, SDL_Color bg);
        void draw() const override;
    };

    class widget_list final
    {
        SDL_Renderer *const m_renderer;
        std::vector<std::unique_ptr<widget>> m_widgets;

    public:
        widget_list(SDL_Renderer *renderer);

        void on_event(const SDL_Event &event);

        template<typename T, typename ...Args>
        T &add_widget(Args&&... args)
        {
            return dynamic_cast<T&>(*m_widgets.emplace_back(new T{ m_renderer, std::forward<Args>(args) ... }));
        }
        void remove_widget(const widget &w);

        void draw() const;
    };
}

#endif  // GAMES_UI_HPP
