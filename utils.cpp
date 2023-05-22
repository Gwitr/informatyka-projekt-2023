#include "utils.hpp"

// All the constructor of sdl_error needs to do is to save the error string somewhere
sdl_error::sdl_error(const char *what)
    : m_what{what}
{}

// All the what() getter needs to do is to provide the previously-stored error message
const char *sdl_error::what() const throw()
{
    return m_what;
}

/*
 * Utility function for creating a new texture with new text in place of an old one
 */
void update_text(SDL_Renderer *renderer, TTF_Font *font, std::string_view newText, SDL_Color color, SDL_Texture *&textOut, int &textWidth, int &textHeight)
{
    // Simply delegate to render_text
    SDL_Texture *newTexture = render_text(renderer, font, newText, color, textWidth, textHeight);
    if (textOut)  // Only destroy the old texture if render_text succeeded
        sdlCall(SDL_DestroyTexture)(textOut);
    // Replace the old texture with the new one
    textOut = newTexture;
}
SDL_Texture *render_text(SDL_Renderer *renderer, TTF_Font *font, std::string_view newText, SDL_Color color)
{
    // This is a bit lazy -- we just call the version that does SDL_QueryTexture, but ignore the width and height that it gathers.
    int w, h;
    return render_text(renderer, font, newText, color, w, h);
}
SDL_Texture *render_text(SDL_Renderer *renderer, TTF_Font *font, std::string_view newText, SDL_Color color, int &textWidth, int &textHeight)
{
    // Use the SDL_ttf library to render the text into an SDL_Surface
    SDL_Surface *textSurface = sdlCall(TTF_RenderText_Solid)(font, newText.data(), color);

    // Turn that CPU-bound SDL_Surface into a GPU-bound SDL_Texture
    SDL_Texture *result = sdlCall(SDL_CreateTextureFromSurface)(renderer, textSurface);
    sdlCall(SDL_FreeSurface)(textSurface);  // The data is copied, we don't need the surface anymore

    // Gather the texture's size, and store it in textWidth and textHeight
    uint32_t _1;
    int _2;
    sdlCall(SDL_QueryTexture)(result, &_1, &_2, &textWidth, &textHeight);

    // Return the texture
    return result;
}

bool aabb_overlap(const SDL_Rect &rect1, const SDL_Rect &rect2)
{
    // This is quite bad. What else can I say
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
