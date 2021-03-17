#include "lxgui/impl/gui_sdl_font.hpp"
#include "lxgui/impl/gui_sdl_material.hpp"
#include <lxgui/gui_exception.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/utils_string.hpp>

#include <SDL.h>
#include <SDL_ttf.h>

constexpr uint uiMinChar = 32;
constexpr uint uiMaxChar = 255;

namespace lxgui {
namespace gui {
namespace sdl
{
font::font(SDL_Renderer* pRenderer, const std::string& sFontFile, uint uiSize) : uiSize_(uiSize)
{
    if (!TTF_WasInit() && TTF_Init() != 0)
    {
        throw gui::exception("gui::sdl::font", "Could not initialise SDL_ttf: "+
            std::string(TTF_GetError()));
    }

    TTF_Font* pFont = TTF_OpenFont(sFontFile.c_str(), uiSize);
    if (!pFont)
    {
        throw gui::exception("gui::sdl::font", "Could not load font file '"+
            sFontFile+"' at size "+utils::to_string(uiSize)+": "+
            std::string(TTF_GetError())+".");
    }

    // Add some space between letters to prevent artifacts
    uint uiSpacing = 1;

    int iMaxHeight = 0, iMaxWidth = 0, iMaxBearingY = 0;

    // Calculate maximum width and height
    for (uint cp = uiMinChar; cp <= uiMaxChar; ++cp)
    {
        const Uint16 uiAltChar = static_cast<Uint16>(cp);

        int iMinX = 0, iMaxX = 0, iMinY = 0, iMaxY = 0, iAdvance = 0;
        if (TTF_GlyphMetrics(pFont, uiAltChar, &iMinX, &iMaxX, &iMinY, &iMaxY, &iAdvance) != 0)
            continue;

        int iCharHeight = iMaxY - iMinY;
        if (iCharHeight > iMaxHeight)
            iMaxHeight = iCharHeight;

        int iCharWidth = iMaxX - iMinX;
        if (iCharWidth > iMaxWidth)
            iMaxWidth = iCharWidth;
    }

    // Calculate the size of the texture
    size_t uiTexSize = (iMaxWidth + uiSpacing)*(iMaxHeight + uiSpacing)*(uiMaxChar - uiMinChar + 1);

    uint uiTexSide = static_cast<uint>(std::sqrt((float)uiTexSize));
    uiTexSide += std::max(iMaxWidth, iMaxHeight);

    // Round up to nearest power of two
    {
        uint i = 1;
        while (uiTexSide > i)
            i *= 2;
        uiTexSide = i;
    }

    size_t uiFinalWidth, uiFinalHeight;
    if (uiTexSide*uiTexSide/2 >= uiTexSize)
        uiFinalHeight = uiTexSide/2;
    else
        uiFinalHeight = uiTexSide;

    uiFinalWidth = uiTexSide;

    fTextureWidth_ = static_cast<float>(uiFinalWidth);
    fTextureHeight_ = static_cast<float>(uiFinalHeight);

    pTexture_ = std::make_shared<sdl::material>(pRenderer, uiFinalWidth, uiFinalHeight);

    void* pPixelData = nullptr;
    int iPitch = 0;
    if (SDL_LockTexture(pTexture_->get_texture(), nullptr, &pPixelData, &iPitch) != 0)
    {
        throw gui::exception("gui::sdl::font", "Could not lock texture for copying pixels.");
    }

    ub32color* pTexturePixels = reinterpret_cast<ub32color*>(pPixelData);
    std::fill(pTexturePixels, pTexturePixels + uiFinalWidth * uiFinalHeight, ub32color(0,0,0,0));

    lCharacterList_.resize(uiMaxChar + 1);

    size_t x = 0, y = 0;
    character_info mCI;

    const SDL_Color mColor = {255, 255, 255, 255};

    for (uint cp = uiMinChar; cp <= uiMaxChar; ++cp)
    {
        mCI.uiCodePoint = cp;

        const Uint16 uiAltChar = static_cast<Uint16>(cp);

        int iMinX = 0, iMaxX = 0, iMinY = 0, iMaxY = 0, iAdvance = 0;
        if (TTF_GlyphMetrics(pFont, uiAltChar, &iMinX, &iMaxX, &iMinY, &iMaxY, &iAdvance) != 0)
        {
            gui::out << gui::warning << "gui::sdl::font : Cannot load character " << cp
                << " in font \"" << sFontFile << "\"." << std::endl;
            continue;
        }

        SDL_Surface* pGlyphSurface = TTF_RenderGlyph_Blended(pFont, uiAltChar, mColor);
        if (!pGlyphSurface)
        {
            gui::out << gui::warning << "gui::sdl::font : Cannot draw character " << cp
                << " in font \"" << sFontFile << "\"." << std::endl;
            continue;
        }

        const uint uiGlyphWidth = pGlyphSurface->w;
        const uint uiGlyphHeight = pGlyphSurface->h;

        // If at end of row, jump to next line
        if (x + uiGlyphWidth > uiFinalWidth - 1)
        {
            y += iMaxHeight + uiSpacing;
            x = 0;
        }

        ub32color* pGlyphPixels = reinterpret_cast<ub32color*>(pGlyphSurface->pixels);
        for (uint j = 0; j < uiGlyphHeight; ++j)
        for (uint i = 0; i < uiGlyphWidth; ++i, ++pGlyphPixels)
            pTexturePixels[x + i + (y + j) * uiFinalWidth] = *pGlyphPixels;

        SDL_FreeSurface(pGlyphSurface);

        mCI.fXOffset = iMinX;
        mCI.fYOffset = iMinY;

        mCI.mUVs.left   = x/float(uiFinalWidth);
        mCI.mUVs.top    = y/float(uiFinalHeight);
        mCI.mUVs.right  = (x + iAdvance)/float(uiFinalWidth);
        mCI.mUVs.bottom = (y + iMaxHeight)/float(uiFinalHeight);

        lCharacterList_[cp] = mCI;

        // Advance a column
        x += (uiGlyphWidth + uiSpacing);
    }

    // Get the width of a space ' ' (32) and tab '\t' (9)
    int iMinX = 0, iMaxX = 0, iMinY = 0, iMaxY = 0, iAdvance = 0;
    if (TTF_GlyphMetrics(pFont, 32, &iMinX, &iMaxX, &iMinY, &iMaxY, &iAdvance) == 0)
    {
        lCharacterList_[32].mUVs.left = 0.0f;
        lCharacterList_[32].mUVs.right = iAdvance/float(uiFinalWidth);
        lCharacterList_[9].mUVs.left = 0.0f;
        lCharacterList_[9].mUVs.right = 4.0f*lCharacterList_[32].mUVs.right;
    }

    TTF_CloseFont(pFont);

    // Pre-multiply alpha
    const uint uiArea = uiFinalWidth * uiFinalHeight;
    for (uint i = 0; i < uiArea; ++i)
    {
        float a = pTexturePixels[i].a/255.0f;
        pTexturePixels[i].r *= a;
        pTexturePixels[i].g *= a;
        pTexturePixels[i].b *= a;
    }

    SDL_UnlockTexture(pTexture_->get_texture());
}

quad2f font::get_character_uvs(char32_t uiChar) const
{
    if (uiChar < 32 || uiChar > 255) return quad2f{};

    return lCharacterList_[uiChar].mUVs;
}

quad2f font::get_character_bounds(char32_t uiChar) const
{
    if (uiChar < 32 || uiChar > 255) return quad2f{};

    const auto& mCharacterInfo = lCharacterList_[uiChar];
    const float fCharWidth = mCharacterInfo.mUVs.width()*fTextureWidth_;
    const float fCharHeight = mCharacterInfo.mUVs.height()*fTextureHeight_;

    return quad2f(mCharacterInfo.fXOffset, mCharacterInfo.fXOffset + fCharWidth,
        mCharacterInfo.fYOffset, mCharacterInfo.fYOffset + fCharHeight);
}

float font::get_character_width(char32_t uiChar) const
{
    if (uiChar < 32 || uiChar > 255) return 0.0f;

    return lCharacterList_[uiChar].mUVs.width()*fTextureWidth_;
}

float font::get_character_height(char32_t uiChar) const
{
    if (uiChar < 32 || uiChar > 255) return 0.0f;

    return lCharacterList_[uiChar].mUVs.height()*fTextureHeight_;
}

float font::get_character_kerning(char32_t, char32_t) const
{
    // Note: SDL_ttf does not expose kerning
    return 0.0f;
}

std::weak_ptr<gui::material> font::get_texture() const
{
    return pTexture_;
}
}
}
}
