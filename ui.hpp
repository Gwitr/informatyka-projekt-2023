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

// Here we define our very bare-bones UI classes. They're enough for this little project, and since I ran out of time with the entire dungeon crawler part, I had no reason
// to extend it.

// The ui classes exist in a namespace
namespace ui
{
    // The generic widget class. It describes an on-screen element.
    class widget
    {
        std::vector<std::function<void()>> m_onClickHandlers;  // A list of functions to call when the widget is clicked
    protected:
        SDL_Renderer *const m_renderer;  // Each widget stores a pointer to the renderer

        // The position of the widget, as well as its size
        int m_x, m_y;
        int m_w, m_h;

        SDL_Color m_bg;  // The background color of the widget

    public:
        widget(SDL_Renderer *renderer, int x, int y, int w, int h, SDL_Color bg);  // The base constructor; note that the subclasses might have it look different (though the renderer should remain as the first parameter)
        widget(const widget &) = delete;  // We don't allow copying it; it should only ever belong to a widget_list object
        virtual ~widget();  // The destructor has to be virtual for safe polymorphism

        void bind_mouse_click(std::function<void()> handler);  // A method to add a function to the list of functions to call on click
        void on_event(const SDL_Event &event);  // A method that should be called whenever the game window receives an event
        SDL_Rect get_bounding_box() const;  // A method to query the bounding box of the widget on-screen
        virtual void draw() const = 0;  // An abstract method that draws the widget, it's supposed to be overriden
    };

    // A text widget
    class text : public widget
    {
        SDL_Texture *m_tex = nullptr;  // The cached texture
        SDL_Color m_fg;  // The color of the text

    public:
        // The constructor; instead of accepting a size, it accepts a font, some text, and a color
        text(SDL_Renderer *renderer, int x, int y, TTF_Font *font, std::string_view text, SDL_Color fg, SDL_Color bg);
        void draw() const override;  // We override the drawing function, as is required by the base widget class
    };

    // A class that contains and manages a list of widgets
    class widget_list final
    {
        SDL_Renderer *const m_renderer;  // The renderer it uses to draw
        std::vector<std::unique_ptr<widget>> m_widgets;  // The list of widgets

    public:
        widget_list(SDL_Renderer *renderer);  // The constructor. It doesn't need anything more than the renderer
        widget_list(const widget_list &) = delete;  // We don't allow copying it

        void on_event(const SDL_Event &event);  // This should be called when the main window receives an event

        // A template method to add a widget to the list of widgets. See: scenes::push_scene<T, Args...>
        template<typename T, typename ...Args>
        T &add_widget(Args&&... args)
        {
            return dynamic_cast<T&>(*m_widgets.emplace_back(new T{ m_renderer, std::forward<Args>(args) ... }));
        }
        void remove_widget(const widget &w); // A method to remove a widget from the list

        void draw() const;  // A function that should be called to draw the widgets on-screen
    };
}

#endif  // GAMES_UI_HPP
