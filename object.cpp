#include "object.hpp"

object::object(SDL_Renderer *renderer, SDL_Texture *tex, int startX, int startY)
    : m_renderer{renderer}, m_texture{tex}, m_x{(float)startX}, m_y{(float)startY},
        m_startX{startX}, m_startY{startY}
{
    if (tex) {
        int _1;
        uint32_t _2;
        sdlCall(SDL_QueryTexture)(tex, &_2, &_1, &m_texWidth, &m_texHeight);
    }
    sdlCall(SDL_GetRendererOutputSize)(renderer, &m_maxX, &m_maxY);
}

object::~object()
{

}

void object::reset()
{
    m_x = m_startX;
    m_y = m_startY;
}

bool aabb_overlap(const SDL_Rect &rect1, const SDL_Rect &rect2)
{
    auto checkContainedInRange = [](int start, int end, int num)
    {
        return num >= start && num <= end;
    };
    bool overlapX = checkContainedInRange(rect1.x, rect1.x + rect1.w, rect2.x);
    overlapX |= checkContainedInRange(rect1.x, rect1.x + rect1.w, rect2.x + rect2.w);
    overlapX |= checkContainedInRange(rect2.x, rect2.x + rect2.w, rect1.x);
    overlapX |= checkContainedInRange(rect2.x, rect2.x + rect2.w, rect1.x + rect1.w);

    bool overlapY = checkContainedInRange(rect1.y, rect1.y + rect1.h, rect2.y);
    overlapY |= checkContainedInRange(rect1.y, rect1.y + rect1.h, rect2.y + rect2.h);
    overlapY |= checkContainedInRange(rect2.y, rect2.y + rect2.h, rect1.y);
    overlapY |= checkContainedInRange(rect2.y, rect2.y + rect2.h, rect1.y + rect1.h);

    return overlapX && overlapY;
}

// FIXME: Pressed keys should be handled globally
void object::keyDown(int key)
{
    m_keys[key] = true;
}

void object::keyUp(int key)
{
    m_keys[key] = false;
}

void object::draw() const
{
    SDL_Rect srcrect = { 0, 0, m_texWidth, m_texHeight };
    SDL_Rect dstrect = { (int)m_x, (int)m_y, m_texWidth, m_texHeight };
    sdlCall(SDL_RenderCopy)(m_renderer, m_texture, &srcrect, &dstrect);
}

void object::update(float deltaTime, const std::vector<std::unique_ptr<object>> &others)
{
    (void)deltaTime; (void)others;
}

bool object::can_collide() const
{
    return true;
}

std::vector<SDL_Rect> object::get_collision_areas() const
{
    return std::vector{ SDL_Rect{ (int)m_x, (int)m_y, m_texWidth, m_texHeight } };
}
