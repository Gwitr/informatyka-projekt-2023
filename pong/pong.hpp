#ifndef GAMES_PONG_PONG_HPP
#define GAMES_PONG_PONG_HPP

#include "../scene.hpp"
#include "object.hpp"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

// An object that handles the scoreboard
class scoreboard : public object
{
    TTF_Font *const m_font;  // The font used to render it
    int m_score1 = 0, m_score2 = 0;  // Scores of both the players
    void update_score_text();  // Internal helper function to update the displayed text's texture
public:
    scoreboard(SDL_Renderer *renderer, TTF_Font *font, int startX, int startY);   // Constructor for the scoreboard object
    scoreboard(const scoreboard &) = delete;  // Don't allow copying it
    scoreboard(scoreboard &&other);  // Moving it is allowed, though
    virtual void draw() const override;  // We override the draw() function, as the object has custom behavior for that
    ~scoreboard();  // The destructor is overriden as well to let go of the allocated texture
    void addPoint(int player);  // A public function to add a point to a player
    virtual bool can_collide() const override;  // We also override the can_collide() function
};

// The scene of the pong game
class pong_scene final : public scene
{
    TTF_Font *m_font;  // Holds the font used in the game
    SDL_Texture *m_tex1, *m_tex2;  // Holds 2 textures
    std::vector<std::unique_ptr<object>> m_objects;  // Holds a list of objects
    scoreboard *m_scores;  // A cached pointer to the scoreboard object specifically

public:
    pong_scene(scenes &scenes, SDL_Renderer *renderer, bool hockeyMode);  // Constructor for pong_scene; invoked by scenes::push_scene<pong_scene>(bool), it takes hockeyMode as a required parameter
    ~pong_scene();   // Destructor for the pong scene, releases resources
    void update(float deltaTime) override;  // We override the update function
    void draw() const override;  // As well as the drawing function
    void on_event(const SDL_Event &event) override; // As well as the function that responds to SDL events
};

// Bit of a hack -- we have a global variable holding a custom event ID. I use the SDL event loop for communiaction here, as I have no other mechanism for hat
extern uint32_t givePointEventType;

#endif  // GAMES_PONG_PONG_HPP
