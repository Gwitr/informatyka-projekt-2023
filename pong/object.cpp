#include "object.hpp"

// The constructor for an object object
object::object(SDL_Renderer *renderer, SDL_Texture *tex, int startX, int startY)
    // We simply initialize all member variables from the given arguments
    : m_renderer{renderer}, m_texture{tex}, m_x{(float)startX}, m_y{(float)startY},
        m_startX{startX}, m_startY{startY}
{
    // If we were given a texture (the caller is not required to provide one!)
    if (tex) {
        // Then query its size for convenience
        int _1;
        uint32_t _2;
        sdlCall(SDL_QueryTexture)(tex, &_2, &_1, &m_texWidth, &m_texHeight);
    }
    // Also query the window size for convenience
    sdlCall(SDL_GetRendererOutputSize)(renderer, &m_maxX, &m_maxY);
}

// The destructor that does nothing (it must be here if it's declared virtual)
object::~object()
{

}

// The default implementation for reset()
void object::reset()
{
    // Reset the object's position to its original state
    m_x = m_startX;
    m_y = m_startY;
}

// Default implementation for keyDown()
void object::keyDown(int key)
{
    // Make sure that we know the key is pressed
    m_keys[key] = true;
}

// Default implementation for keyUp()
void object::keyUp(int key)
{
    // Make sure that we know the key is no longer pressed
    m_keys[key] = false;
}

// The default behavior for drawing an object
void object::draw() const
{
    SDL_Rect srcrect = { 0, 0, m_texWidth, m_texHeight };
    SDL_Rect dstrect = { (int)m_x, (int)m_y, m_texWidth, m_texHeight };
    // Simply draw its texture
    sdlCall(SDL_RenderCopy)(m_renderer, m_texture, &srcrect, &dstrect);
}

// The default behavior for updating an object
void object::update(float deltaTime, const std::vector<std::unique_ptr<object>> &others)
{
    // Does absolutely nothing
    (void)deltaTime; (void)others;
}

// The default collidability of an object
bool object::can_collide() const
{
    // By default, all objects can collide
    return true;
}

// The default collision areas of an object
std::vector<SDL_Rect> object::get_collision_areas() const
{
    // By default, the only collision area present is the bounding rectangle of the texture
    return std::vector{ SDL_Rect{ (int)m_x, (int)m_y, m_texWidth, m_texHeight } };
}
