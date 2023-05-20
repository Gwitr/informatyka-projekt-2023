#include "utils.hpp"

sdl_error::sdl_error(const char *what)
    : m_what{what}
{}

const char *sdl_error::what() const throw()
{
    return m_what;
}

/*
 * Utility function for creating a new texture with new text in place of an old one
 */
void update_text(SDL_Renderer *renderer, TTF_Font *font, std::string_view newText, SDL_Color color, SDL_Texture *&textOut, int &textWidth, int &textHeight)
{
    SDL_Texture *newTexture = render_text(renderer, font, newText, color, textWidth, textHeight);
    if (textOut)
        sdlCall(SDL_DestroyTexture)(textOut);
    textOut = newTexture;
}
SDL_Texture *render_text(SDL_Renderer *renderer, TTF_Font *font, std::string_view newText, SDL_Color color)
{
    int w, h;
    return render_text(renderer, font, newText, color, w, h);
}
SDL_Texture *render_text(SDL_Renderer *renderer, TTF_Font *font, std::string_view newText, SDL_Color color, int &textWidth, int &textHeight)
{
    SDL_Surface *textSurface = sdlCall(TTF_RenderText_Solid)(font, newText.data(), color);

    SDL_Texture *result = sdlCall(SDL_CreateTextureFromSurface)(renderer, textSurface);
    sdlCall(SDL_FreeSurface)(textSurface);

    uint32_t _1;
    int _2;
    sdlCall(SDL_QueryTexture)(result, &_1, &_2, &textWidth, &textHeight);

    return result;
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
