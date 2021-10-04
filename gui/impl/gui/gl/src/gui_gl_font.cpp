#include "lxgui/impl/gui_gl_font.hpp"
#include "lxgui/impl/gui_gl_material.hpp"
#include <lxgui/gui_manager.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_filesystem.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_STROKER_H

// Convert fixed point to float point (from SDL_ttf)
#define FT_FLOOR(X) (((X) & -64) / 64)
#define FT_CEIL(X)  FT_FLOOR((X) + 63)

namespace lxgui {
namespace gui {
namespace gl
{
namespace
{
    // Global state for the Freetype library (one per thread)
    thread_local uint uiFTCount = 0u;
    thread_local FT_Library mSharedFT = nullptr;

    FT_Library get_freetype()
    {
        if (uiFTCount == 0u)
        {
            if (FT_Init_FreeType(&mSharedFT))
                throw lxgui::gui::exception("gui::gl::font", "Error initializing FreeType !");
        }

        ++uiFTCount;
        return mSharedFT;
    }

    void release_freetype()
    {
        if (uiFTCount != 0u)
        {
            --uiFTCount;
            if (uiFTCount == 0u)
                FT_Done_FreeType(mSharedFT);
        }
    }
}

font::font(const std::string& sFontFile, uint uiSize, uint uiOutline,
    const std::vector<code_point_range>& lCodePoints, char32_t uiDefaultCodePoint) :
    uiSize_(uiSize), uiOutline_(uiOutline), uiDefaultCodePoint_(uiDefaultCodePoint)
{
    // NOTE : Code inspired from Ogre::Font, from the OGRE3D graphics engine
    // http://www.ogre3d.org
    // ... and SFML
    // https://www.sfml-dev.org
    //
    // Some tweaking has been done to improve the text quality :
    //  - Disable hinting (FT_LOAD_NO_HINTING)
    //  - Character width is calculated as : max(x_bearing + width, advance),
    //    since advance sometimes doesn't cover the whole glyph
    //    (typical example is the 'w' character, in Consolas:9).

    if (!utils::file_exists(sFontFile))
        throw gui::exception("gui::gl::font", "Cannot find file \""+sFontFile+"\".");

    FT_Library mFT = get_freetype();
    FT_Stroker mStroker = nullptr;
    FT_Glyph mGlyph = nullptr;

    try
    {
        // Add some space between letters to prevent artifacts
        uint uiSpacing = 1;

        if (FT_New_Face(mFT, sFontFile.c_str(), 0, &mFace_))
        {
            throw gui::exception("gui::gl::font", "Error loading font : \""+sFontFile+
                "\" : cannot load face."
            );
        }

        if (uiOutline > 0)
        {
            if (FT_Stroker_New(mFT, &mStroker))
            {
                throw gui::exception("gui::gl::font", "Error loading font : \""+sFontFile+
                    "\" : cannot create stroker."
                );
            }
        }

        if (FT_Set_Char_Size(mFace_, uiSize*64, uiSize*64, 96, 96))
        {
            throw gui::exception("gui::gl::font", "Error loading font : \""+sFontFile+
                "\" : cannot set font size."
            );
        }

        int iMaxHeight = 0, iMaxWidth = 0, iMaxBearingY = 0;

        FT_Int32 iLoadFlags = FT_LOAD_TARGET_NORMAL | FT_LOAD_NO_HINTING;
        if (uiOutline != 0)
            iLoadFlags |= FT_LOAD_NO_BITMAP;

        // Calculate maximum width, height and bearing
        uint uiNumChar = 0;
        for (const code_point_range& mRange : lCodePoints)
        {
            for (char32_t uiCodePoint = mRange.uiFirst; uiCodePoint <= mRange.uiLast; ++uiCodePoint)
            {
                if (FT_Load_Char(mFace_, uiCodePoint, iLoadFlags))
                    continue;

                int iCharHeight = 2*(mFace_->glyph->bitmap.rows << 6)
                    - mFace_->glyph->metrics.horiBearingY;

                if (iCharHeight > iMaxHeight)
                    iMaxHeight = iCharHeight;

                if (mFace_->glyph->metrics.horiBearingY > iMaxBearingY)
                    iMaxBearingY = mFace_->glyph->metrics.horiBearingY;

                int iCharWidth = std::max(
                    int(mFace_->glyph->bitmap.width) + int(mFace_->glyph->metrics.horiBearingX >> 6),
                    int(mFace_->glyph->advance.x >> 6)
                );

                if (iCharWidth > iMaxWidth)
                    iMaxWidth = iCharWidth;

                ++uiNumChar;
            }
        }

        iMaxBearingY = iMaxBearingY >> 6;
        iMaxHeight = (iMaxHeight >> 6) + 2*uiOutline;
        iMaxWidth = iMaxWidth + 2*uiOutline;

        // Calculate the size of the texture
        size_t uiTexSize = (iMaxWidth + uiSpacing)*(iMaxHeight + uiSpacing)*uiNumChar;
        uint uiTexSide = static_cast<uint>(std::sqrt(static_cast<float>(uiTexSize)));

        // Add a bit of overhead since we won't be able to tile this area perfectly
        uiTexSide += std::max(iMaxWidth, iMaxHeight);
        uiTexSize = uiTexSide*uiTexSide;

        // Round up to nearest power of two
        {
            uint i = 1;
            while (uiTexSide > i)
                i *= 2;
            uiTexSide = i;
        }

        // Set up area as square
        size_t uiFinalWidth = uiTexSide;
        size_t uiFinalHeight = uiTexSide;

        // Reduce height if we don't actually need a square
        if (uiFinalWidth*uiFinalHeight/2 >= uiTexSize)
            uiFinalHeight = uiFinalHeight/2;

        std::vector<ub32color> lData(uiFinalWidth*uiFinalHeight);
        std::fill(lData.begin(), lData.end(), ub32color(0, 0, 0, 0));

        size_t x = 0, y = 0;

        if (FT_HAS_KERNING(mFace_))
            bKerning_ = true;

        if (FT_IS_SCALABLE(mFace_))
        {
            FT_Fixed mScale = mFace_->size->metrics.y_scale;
            fYOffset_ = FT_CEIL(FT_MulFix(mFace_->descender, mScale));
        }
        else
        {
            fYOffset_ = FT_CEIL(mFace_->size->metrics.descender);
        }

        for (const code_point_range& mRange : lCodePoints)
        {
            range_info mInfo;
            mInfo.mRange = mRange;
            mInfo.lData.resize(mRange.uiLast - mRange.uiFirst + 1);

            for (char32_t uiCodePoint = mRange.uiFirst; uiCodePoint <= mRange.uiLast; ++uiCodePoint)
            {
                character_info& mCI = mInfo.lData[uiCodePoint - mRange.uiFirst];
                mCI.uiCodePoint = uiCodePoint;

                if (FT_Load_Char(mFace_, uiCodePoint, iLoadFlags))
                {
                    gui::out << gui::warning << "gui::gl::font : Cannot load character " << uiCodePoint
                        << " in font \"" << sFontFile << "\"." << std::endl;
                    continue;
                }

                if (FT_Get_Glyph(mFace_->glyph, &mGlyph))
                {
                    gui::out << gui::warning << "gui::gl::font : Cannot get glyph for character " << uiCodePoint
                        << " in font \"" << sFontFile << "\"." << std::endl;
                    continue;
                }

                if (mGlyph->format == FT_GLYPH_FORMAT_OUTLINE && uiOutline > 0)
                {
                    FT_Stroker_Set(mStroker, static_cast<FT_Fixed>(uiOutline * static_cast<float>(1 << 6)),
                        FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
                    FT_Glyph_Stroke(&mGlyph, mStroker, true);
                }

                FT_Glyph_To_Bitmap(&mGlyph, FT_RENDER_MODE_NORMAL, nullptr, true);
                FT_Bitmap& mBitmap = reinterpret_cast<FT_BitmapGlyph>(mGlyph)->bitmap;

                size_t uiXBearing = std::max(0, int(mFace_->glyph->metrics.horiBearingX >> 6));

                FT_Int iAdvance = std::max(
                    int(uiXBearing + mBitmap.width), int(mFace_->glyph->advance.x >> 6)
                );

                // If at end of row, jump to next line
                if (x + iAdvance > uiFinalWidth - 1)
                {
                    y += iMaxHeight + uiSpacing;
                    x = 0;
                }

                ub32color::chanel* sBuffer = mBitmap.buffer;
                if (sBuffer)
                {
                    int iYBearing = iMaxBearingY - (mFace_->glyph->metrics.horiBearingY >> 6);

                    for (int j = 0; j < int(mBitmap.rows);  ++j)
                    {
                        uint uiRowOffset = (y + j + iYBearing)*uiFinalWidth;
                        for (int i = 0; i < int(mBitmap.width); ++i, ++sBuffer)
                            lData[x + i + uiXBearing + uiRowOffset] = ub32color(255, 255, 255, *sBuffer);
                    }
                }

                FT_Done_Glyph(mGlyph);
                mGlyph = nullptr;

                mCI.mUVs.left   = x/float(uiFinalWidth);
                mCI.mUVs.top    = y/float(uiFinalHeight);
                mCI.mUVs.right  = (x + iAdvance)/float(uiFinalWidth);
                mCI.mUVs.bottom = (y + iMaxHeight)/float(uiFinalHeight);

                // Advance a column
                x += (iAdvance + uiSpacing);
            }

            lRangeList_.push_back(std::move(mInfo));
        }

        FT_Stroker_Done(mStroker);

        gl::material::premultiply_alpha(lData);

        pTexture_ = std::make_shared<gl::material>(uiFinalWidth, uiFinalHeight);
        pTexture_->update_texture(lData.data());
    }
    catch (...)
    {
        if (mGlyph) FT_Done_Glyph(mGlyph);
        if (mStroker) FT_Stroker_Done(mStroker);
        if (mFace_) FT_Done_Face(mFace_);
        release_freetype();
        throw;
    }
}

font::~font()
{
    if (mFace_) FT_Done_Face(mFace_);
    release_freetype();
}

uint font::get_size() const
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

quad2f font::get_character_uvs(char32_t uiChar) const
{
    const character_info* pChar = get_character_(uiChar);
    if (!pChar)
        return quad2f{};

    vector2f mTopLeft = pTexture_->get_canvas_uv(pChar->mUVs.top_left(), true);
    vector2f mBottomRight = pTexture_->get_canvas_uv(pChar->mUVs.bottom_right(), true);
    return quad2f(mTopLeft.x, mBottomRight.x, mTopLeft.y, mBottomRight.y);
}

float font::get_character_width_(const character_info& mChar) const
{
    return mChar.mUVs.width()*pTexture_->get_rect().width() - 2*uiOutline_;
}

float font::get_character_height_(const character_info& mChar) const
{
    return mChar.mUVs.height()*pTexture_->get_rect().height() - 2*uiOutline_;
}

quad2f font::get_character_bounds(char32_t uiChar) const
{
    const character_info* pChar = get_character_(uiChar);
    if (!pChar)
        return quad2f{};

    const float fCharWidth = get_character_width_(*pChar);
    const float fCharHeight = get_character_height_(*pChar);
    const float fOffset = static_cast<float>(uiOutline_);

    return quad2f(-fOffset, fOffset + fCharWidth,
        -fOffset + fYOffset_, fOffset + fYOffset_ + fCharHeight);
}

float font::get_character_width(char32_t uiChar) const
{
    const character_info* pChar = get_character_(uiChar);
    if (!pChar)
        return 0.0f;

    return get_character_width_(*pChar);
}

float font::get_character_height(char32_t uiChar) const
{
    const character_info* pChar = get_character_(uiChar);
    if (!pChar)
        return 0.0f;

    return get_character_height_(*pChar);
}

float font::get_character_kerning(char32_t uiChar1, char32_t uiChar2) const
{
    if (bKerning_)
    {
        FT_Vector mKerning;
        uint uiPrev = FT_Get_Char_Index(mFace_, uiChar1);
        uint uiNext = FT_Get_Char_Index(mFace_, uiChar2);
        if (!FT_Get_Kerning(mFace_, uiPrev, uiNext, FT_KERNING_UNFITTED, &mKerning))
            return mKerning.x >> 6;
        else
            return 0.0f;
    }
    else
        return 0.0f;
}

std::weak_ptr<gui::material> font::get_texture() const
{
    return pTexture_;
}

void font::update_texture(std::shared_ptr<gui::material> pMat)
{
    pTexture_ = std::static_pointer_cast<gl::material>(pMat);
}

}
}
}
