#include "lxgui/impl/gui_sdl_font.hpp"
#include "lxgui/impl/gui_sdl_material.hpp"
#include <lxgui/gui_exception.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/utils_string.hpp>

#include <SDL.h>
#include <SDL_ttf.h>

static constexpr lxgui::uint uiMinChar = 32;
static constexpr lxgui::uint uiMaxChar = 255;

namespace lxgui {
namespace gui {
namespace sdl
{
font::font(SDL_Renderer* pRenderer, const std::string& sFontFile, uint uiSize,
    bool bPreMultipliedAlphaSupported) : uiSize_(uiSize)
{
    if (!TTF_WasInit() && TTF_Init() != 0)
    {
        throw gui::exception("gui::sdl::font", "Could not initialise SDL_ttf: "+
            std::string(TTF_GetError()));
    }

    uint uiSDLSize = floor(uiSize_ * 96.0/72.0);
    TTF_Font* pFont = TTF_OpenFont(sFontFile.c_str(), uiSDLSize);
    if (!pFont)
    {
        throw gui::exception("gui::sdl::font", "Could not load font file '"+
            sFontFile+"' at size "+utils::to_string(uiSize)+": "+
            std::string(TTF_GetError())+".");
    }

    // Add some space between letters to prevent artifacts
    uint uiSpacing = 1;

    int iMaxHeight = 0, iMaxWidth = 0;

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

    pTexture_ = std::make_shared<sdl::material>(pRenderer, uiFinalWidth, uiFinalHeight);

    uint uiTextureRealWidth = pTexture_->get_canvas_width();
    uint uiTextureRealHeight = pTexture_->get_canvas_height();
    float fTextureWidth = static_cast<float>(uiTextureRealWidth);
    float fTextureHeight = static_cast<float>(uiTextureRealHeight);

    uint uiPitch = 0;
    ub32color* pTexturePixels = pTexture_->lock_pointer(&uiPitch);
    std::fill(pTexturePixels, pTexturePixels + uiPitch * uiTextureRealHeight, ub32color(0,0,0,0));

    lCharacterList_.resize(uiMaxChar + 1);

    size_t x = 0, y = 0;
    uint uiLineMaxHeight = iMaxHeight;
    character_info mCI;

    const SDL_Color mColor = {255, 255, 255, 255};

    fYOffset_ = TTF_FontDescent(pFont);

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

        if (pGlyphSurface->format->format != SDL_PIXELFORMAT_ARGB8888)
        {
            throw gui::exception("gui::sdl::font", "SDL_ttf output format is not ARGB8888 (got "+
                utils::to_string(pGlyphSurface->format->format)+")");
        }

        const uint uiGlyphWidth = pGlyphSurface->w;
        const uint uiGlyphHeight = pGlyphSurface->h;

        uiLineMaxHeight = std::max(uiLineMaxHeight, uiGlyphHeight);

        // If at end of row, jump to next line
        if (x + uiGlyphWidth > (uint)uiTextureRealWidth - 1)
        {
            y += uiLineMaxHeight + uiSpacing;
            x = 0;
        }

        // SDL_ttf outputs glyphs in BGRA (little-endian) and we use RGBA;
        // this is fine because we always render glyphs in white, and don't care about
        // the color information.
        ub32color* pGlyphPixels = reinterpret_cast<ub32color*>(pGlyphSurface->pixels);
        int iGlyphPitch = pGlyphSurface->pitch/sizeof(ub32color);
        for (uint j = 0; j < uiGlyphHeight; ++j)
        for (uint i = 0; i < uiGlyphWidth; ++i)
            pTexturePixels[x + i + (y + j)*uiPitch] = pGlyphPixels[i + j*iGlyphPitch];

        SDL_FreeSurface(pGlyphSurface);

        mCI.mUVs.left   = x/fTextureWidth;
        mCI.mUVs.top    = y/fTextureHeight;
        mCI.mUVs.right  = (x + iAdvance)/fTextureWidth;
        mCI.mUVs.bottom = (y + uiGlyphHeight)/fTextureHeight;

        lCharacterList_[cp] = mCI;

        // Advance a column
        x += (std::max((uint)iAdvance, uiGlyphWidth) + uiSpacing);
    }

    // Get the width of a space ' ' (32) and tab '\t' (9)
    int iMinX = 0, iMaxX = 0, iMinY = 0, iMaxY = 0, iAdvance = 0;
    if (TTF_GlyphMetrics(pFont, 32, &iMinX, &iMaxX, &iMinY, &iMaxY, &iAdvance) == 0)
    {
        lCharacterList_[32].mUVs.left = 0.0f;
        lCharacterList_[32].mUVs.right = iAdvance/fTextureWidth;
        lCharacterList_[9].mUVs.left = 0.0f;
        lCharacterList_[9].mUVs.right = 4.0f*lCharacterList_[32].mUVs.right;
    }

    TTF_CloseFont(pFont);

    // Pre-multiply alpha
    if (bPreMultipliedAlphaSupported)
    {
        const uint uiArea = uiTextureRealWidth * uiTextureRealHeight;
        for (uint i = 0; i < uiArea; ++i)
        {
            float a = pTexturePixels[i].a/255.0f;
            pTexturePixels[i].r *= a;
            pTexturePixels[i].g *= a;
            pTexturePixels[i].b *= a;
        }
    }

    pTexture_->unlock_pointer();
}

uint font::get_size() const
{
    return uiSize_;
}

quad2f font::get_character_uvs(char32_t uiChar) const
{
    if (uiChar < 32 || uiChar > 255) return quad2f{};

    vector2f mTopLeft = pTexture_->get_canvas_uv(lCharacterList_[uiChar].mUVs.top_left(), true);
    vector2f mBottomRight = pTexture_->get_canvas_uv(lCharacterList_[uiChar].mUVs.bottom_right(), true);
    return quad2f(mTopLeft.x, mBottomRight.x, mTopLeft.y, mBottomRight.y);
}

quad2f font::get_character_bounds(char32_t uiChar) const
{
    if (uiChar < 32 || uiChar > 255) return quad2f{};

    const auto& mCharacterInfo = lCharacterList_[uiChar];
    const float fCharWidth = mCharacterInfo.mUVs.width()*pTexture_->get_rect().width();
    const float fCharHeight = mCharacterInfo.mUVs.height()*pTexture_->get_rect().height();

    return quad2f(0.0f, fCharWidth, fYOffset_, fYOffset_ + fCharHeight);
}

float font::get_character_width(char32_t uiChar) const
{
    if (uiChar < 32 || uiChar > 255) return 0.0f;

    return lCharacterList_[uiChar].mUVs.width()*pTexture_->get_rect().width();
}

float font::get_character_height(char32_t uiChar) const
{
    if (uiChar < 32 || uiChar > 255) return 0.0f;

    return lCharacterList_[uiChar].mUVs.height()*pTexture_->get_rect().height();
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

void font::update_texture(std::shared_ptr<gui::material> pMat)
{
    pTexture_ = std::static_pointer_cast<sdl::material>(pMat);
}

}
}
}
