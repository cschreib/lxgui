#include "lxgui/impl/gui_sdl_font.hpp"
#include "lxgui/impl/gui_sdl_material.hpp"
#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/utils_string.hpp"

#include <SDL.h>
#include <SDL_ttf.h>

namespace lxgui {
namespace gui {
namespace sdl
{
font::font(SDL_Renderer* pRenderer, const std::string& sFontFile, std::size_t uiSize, std::size_t uiOutline,
    const std::vector<code_point_range>& lCodePoints, char32_t uiDefaultCodePoint,
    bool bPreMultipliedAlphaSupported) :
    uiSize_(uiSize), uiDefaultCodePoint_(uiDefaultCodePoint)
{
    if (!TTF_WasInit() && TTF_Init() != 0)
    {
        throw gui::exception("gui::sdl::font", "Could not initialise SDL_ttf: "+
            std::string(TTF_GetError()));
    }

    TTF_Font* pFont = TTF_OpenFont(sFontFile.c_str(), uiSize_);
    if (!pFont)
    {
        throw gui::exception("gui::sdl::font", "Could not load font file '"+
            sFontFile+"' at size "+utils::to_string(uiSize)+": "+
            std::string(TTF_GetError())+".");
    }

    if (uiOutline > 0)
        TTF_SetFontOutline(pFont, uiOutline);

    // Add some space between letters to prevent artifacts
    std::size_t uiSpacing = 1;

    int iMaxHeight = 0, iMaxWidth = 0;

    // Calculate maximum width and height
    std::size_t uiNumChar = 0;
    for (const code_point_range& mRange : lCodePoints)
    {
        for (char32_t uiCodePoint = mRange.uiFirst; uiCodePoint <= mRange.uiLast; ++uiCodePoint)
        {
            if (uiCodePoint > std::numeric_limits<Uint16>::max())
                break;

            const Uint16 uiAltChar = static_cast<Uint16>(uiCodePoint);

            int iMinX = 0, iMaxX = 0, iMinY = 0, iMaxY = 0, iAdvance = 0;
            if (TTF_GlyphMetrics(pFont, uiAltChar, &iMinX, &iMaxX, &iMinY, &iMaxY, &iAdvance) != 0)
                continue;

            int iCharHeight = iMaxY - iMinY;
            if (iCharHeight > iMaxHeight)
                iMaxHeight = iCharHeight;

            int iCharWidth = iMaxX - iMinX;
            if (iCharWidth > iMaxWidth)
                iMaxWidth = iCharWidth;

            ++uiNumChar;
        }
    }

    iMaxHeight = iMaxHeight + 2*uiOutline;
    iMaxWidth = iMaxWidth + 2*uiOutline;

    // Calculate the size of the texture
    std::size_t uiTexSize = (iMaxWidth + uiSpacing)*(iMaxHeight + uiSpacing)*uiNumChar;
    std::size_t uiTexSide = static_cast<std::size_t>(std::sqrt((float)uiTexSize));
    uiTexSide += std::max(iMaxWidth, iMaxHeight);

    // Round up to nearest power of two
    {
        std::size_t i = 1;
        while (uiTexSide > i)
            i *= 2;
        uiTexSide = i;
    }

    std::size_t uiFinalWidth, uiFinalHeight;
    if (uiTexSide*uiTexSide/2 >= uiTexSize)
        uiFinalHeight = uiTexSide/2;
    else
        uiFinalHeight = uiTexSide;

    uiFinalWidth = uiTexSide;

    pTexture_ = std::make_shared<sdl::material>(pRenderer, vector2ui(uiFinalWidth, uiFinalHeight));

    vector2ui mCanvasDimensions = pTexture_->get_canvas_dimensions();
    vector2f  mCanvasDimensionsFloat = vector2f(mCanvasDimensions);

    std::size_t uiPitch = 0;
    ub32color* pTexturePixels = pTexture_->lock_pointer(&uiPitch);
    std::fill(pTexturePixels, pTexturePixels + uiPitch * mCanvasDimensions.y, ub32color(0,0,0,0));

    std::size_t x = 0, y = 0;
    std::size_t uiLineMaxHeight = iMaxHeight;

    const SDL_Color mColor = {255, 255, 255, 255};

    const float fYOffset = TTF_FontDescent(pFont);

    for (const code_point_range& mRange : lCodePoints)
    {
        range_info mInfo;
        mInfo.mRange = mRange;
        mInfo.lData.resize(mRange.uiLast - mRange.uiFirst + 1);

        for (char32_t uiCodePoint = mRange.uiFirst; uiCodePoint <= mRange.uiLast; ++uiCodePoint)
        {
            character_info& mCI = mInfo.lData[uiCodePoint - mRange.uiFirst];
            mCI.uiCodePoint = uiCodePoint;

            if (uiCodePoint > std::numeric_limits<Uint16>::max())
            {
                gui::out << gui::warning << "gui::sdl::font : Cannot load character " << uiCodePoint
                    << " because SDL_ttf only accepts 16bit code points." << std::endl;
                break;
            }

            const Uint16 uiAltChar = static_cast<Uint16>(uiCodePoint);

            int iMinX = 0, iMaxX = 0, iMinY = 0, iMaxY = 0, iAdvance = 0;
            if (TTF_GlyphMetrics(pFont, uiAltChar, &iMinX, &iMaxX, &iMinY, &iMaxY, &iAdvance) != 0)
            {
                gui::out << gui::warning << "gui::sdl::font : Cannot load character " << uiCodePoint
                    << " in font \"" << sFontFile << "\"." << std::endl;
                continue;
            }

            SDL_Surface* pGlyphSurface = TTF_RenderGlyph_Blended(pFont, uiAltChar, mColor);
            if (!pGlyphSurface)
            {
                gui::out << gui::warning << "gui::sdl::font : Cannot draw character " << uiCodePoint
                    << " in font \"" << sFontFile << "\"." << std::endl;
                continue;
            }

            if (pGlyphSurface->format->format != SDL_PIXELFORMAT_ARGB8888)
            {
                throw gui::exception("gui::sdl::font", "SDL_ttf output format is not ARGB8888 (got "+
                    utils::to_string(pGlyphSurface->format->format)+")");
            }

            const std::size_t uiGlyphWidth = pGlyphSurface->w;
            const std::size_t uiGlyphHeight = pGlyphSurface->h;

            uiLineMaxHeight = std::max(uiLineMaxHeight, uiGlyphHeight);

            // If at end of row, jump to next line
            if (x + uiGlyphWidth > static_cast<std::size_t>(mCanvasDimensions.x) - 1)
            {
                y += uiLineMaxHeight + uiSpacing;
                x = 0;
            }

            // SDL_ttf outputs glyphs in BGRA (little-endian) and we use RGBA;
            // this is fine because we always render glyphs in white, and don't care about
            // the color information.
            ub32color* pGlyphPixels = reinterpret_cast<ub32color*>(pGlyphSurface->pixels);
            std::size_t iGlyphPitch = pGlyphSurface->pitch/sizeof(ub32color);
            for (std::size_t j = 0; j < uiGlyphHeight; ++j)
            for (std::size_t i = 0; i < uiGlyphWidth; ++i)
                pTexturePixels[x + i + (y + j)*uiPitch] = pGlyphPixels[i + j*iGlyphPitch];

            SDL_FreeSurface(pGlyphSurface);

            mCI.mUVs.left   = x/mCanvasDimensionsFloat.x;
            mCI.mUVs.top    = y/mCanvasDimensionsFloat.y;
            mCI.mUVs.right  = (x + uiGlyphWidth)/mCanvasDimensionsFloat.x;
            mCI.mUVs.bottom = (y + uiGlyphHeight)/mCanvasDimensionsFloat.y;

            // NB: do not use iMinX etc here; SDL_ttf has already applied them to the rendered glyph
            mCI.mRect.left = -static_cast<float>(uiOutline);
            mCI.mRect.right = mCI.mRect.left + uiGlyphWidth;
            mCI.mRect.top = fYOffset - static_cast<float>(uiOutline);
            mCI.mRect.bottom = mCI.mRect.top + uiGlyphHeight;

            mCI.fAdvance = iAdvance;

            // Advance a column
            x += uiGlyphWidth + uiSpacing;
        }

        lRangeList_.push_back(std::move(mInfo));
    }

    TTF_CloseFont(pFont);

    // Pre-multiply alpha
    if (bPreMultipliedAlphaSupported)
    {
        const std::size_t uiArea = mCanvasDimensions.x * mCanvasDimensions.y;
        for (std::size_t i = 0; i < uiArea; ++i)
        {
            float a = pTexturePixels[i].a/255.0f;
            pTexturePixels[i].r *= a;
            pTexturePixels[i].g *= a;
            pTexturePixels[i].b *= a;
        }
    }

    pTexture_->unlock_pointer();
}

std::size_t font::get_size() const
{
    return uiSize_;
}

const font::character_info* font::get_character_(char32_t uiChar) const
{
    for (const auto& mInfo : lRangeList_)
    {
        if (uiChar < mInfo.mRange.uiFirst || uiChar > mInfo.mRange.uiLast)
            continue;

        return &mInfo.lData[uiChar - mInfo.mRange.uiFirst];
    }

    if (uiChar != uiDefaultCodePoint_)
        return get_character_(uiDefaultCodePoint_);
    else
        return nullptr;
}

bounds2f font::get_character_uvs(char32_t uiChar) const
{
    const character_info* pChar = get_character_(uiChar);
    if (!pChar)
        return bounds2f{};

    vector2f mTopLeft = pTexture_->get_canvas_uv(pChar->mUVs.top_left(), true);
    vector2f mBottomRight = pTexture_->get_canvas_uv(pChar->mUVs.bottom_right(), true);
    return bounds2f(mTopLeft.x, mBottomRight.x, mTopLeft.y, mBottomRight.y);
}

bounds2f font::get_character_bounds(char32_t uiChar) const
{
    const character_info* pChar = get_character_(uiChar);
    if (!pChar)
        return bounds2f{};

    return pChar->mRect;
}

float font::get_character_width(char32_t uiChar) const
{
    const character_info* pChar = get_character_(uiChar);
    if (!pChar)
        return 0.0f;

    return pChar->fAdvance;
}

float font::get_character_height(char32_t uiChar) const
{
    const character_info* pChar = get_character_(uiChar);
    if (!pChar)
        return 0.0f;

    return pChar->mRect.height();
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
