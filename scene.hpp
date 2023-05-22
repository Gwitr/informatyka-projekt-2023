#ifndef GAMES_SCENE_HPP
#define GAMES_SCENE_HPP

#include <vector>
#include <memory>
#include <tuple>
#include <type_traits>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

class scenes;

// class that describes a scene of the game (so, basically a bundle of unique behavior, a game-state)
class scene {
protected:
    scenes &m_scenes;  // a reference to the collection of all scenes
    SDL_Renderer *const m_renderer;  // the SDL_Renderer attached to the main window
public:
    scene(scenes &scenes, SDL_Renderer *renderer);  // The constructor for this class
    scene(const scene &) = delete;  // The copy constructor is deleted, to avoid accidental copying of this object (as it's not designed for that, it should only exist under the ownership of a scenes object)
    virtual ~scene();  // The destructor has to be declared virtual for polymorphism to safely function

    bool active() const;  // Returns true if this scene is on top of the scenes stack

    // These 3 methods are marked as abstract -- any scene has to implement them
    virtual void draw() const = 0;  // Called on draw; is expected to not modify the scene's state
    virtual void update(float deltaTime) = 0;  // Called on update; is allowed to modify state, gets an argument that contains the time it took to render the last frame
    virtual void on_event(const SDL_Event &event) = 0;  // Called on an SDL event

    // These 2 methods are special hooks invoked when the scene is paused or unpaused. This isn't actually ever used, I probably could have done without it. I included it for completeness' sake.
    virtual void activate();  // Invoked when the scene becomes the topmost on the stack
    virtual void deactivate();  // Invoked when the scene stops being the topmost on the stack
};

// class that holds all scenes, as well as is responsible for running the main game loop, and handling the window, and all.
class scenes final {
    SDL_Renderer *m_renderer;  // The SDL_Renderer attached to the game window
    SDL_Window *m_window;  // The game window

    std::vector<std::unique_ptr<scene>> m_scenes;  // The stack of scenes. Only the top one is "active" -- all are rendered, but only the top one is updated or receives events.

    const int m_windowWidth, m_windowHeight;  // Helper constants that contain the game window dimensions
    const std::string m_titleText;  // A string containing the window caption text. Put it here because I'm not sure if SDL copies the window caption string or no (I checked and it does, but it's too late to do changes)
public:
    scenes(int windowWidth, int windowHeight, std::string windowTitle);  // The constructor. You give it all the data necessary to create the game window
    scenes(const scene &) = delete;  // We don't permit copying of this object (it wouldn't make much sense)

    std::tuple<int, int> window_dimensions() const;  // A method that gives the dimensions of the game window

    // A method that allows you to switch to another scene. The scene is pushed on the stack -- that means that the scene that was active before will become
    // inactive, BUT will still be rendered. This is to allow stacked menus and such; something that in the end I never did
    template<typename T, typename ...Args> requires std::is_base_of_v<scene, T>  // Fancy C++20 feature to ensure you don't pass in some random type
    void push_scene(Args&&... args)  // This method takes in an arbitrary amount of arguments
    {
        // Each scene type is actually expected to take a `scenes&` as its first argument, and an SDL_Renderer* as its second.
        // The push_scene method is basically a more convenient way to construct scene objects, too, as you don't need to pass
        // those 2 arguments in, it's all done automatically.
        m_scenes.emplace_back(new T{ *this, m_renderer, std::forward<Args>(args) ... })
            ->activate();  // Pushing a scene on the stack activates it
    }
    void pop_scene();  // A method to remove the current active scene off the stack, making the one below it active instead.

    scene &current_scene();  // Helper function to get the current active scene
    scene &previous_scene();  // Helper function to get the previous active scene

    void mainloop();  // This is the entry point to the game -- you call this, and the scenes object takes control of program flow, until the user exits the game.
};

#endif  // GAMES_SCENE_HPP
