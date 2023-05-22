#include <algorithm>
#include "utils.hpp"
#include "ui.hpp"

namespace ui
{
    // The constructor of a base widget simply sets some member variables
    widget::widget(SDL_Renderer *renderer, int x, int y, int w, int h, SDL_Color bg)
        : m_renderer{renderer}, m_x{x}, m_y{y}, m_w{w}, m_h{h}, m_bg{bg}
    {

    }

    // The destructor doesn't need to do anything
    widget::~widget()
    {

    }

    // Binding a function is as simple as adding it to the list of bound functions
    void widget::bind_mouse_click(std::function<void()> handler)
    {
        m_onClickHandlers.emplace_back(handler);
    }

    // The on-event behavior of any widget
    void widget::on_event(const SDL_Event &event)
    {
        // If a mouse button is down...
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            // And it's the left button...
            if (event.button.button == SDL_BUTTON_LEFT) {
                int x, y;
                sdlCall(SDL_GetMouseState)(&x, &y);
                // And the mouse overlaps the widget's bounding box...
                if (aabb_overlap(get_bounding_box(), { x, y, 1, 1 }))
                    // ...then a click has occured -- invoke all the handlers.
                    for (const auto &handler : m_onClickHandlers)  // FIXME: I should probably be copying m_widgets here
                        handler();
            }
        }
    }

    SDL_Rect widget::get_bounding_box() const
    {
        // The bounding box of a widget is very simple to figure out -- note that we subtract half the width and height from the position,
        // we do that to center the widget
        return { (int)(m_x-m_w/2), (int)(m_y-m_h/2), (int)m_w, (int)m_h };
    }


    // The constructor for the text widget
    text::text(SDL_Renderer *renderer, int x, int y, TTF_Font *font, std::string_view text, SDL_Color fg, SDL_Color bg)
        : widget{renderer, x, y, 0, 0, bg}, m_fg{fg}  // We delegate most work to the base constructor
    {
        // Updating the text is very simple!
        update_text(renderer, font, text, fg, m_tex, m_w, m_h);
    }

    void text::draw() const
    {
        // Drawing is not too hard either, thanks to get_bounding_box we can make the method more readable
        SDL_Rect dstrect = get_bounding_box();
        SDL_Rect srcrect = dstrect;
        srcrect.x = srcrect.y = 0;
        sdlCall(SDL_SetRenderDrawColor)(m_renderer, m_bg.r, m_bg.g, m_bg.b, m_bg.a);
        sdlCall(SDL_RenderFillRect)(m_renderer, &dstrect);
        sdlCall(SDL_RenderCopy)(m_renderer, m_tex, &srcrect, &dstrect);
    }


    // The widget_list constructor doesn't need to do anything besides setting member variables
    widget_list::widget_list(SDL_Renderer *renderer)
        : m_renderer{renderer}
    {

    }

    // On an event, the widget_list propagates it to all existing widgets. You could probably add a fancy focusing system here, but it wasn't necessary,
    // and also, that would have required me to plan things out in advance.
    void widget_list::on_event(const SDL_Event &event)
    {
        for (auto &widget : m_widgets) {  // FIXME: I should probably be copying m_widgets here
            widget->on_event(event);
        }
    }

    void widget_list::remove_widget(const widget &w)
    {
        // Find a matching widget in the list
        auto iter = std::find_if(m_widgets.begin(), m_widgets.end(), [&w](const std::unique_ptr<widget> &widgetPtr)
        {
            return &*widgetPtr == &w;
        });
        // And remove it from said list.
        m_widgets.erase(iter);
    }

    void widget_list::draw() const
    {
        // Drawing is trivial as well -- like on_event, we simply propagate it to all widgets in the list.
        for (auto &widgetPtr : m_widgets) {
            widgetPtr->draw();
        }
    }
}

