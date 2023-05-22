#include "scene.hpp"
#include "utils.hpp"

// Most of the methods of a scene are left blank -- they're meant to be overriden
scene::scene(scenes &scenes, SDL_Renderer *renderer)
    : m_scenes{scenes}, m_renderer{renderer}  // Here, the member variables of the object are assigned
{

}
scene::~scene()
{
    // The destructor doesn't actually need to do anything
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


// The methods of the scenes object
// The constructor
scenes::scenes(int windowWidth, int windowHeight, std::string windowTitle)
    : m_windowWidth{windowWidth}, m_windowHeight{windowHeight}, m_titleText{windowTitle}
{
    // Creating the SDL window and renderer -- the very things we wrap around
    sdlCall(SDL_CreateWindowAndRenderer)(m_windowWidth, m_windowHeight, 0, &m_window, &m_renderer);
    // Setting the title of the window to the user-provided one
    sdlCall(SDL_SetWindowTitle)(m_window, m_titleText.c_str());
}

std::tuple<int, int> scenes::window_dimensions() const
{
    // The helper function to return the window size is trivial, thanks to the fact that we cached those properties in the constructor
    return { m_windowWidth, m_windowHeight };
}

scene &scenes::current_scene()
{
    auto last = m_scenes.end();
    if (m_scenes.begin() == last)  // Check edge case
        throw std::out_of_range("no scenes are currently on the stack");
    --last;  // The .end() iterator points to the non-existent "one element after the last" element, so we do -- to make it point to the last one
    return **last;  // First * dereferences the iterator, second * dereferences the unique_ptr
}

scene &scenes::previous_scene()
{
    // Analogous to the previous function, except here, the iterator needs to be decremented twice
    auto last = m_scenes.end();
    if (m_scenes.begin() == last)
        throw std::out_of_range("no scenes are currently on the stack");
    --last;
    if (m_scenes.begin() == last)
        throw std::out_of_range("only 1 scene is currently on the stack");
    --last;
    return **last;
}

void scenes::mainloop()
{
    // A basic SDL mainloop

    float deltaTime = 0.016;
    uint64_t lastFrameTicks = sdlCall(SDL_GetTicks64)();  // This will be used to keep track of the time took to render a frame
    bool running = true;
    while (running) {
        // Check events
        SDL_Event event;
        while (sdlCall(SDL_PollEvent)(&event)) {
            if (event.type == SDL_QUIT) {
                // The exit button is not forwarded to the scenes
                running = false;
            } else {
                // Only send events to the active scene
                auto last = m_scenes.end();
                if (last != m_scenes.begin()) {
                    --last;
                    (*last)->on_event(event);
                }
            }
        }

        // Only update the last scene
        auto last = m_scenes.end();
        if (last != m_scenes.begin()) {
            --last;
            (*last)->update(deltaTime);
        }

        // Draw every scene, making sure the active scene is drawn last (so, on top of all the other ones)
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

        // If there are no scenes left to run, that means that the game should quit
        if (m_scenes.size() == 0)
            running = false;
    }
}

void scenes::pop_scene()
{
    // Find the current last scene, and notify it that it is getting deactivated
    auto last = m_scenes.end();
    --last;
    (*last)->deactivate();

    // Remove the currently activated scene from the stack
    m_scenes.pop_back();

    // Notify the new active scene that it is back to living
    last = m_scenes.end();
    if (last != m_scenes.begin()) {
        --last;
        (*last)->activate();
    }
}
