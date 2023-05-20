#ifndef GAMES_SCENE_HPP
#define GAMES_SCENE_HPP

#include <vector>
#include <memory>
#include <tuple>
#include <type_traits>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

class scenes;

class scene {
protected:
    scenes &m_scenes;
    SDL_Renderer *const m_renderer;
public:
    scene(scenes &scenes, SDL_Renderer *renderer);
    scene(const scene &) = delete;
    virtual ~scene();

    bool active() const;

    virtual void draw() const = 0;
    virtual void update(float deltaTime) = 0;    
    virtual void on_event(const SDL_Event &event) = 0;

    virtual void activate();
    virtual void deactivate();
};

class scenes final {
    SDL_Renderer *m_renderer;
    SDL_Window *m_window;

    std::vector<std::unique_ptr<scene>> m_scenes;

    const int m_windowWidth, m_windowHeight;
    const std::string m_titleText;
public:
    scenes(int windowWidth, int windowHeight, std::string windowTitle);

    std::tuple<int, int> window_dimensions() const;

    template<typename T, typename ...Args> requires std::is_base_of_v<scene, T>
    void push_scene(Args&&... args)
    {
        m_scenes.emplace_back(new T{ *this, m_renderer, std::forward<Args>(args) ... })
            ->activate();
    }
    void pop_scene();

    scene &current_scene();
    scene &previous_scene();

    void mainloop();
};

#endif  // GAMES_SCENE_HPP
