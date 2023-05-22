#include <cstdint>           // uint32_t, intptr_t
#include <cmath>             // sqrtf
#include <algorithm>         // std::find
#include <iostream>          // std::cout
#include <string>            // std::string
#include <memory>            // std::unique_ptr
#include <vector>            // std::vector
#include <initializer_list>  // std::initializer_list
#include <array>             // std::array

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "utils.hpp"
#include "scene.hpp"
#include "ui.hpp"
#include "pong/pong.hpp"

/*
// Left unfinished, I sadly ran out of time

class room
{
    friend room_graph;

    struct neighbor_ref
    {
        std::weak_ptr<room> room;
        unsigned distance;
    };

    struct connection
    {
        std::weak_ptr<room> room1, room2;
        int x1, y1, x2, y2;

        std::weak_ptr<room> other(const room &me)
        {
            if (&me == &*room1.lock())
                return room2;
            return room1;
        }
    };

    room_graph &graph;
    int m_x, m_y;
    int m_mapX1, m_mapY1, m_mapX2, m_mapY2;
    mutable std::array<neighbor_ref, 4> m_neighbors;
    mutable bool m_taintID;

    std::vector<std::shared_ptr<connection>> m_connections;

    void regen_cache() const
    {
        if (m_taintID == graph.m_taintID)
            return;
        m_taintID = graph.m_taintID;

        // Get neighbors
        const std::vector<std::array<int,2>> dirs = { {0,1}, {0,-1}, {-1,0}, {1,0} };
        for (int i = 0; i < 4; ++i) {
            int x = m_x + dirs[i][0], y = m_y + dirs[i][1];
            m_neighbors[i] = { std::weak_ptr<room>(), 0 };
            unsigned dst = 1;
            while (x >= 0 && x < graph.width && y >= 0 && y < graph.height) {
                std::weak_ptr<room> maybeNeighbor = graph.at_cell(x, y);
                if (!maybeNeighbor.expired()) {
                    m_neighbors[i] = { maybeNeighbor, dst };
                    break;
                }
                x += dirs[i][0];
                y += dirs[i][1];
                ++dst;
            }

        }
    }

    void connect_to(room &other, int x1, int y1, int x2, int y2)
    {
        auto conn = std::make_shared<connection>(*this, other, x1, y1, x2, y2);
        m_connections.push_back(conn);
        other.m_connections.push_back(conn);
    }

public:
    neighbor_ref &neighbor(int dir)
    {
        regen_cache();
        return m_neighbors[dir];
    }

    const std::array<neighbor_ref, 4> &neighbors()
    {
        return m_neighbors;
    }
    
    std::tuple<int, int> pos() const { return { m_x, m_y }; }
};

class room_graph
{
    friend room;

    std::vector<std::shared_ptr<room>> m_rooms;
    int m_taintID;
public:
    const int width, height;

    std::weak_ptr<room> at_cell(int x, int y)
    {
        for (auto room : m_rooms) {
            auto [roomx, roomy] = room->pos();
            if (roomx == x && roomy == y)
                return room;
        }
        return std::weak_ptr<room>{};
    }

    using room_list = std::vector<std::weak_ptr<room>>;
    std::vector<room_list> find_disjoint() const
    {
        room_list visited;
        room_list toVisit;
        toVisit.push_back(m_rooms[0]);
        for (int i = 0; i < toVisit.size();) {
            auto room = toVisit[i];
            visited.push_back(toVisit[i]);
            toVisit.erase(toVisit.begin());
            auto roomPtr = room.lock();
            for (auto connection : roomPtr->m_connections) {
                toVisit.push_back(connection->other(*roomPtr));
            }
        }
        // ...
    }
};

class dungeon_crawler_scene final : public scene
{
    std::array<std::array<uint8_t, 64>, 48> m_tiles;
public:
    dungeon_crawler_scene(scenes &scenes, SDL_Renderer *renderer)
        : scene{scenes, renderer}
    {
    }

    void activate() override
    {
        for (unsigned y = 0; y < 48; ++y) {
            for (unsigned x = 0; x < 64; ++x) {
                m_tiles[y][x] = 0;
            }
        }

        draw_rect(40, 30, 50, 40, 1);

        draw_line(20, 40, 10, 10, 4, 1);
    }

    int clip_x(int x)
    {
        return x < 0 ? 0 : (x > 63 ? 63 : x);
    }

    int clip_y(int y)
    {
        return y < 0 ? 0 : (y > 47 ? 47 : y);
    }

    void draw_rect(int x1, int y1, int x2, int y2, uint8_t fill)
    {
        if (x1 > x2)
            std::swap(x1, x2);
        if (y1 > y2)
            std::swap(y1, y2);
        for (int y = y1; y <= y2; ++y)
            for (int x = x1; x <= x2; ++x)
                m_tiles[y][x] = fill;
    }

    void draw_hline(int x1, int x2, int y, uint8_t fill)
    {
        if (x2 < x1)
            std::swap(x1, x2);
        y = clip_y(y);
        x1 = clip_x(x1);
        x2 = clip_x(x2);

        for (int x = x1; x <= x2; ++x) {
            m_tiles[y][x] = fill;
        }
    }

    void draw_vline(int x, int y1, int y2, uint8_t fill)
    {
        if (y2 < y1)
            std::swap(y1, y2);
        x = clip_x(x);
        y1 = clip_y(y1);
        y2 = clip_y(y2);

        for (int y = y1; y <= y2; ++y) {
            m_tiles[y][x] = fill;
        }
    }

    void draw_line(int x1, int y1, int x2, int y2, int nsegments, uint8_t fill)
    {
        int diffx = x2 - x1, diffy = y2 - y1;
        if (diffx < 0)
            diffx = -diffx;
        if (diffy < 0)
            diffy = -diffy;

        if (diffx < diffy) {
            std::vector<int> yJumpPositions;
            std::vector<int> xPositions;
            for (int i = 0; i < nsegments; ++i) {
                float t = (float)i / (nsegments-1);
                xPositions.push_back((1-t) * x1 + t * x2);
            }
            for (int i = 0; i < nsegments+1; ++i) {
                float t = (float)i / (nsegments);
                yJumpPositions.push_back((1-t) * y1 + t * y2);
            }
            int yJump = 0;
            for (int i = 0; i < nsegments; ++i)
            {
                draw_vline(xPositions[yJump], yJumpPositions[yJump], yJumpPositions[yJump+1], fill);
                if (i != nsegments - 1)
                    draw_hline(xPositions[yJump], xPositions[yJump+1], yJumpPositions[yJump+1], fill);
                ++yJump;
            }

        } else {
            std::vector<int> xJumpPositions;
            std::vector<int> yPositions;
            for (int i = 0; i < nsegments; ++i) {
                float t = (float)i / (nsegments-1);
                yPositions.push_back((1-t) * y1 + t * y2);
            }
            for (int i = 0; i < nsegments+1; ++i) {
                float t = (float)i / (nsegments);
                xJumpPositions.push_back((1-t) * x1 + t * x2);
            }
            int xJump = 0;
            for (int i = 0; i < nsegments; ++i)
            {
                draw_hline(xJumpPositions[xJump], xJumpPositions[xJump+1], yPositions[xJump], fill);
                if (i != nsegments - 1)
                    draw_vline(xJumpPositions[xJump+1], yPositions[xJump], yPositions[xJump+1], fill);
                ++xJump;
            }
        }
    }

    void draw() const override
    {
        sdlCall(SDL_SetRenderDrawColor)(m_renderer, 0, 0, 0, 255);
        sdlCall(SDL_RenderClear)(m_renderer);
        for (unsigned y = 0; y < 48; ++y) {
            for (unsigned x = 0; x < 64; ++x) {
                if (m_tiles[y][x] > 0)
                    sdlCall(SDL_SetRenderDrawColor)(m_renderer, 255, 255, 255, 255);
                else
                    sdlCall(SDL_SetRenderDrawColor)(m_renderer, 30, 30, 30, 255);
                SDL_Rect pixel = { (int)x * 4, (int)y * 4, 4, 4 };
                sdlCall(SDL_RenderFillRect)(m_renderer, &pixel);
            }
        }
    }

    void update(float) override { }

    void on_event(const SDL_Event &event) override
    {
        if (event.type == SDL_KEYDOWN)
            if (event.key.keysym.sym == SDLK_ESCAPE)
                m_scenes.pop_scene();
    }
};
*/
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

        /*
        auto &text3 = m_widgets.add_widget<ui::text>(
            windowW / 2, windowH / 2 + text1.get_bounding_box().h * 2 + 8 * 2,
            m_font, "Play Dungeon Crawler",
            SDL_Color { 255, 255, 255, 255 }, SDL_Color { 255, 0, 0, 255 }
        );
        text3.bind_mouse_click([this]()
        {
            m_scenes.push_scene<dungeon_crawler_scene>();
        });
        */

        auto &text4 = m_widgets.add_widget<ui::text>(
            windowW / 2, windowH / 2 + text1.get_bounding_box().h * 2 + 8 * 2,
            m_font, "Exit",
            SDL_Color { 255, 255, 255, 255 }, SDL_Color { 255, 0, 0, 255 }
        );
        text4.bind_mouse_click([this]()
        {
            m_scenes.pop_scene();
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

    void on_event(const SDL_Event &event) override
    {
        m_widgets.on_event(event);
    }
};

int main()
{
    // srand(time(NULL));

    // SDL initialization
    sdlCall(SDL_Init)(SDL_INIT_EVERYTHING);
    sdlCall(IMG_Init)(IMG_INIT_JPG | IMG_INIT_PNG);
    sdlCall(TTF_Init)();

    // Make a new event for use in the game (this is a bit ugly, but I ran out of time to fix it)
    givePointEventType = sdlCall(SDL_RegisterEvents)(1);

    // Making sure that this is a block so that we don't end up invoking any destructors after calling the *_Quit functions
    {
        // Create the scene stack (it holds the stack of scenes featured in the game)
        scenes sceneStack{SCREEN_WIDTH, SCREEN_HEIGHT, "Bouncy games"};
        // Make the game start off in the menu scene
        sceneStack.push_scene<menu_scene>();
        // Run the game
        sceneStack.mainloop();
    }

    // De-initialize all the SDL libraries
    sdlCall(TTF_Quit)();
    sdlCall(IMG_Quit)();
    sdlCall(SDL_Quit)();

    return 0;
}
