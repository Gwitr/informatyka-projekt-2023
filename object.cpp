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

bool object::aabb_overlap(const object &other) const
{
    auto checkContainedInRange = [](int start, int end, int num)
    {
        return num >= start && num <= end;
    };
    bool overlapX = checkContainedInRange(m_x, m_x + m_texWidth, other.m_x);
    overlapX |= checkContainedInRange(m_x, m_x + m_texWidth, other.m_x + other.m_texWidth);
    overlapX = checkContainedInRange(other.m_x, other.m_x + other.m_texWidth, m_x);
    overlapX |= checkContainedInRange(other.m_x, other.m_x + other.m_texWidth, m_x + m_texWidth);

    bool overlapY = checkContainedInRange(m_y, m_y + m_texHeight, other.m_y);
    overlapY |= checkContainedInRange(m_y, m_y + m_texHeight, other.m_y + other.m_texHeight);
    overlapY = checkContainedInRange(other.m_y, other.m_y + other.m_texHeight, m_y);
    overlapY |= checkContainedInRange(other.m_y, other.m_y + other.m_texHeight, m_y + m_texHeight);

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
