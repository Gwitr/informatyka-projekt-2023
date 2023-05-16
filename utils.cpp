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
    SDL_Surface *textSurface = sdlCall(TTF_RenderText_Solid)(font, newText.data(), color);
    if (textOut)
        sdlCall(SDL_DestroyTexture)(textOut);
    textOut = sdlCall(SDL_CreateTextureFromSurface)(renderer, textSurface);
    sdlCall(SDL_FreeSurface)(textSurface);

    uint32_t _1;
    int _2;
    sdlCall(SDL_QueryTexture)(textOut, &_1, &_2, &textWidth, &textHeight);
}
