#include <algorithm>
#include "utils.hpp"
#include "ui.hpp"

namespace ui
{
    widget::widget(SDL_Renderer *renderer, int x, int y, int w, int h, SDL_Color bg)
        : m_renderer{renderer}, m_x{x}, m_y{y}, m_w{w}, m_h{h}, m_bg{bg}
    {

    }

    widget::~widget()
    {

    }

    void widget::bind_mouse_click(std::function<void()> handler)
    {
        m_onClickHandlers.emplace_back(handler);
    }

    void widget::on_event(const SDL_Event &event)
    {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (event.button.button == SDL_BUTTON_LEFT) {
                int x, y;
                sdlCall(SDL_GetMouseState)(&x, &y);
                if (aabb_overlap(get_bounding_box(), { x, y, 1, 1 }))
                    // Mouse click detected
                    for (const auto &handler : m_onClickHandlers)
                        handler();
            }
        }
    }

    SDL_Rect widget::get_bounding_box() const
    {
        return { (int)(m_x-m_w/2), (int)(m_y-m_h/2), (int)m_w, (int)m_h };
    }


    text::text(SDL_Renderer *renderer, int x, int y, TTF_Font *font, std::string_view text, SDL_Color fg, SDL_Color bg)
        : widget{renderer, x, y, 0, 0, bg}, m_fg{fg}
    {
        update_text(renderer, font, text, fg, m_tex, m_w, m_h);
    }

    void text::draw() const
    {
        SDL_Rect dstrect = get_bounding_box();
        SDL_Rect srcrect = dstrect;
        srcrect.x = srcrect.y = 0;
        sdlCall(SDL_SetRenderDrawColor)(m_renderer, m_bg.r, m_bg.g, m_bg.b, m_bg.a);
        sdlCall(SDL_RenderFillRect)(m_renderer, &dstrect);
        sdlCall(SDL_RenderCopy)(m_renderer, m_tex, &srcrect, &dstrect);
    }


    widget_list::widget_list(SDL_Renderer *renderer)
        : m_renderer{renderer}
    {

    }

    void widget_list::on_event(const SDL_Event &event)
    {
        for (auto &widget : m_widgets) {
            widget->on_event(event);
        }
    }

    void widget_list::remove_widget(const widget &w)
    {
        auto iter = std::find_if(m_widgets.begin(), m_widgets.end(), [&w](const std::unique_ptr<widget> &widgetPtr)
        {
            return &*widgetPtr == &w;
        });
        m_widgets.erase(iter);
    }

    void widget_list::draw() const
    {
        for (auto &widgetPtr : m_widgets) {
            widgetPtr->draw();
        }
    }
}

