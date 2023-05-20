#include "scene.hpp"
#include "utils.hpp"

scene::scene(scenes &scenes, SDL_Renderer *renderer)
    : m_scenes{scenes}, m_renderer{renderer}
{

}
scene::~scene()
{

}

void scene::activate()
{

}
void scene::deactivate()
{

}

bool scene::active() const
{
    return &m_scenes.current_scene() == this;
}


scenes::scenes(int windowWidth, int windowHeight, std::string windowTitle)
    : m_windowWidth{windowWidth}, m_windowHeight{windowHeight}, m_titleText{windowTitle}
{
    sdlCall(SDL_CreateWindowAndRenderer)(m_windowWidth, m_windowHeight, 0, &m_window, &m_renderer);
    sdlCall(SDL_SetWindowTitle)(m_window, m_titleText.c_str());
}

std::tuple<int, int> scenes::window_dimensions() const
{
    return { m_windowWidth, m_windowHeight };
}

scene &scenes::current_scene()
{
    auto last = m_scenes.end();
    if (m_scenes.begin() == last)
        throw std::out_of_range("no scenes are currently on the stack");
    --last;
    return **last;
}

scene &scenes::previous_scene()
{
    auto last = m_scenes.end();
    --last;
    if (m_scenes.begin() == last)
        throw std::out_of_range("only 1 scene is currently on the stack");
    --last;
    return **last;
}

void scenes::mainloop()
{
    float deltaTime = 0.016;
    uint64_t lastFrameTicks = sdlCall(SDL_GetTicks64)();
    bool running = true;
    while (running) {
        // Check events
        SDL_Event event;
        while (sdlCall(SDL_PollEvent)(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else {
                // Only send events to the last scene
                auto last = m_scenes.end();
                --last;
                (*last)->on_event(event);
            }
        }

        // Only update the last scene
        auto last = m_scenes.end();
        --last;
        (*last)->update(deltaTime);

        // Draw every scene
        for (const auto &scene : m_scenes)
            scene->draw();
        sdlCall(SDL_RenderPresent)(m_renderer);

        // Lock the framerate
        uint64_t currentFrameTicks = sdlCall(SDL_GetTicks64)();
        deltaTime = (currentFrameTicks - lastFrameTicks) / 1000.0f;
        if (deltaTime < 0.016f) {
            sdlCall(SDL_Delay)(16 - 1000 * deltaTime);
        }
        deltaTime = (sdlCall(SDL_GetTicks64)() - lastFrameTicks) / 1000.0f;
        lastFrameTicks = currentFrameTicks;

        if (m_scenes.size() == 0)
            running = false;
    }
}

void scenes::pop_scene()
{
    auto last = m_scenes.end();
    --last;
    (*last)->deactivate();

    m_scenes.pop_back();

    last = m_scenes.end();
    --last;
    (*last)->activate();
}
