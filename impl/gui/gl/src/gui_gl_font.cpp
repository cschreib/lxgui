#include "lxgui/impl/gui_gl_font.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_gl_material.hpp"
#include "lxgui/utils_filesystem.hpp"
#include "lxgui/utils_string.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_STROKER_H

// Convert fixed point to floating point
template<std::size_t Point, typename T>
float ft_float(T iValue) {
    return static_cast<float>(iValue) / static_cast<float>(1 << Point);
}

// Convert integer or floating point to fixed point
template<std::size_t Point, typename T>
FT_Fixed ft_fixed(T iValue) {
    return static_cast<FT_Fixed>(
        std::round(static_cast<float>(iValue) * static_cast<float>(1 << Point)));
}

// Convert fixed point to integer pixels
template<std::size_t Point, typename T>
T ft_floor(T iValue) {
    return (iValue & -(1 << Point)) / (1 << Point);
}

template<std::size_t Point, typename T>
T ft_ceil(T iValue) {
    return ft_floor<Point>(iValue + (1 << Point) - 1);
}

template<std::size_t Point, typename T>
T ft_round(T iValue) {
    return std::round(ft_float<Point>(iValue));
}

namespace lxgui { namespace gui { namespace gl {

namespace {

// Global state for the Freetype library (one per thread)
thread_local std::size_t uiFTCount = 0u;
thread_local FT_Library  mSharedFT = nullptr;

FT_Library get_freetype() {
    if (uiFTCount == 0u) {
        if (FT_Init_FreeType(&mSharedFT))
            throw lxgui::gui::exception("gui::gl::font", "Error initializing FreeType !");
    }

    ++uiFTCount;
    return mSharedFT;
}

void release_freetype() {
    if (uiFTCount != 0u) {
        --uiFTCount;
        if (uiFTCount == 0u)
            FT_Done_FreeType(mSharedFT);
    }
}

} // namespace

font::font(
    const std::string&                   sFontFile,
    std::size_t                          uiSize,
    std::size_t                          uiOutline,
    const std::vector<code_point_range>& lCodePoints,
    char32_t                             uiDefaultCodePoint) :
    uiSize_(uiSize), uiDefaultCodePoint_(uiDefaultCodePoint) {
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
        throw gui::exception("gui::gl::font", "Cannot find file \"" + sFontFile + "\".");

    FT_Library mFT      = get_freetype();
    FT_Stroker mStroker = nullptr;
    FT_Glyph   mGlyph   = nullptr;

    try {
        // Add some space between letters to prevent artifacts
        const std::size_t uiSpacing = 1;

        if (FT_New_Face(mFT, sFontFile.c_str(), 0, &mFace_) != 0) {
            throw gui::exception(
                "gui::gl::font", "Error loading font : \"" + sFontFile + "\" : cannot load face.");
        }

        if (uiOutline > 0) {
            if (FT_Stroker_New(mFT, &mStroker) != 0) {
                throw gui::exception(
                    "gui::gl::font",
                    "Error loading font : \"" + sFontFile + "\" : cannot create stroker.");
            }
        }

        if (FT_Select_Charmap(mFace_, FT_ENCODING_UNICODE) != 0) {
            throw gui::exception(
                "gui::gl::font", "Error loading font : \"" + sFontFile +
                                     "\" : cannot select Unicode character map.");
        }

        if (FT_Set_Pixel_Sizes(mFace_, 0, uiSize) != 0) {
            throw gui::exception(
                "gui::gl::font",
                "Error loading font : \"" + sFontFile + "\" : cannot set font size.");
        }

        FT_Int32 iLoadFlags = FT_LOAD_TARGET_NORMAL | FT_LOAD_NO_HINTING;
        if (uiOutline != 0)
            iLoadFlags |= FT_LOAD_NO_BITMAP;

        // Calculate maximum width, height and bearing
        std::size_t uiMaxHeight = 0, uiMaxWidth = 0;
        std::size_t uiNumChar = 0;
        for (const code_point_range& mRange : lCodePoints) {
            for (char32_t uiCodePoint = mRange.uiFirst; uiCodePoint <= mRange.uiLast;
                 ++uiCodePoint) {
                if (FT_Load_Char(mFace_, uiCodePoint, iLoadFlags) != 0)
                    continue;

                if (FT_Get_Glyph(mFace_->glyph, &mGlyph) != 0)
                    continue;

                if (mGlyph->format == FT_GLYPH_FORMAT_OUTLINE && uiOutline > 0) {
                    FT_Stroker_Set(
                        mStroker, ft_fixed<6>(uiOutline), FT_STROKER_LINECAP_ROUND,
                        FT_STROKER_LINEJOIN_ROUND, 0);
                    FT_Glyph_StrokeBorder(&mGlyph, mStroker, false, true);
                }

                FT_Glyph_To_Bitmap(&mGlyph, FT_RENDER_MODE_NORMAL, nullptr, true);
                const FT_Bitmap& mBitmap = reinterpret_cast<FT_BitmapGlyph>(mGlyph)->bitmap;

                if (mBitmap.rows > uiMaxHeight)
                    uiMaxHeight = mBitmap.rows;

                if (mBitmap.width > uiMaxWidth)
                    uiMaxWidth = mBitmap.width;

                ++uiNumChar;

                FT_Done_Glyph(mGlyph);
                mGlyph = nullptr;
            }
        }

        uiMaxHeight = uiMaxHeight + 2 * uiOutline;
        uiMaxWidth  = uiMaxWidth + 2 * uiOutline;

        // Calculate the size of the texture
        std::size_t uiTexSize = (uiMaxWidth + uiSpacing) * (uiMaxHeight + uiSpacing) * uiNumChar;
        std::size_t uiTexSide = static_cast<std::size_t>(std::sqrt(static_cast<float>(uiTexSize)));

        // Add a bit of overhead since we won't be able to tile this area perfectly
        uiTexSide += std::max(uiMaxWidth, uiMaxHeight);
        uiTexSize = uiTexSide * uiTexSide;

        // Round up to nearest power of two
        {
            std::size_t i = 1;
            while (uiTexSide > i)
                i *= 2;
            uiTexSide = i;
        }

        // Set up area as square
        std::size_t uiFinalWidth  = uiTexSide;
        std::size_t uiFinalHeight = uiTexSide;

        // Reduce height if we don't actually need a square
        if (uiFinalWidth * uiFinalHeight / 2 >= uiTexSize)
            uiFinalHeight = uiFinalHeight / 2;

        std::vector<ub32color> lData(uiFinalWidth * uiFinalHeight);
        std::fill(lData.begin(), lData.end(), ub32color(0, 0, 0, 0));

        std::size_t x = 0, y = 0;

        if (FT_HAS_KERNING(mFace_))
            bKerning_ = true;

        float fYOffset = 0.0f;
        if (FT_IS_SCALABLE(mFace_)) {
            FT_Fixed mScale = mFace_->size->metrics.y_scale;
            fYOffset        = ft_ceil<6>(FT_MulFix(mFace_->ascender, mScale)) +
                       ft_ceil<6>(FT_MulFix(mFace_->descender, mScale));
        } else {
            fYOffset = ft_ceil<6>(mFace_->size->metrics.ascender) +
                       ft_ceil<6>(mFace_->size->metrics.descender);
        }

        for (const code_point_range& mRange : lCodePoints) {
            range_info mInfo;
            mInfo.mRange = mRange;
            mInfo.lData.resize(mRange.uiLast - mRange.uiFirst + 1);

            for (char32_t uiCodePoint = mRange.uiFirst; uiCodePoint <= mRange.uiLast;
                 ++uiCodePoint) {
                character_info& mCI = mInfo.lData[uiCodePoint - mRange.uiFirst];
                mCI.uiCodePoint     = uiCodePoint;

                if (FT_Load_Char(mFace_, uiCodePoint, iLoadFlags) != 0) {
                    gui::out << gui::warning << "gui::gl::font : Cannot load character "
                             << uiCodePoint << " in font \"" << sFontFile << "\"." << std::endl;
                    continue;
                }

                if (FT_Get_Glyph(mFace_->glyph, &mGlyph) != 0) {
                    gui::out << gui::warning << "gui::gl::font : Cannot get glyph for character "
                             << uiCodePoint << " in font \"" << sFontFile << "\"." << std::endl;
                    continue;
                }

                if (mGlyph->format == FT_GLYPH_FORMAT_OUTLINE && uiOutline > 0) {
                    FT_Stroker_Set(
                        mStroker, ft_fixed<6>(uiOutline), FT_STROKER_LINECAP_ROUND,
                        FT_STROKER_LINEJOIN_ROUND, 0);
                    FT_Glyph_Stroke(&mGlyph, mStroker, true);
                }

                // Warning: after this line, do not use mGlyph! Use mBitmapGlyph.root
                FT_Glyph_To_Bitmap(&mGlyph, FT_RENDER_MODE_NORMAL, nullptr, true);
                FT_BitmapGlyph mBitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(mGlyph);

                const FT_Bitmap& mBitmap = mBitmapGlyph->bitmap;

                // If at end of row, jump to next line
                if (x + mBitmap.width > uiFinalWidth - 1) {
                    y += uiMaxHeight + uiSpacing;
                    x = 0;
                }

                // Some characters do not have a bitmap, like white spaces.
                // This is legal, and we should just have blank geometry for them.
                const ub32color::chanel* sBuffer = mBitmap.buffer;
                if (sBuffer) {
                    for (std::size_t j = 0; j < mBitmap.rows; ++j) {
                        std::size_t uiRowOffset = (y + j) * uiFinalWidth + x;
                        for (std::size_t i = 0; i < mBitmap.width; ++i, ++sBuffer)
                            lData[i + uiRowOffset] = ub32color(255, 255, 255, *sBuffer);
                    }
                }

                mCI.mUVs.left   = x / float(uiFinalWidth);
                mCI.mUVs.top    = y / float(uiFinalHeight);
                mCI.mUVs.right  = (x + mBitmap.width) / float(uiFinalWidth);
                mCI.mUVs.bottom = (y + mBitmap.rows) / float(uiFinalHeight);

                mCI.mRect.left   = mBitmapGlyph->left;
                mCI.mRect.right  = mCI.mRect.left + mBitmap.width;
                mCI.mRect.top    = fYOffset - mBitmapGlyph->top;
                mCI.mRect.bottom = mCI.mRect.top + mBitmap.rows;

                mCI.fAdvance = ft_round<16>(mBitmapGlyph->root.advance.x);

                // Advance a column
                x += mBitmap.width + uiSpacing;

                FT_Done_Glyph(mGlyph);
                mGlyph = nullptr;
            }

            lRangeList_.push_back(std::move(mInfo));
        }

        FT_Stroker_Done(mStroker);

        gl::material::premultiply_alpha(lData);

        pTexture_ = std::make_shared<gl::material>(vector2ui(uiFinalWidth, uiFinalHeight));
        pTexture_->update_texture(lData.data());
    } catch (...) {
        if (mGlyph)
            FT_Done_Glyph(mGlyph);
        if (mStroker)
            FT_Stroker_Done(mStroker);
        if (mFace_)
            FT_Done_Face(mFace_);
        release_freetype();
        throw;
    }
}

font::~font() {
    if (mFace_)
        FT_Done_Face(mFace_);
    release_freetype();
}

std::size_t font::get_size() const {
    return uiSize_;
}

const font::character_info* font::get_character_(char32_t uiChar) const {
    for (const auto& mInfo : lRangeList_) {
        if (uiChar < mInfo.mRange.uiFirst || uiChar > mInfo.mRange.uiLast)
            continue;

        return &mInfo.lData[uiChar - mInfo.mRange.uiFirst];
    }

    if (uiChar != uiDefaultCodePoint_)
        return get_character_(uiDefaultCodePoint_);
    else
        return nullptr;
}

bounds2f font::get_character_uvs(char32_t uiChar) const {
    const character_info* pChar = get_character_(uiChar);
    if (!pChar)
        return bounds2f{};

    vector2f mTopLeft     = pTexture_->get_canvas_uv(pChar->mUVs.top_left(), true);
    vector2f mBottomRight = pTexture_->get_canvas_uv(pChar->mUVs.bottom_right(), true);
    return bounds2f(mTopLeft.x, mBottomRight.x, mTopLeft.y, mBottomRight.y);
}

bounds2f font::get_character_bounds(char32_t uiChar) const {
    const character_info* pChar = get_character_(uiChar);
    if (!pChar)
        return bounds2f{};

    return pChar->mRect;
}

float font::get_character_width(char32_t uiChar) const {
    const character_info* pChar = get_character_(uiChar);
    if (!pChar)
        return 0.0f;

    return pChar->fAdvance;
}

float font::get_character_height(char32_t uiChar) const {
    const character_info* pChar = get_character_(uiChar);
    if (!pChar)
        return 0.0f;

    return pChar->mRect.height();
}

float font::get_character_kerning(char32_t uiChar1, char32_t uiChar2) const {
    if (bKerning_) {
        FT_Vector mKerning;
        FT_UInt   uiPrev = FT_Get_Char_Index(mFace_, uiChar1);
        FT_UInt   uiNext = FT_Get_Char_Index(mFace_, uiChar2);
        if (FT_Get_Kerning(mFace_, uiPrev, uiNext, FT_KERNING_UNFITTED, &mKerning) != 0)
            return ft_round<6>(mKerning.x);
        else
            return 0.0f;
    } else
        return 0.0f;
}

std::weak_ptr<gui::material> font::get_texture() const {
    return pTexture_;
}

void font::update_texture(std::shared_ptr<gui::material> pMat) {
    pTexture_ = std::static_pointer_cast<gl::material>(pMat);
}

}}} // namespace lxgui::gui::gl
