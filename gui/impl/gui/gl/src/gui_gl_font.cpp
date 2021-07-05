#include "lxgui/impl/gui_gl_font.hpp"
#include "lxgui/impl/gui_gl_material.hpp"
#include <lxgui/gui_manager.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_filesystem.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

// Convert fixed point to float point (from SDL_ttf)
#define FT_FLOOR(X) (((X) & -64) / 64)
#define FT_CEIL(X)  FT_FLOOR((X) + 63)

static constexpr lxgui::uint uiMinChar = 32;
static constexpr lxgui::uint uiMaxChar = 255;

namespace lxgui {
namespace gui {
namespace gl
{
font::font(const std::string& sFontFile, uint uiSize) : uiSize_(uiSize)
{
    // NOTE : Code inspired from Ogre::Font, from the OGRE3D graphics engine
    // http://www.ogre3d.org
    //
    // Some tweaking has been done to improve the text quality :
    //  - Disable hinting (FT_LOAD_NO_HINTING)
    //  - Character width is calculated as : max(x_bearing + width, advance),
    //    since advance sometimes doesn't cover the whole glyph
    //    (typical example is the 'w' character, in Consolas:9).

    if (!utils::file_exists(sFontFile))
        throw gui::exception("gui::gl::font", "Cannot find file \""+sFontFile+"\".");

    FT_Library mFT;
    if (FT_Init_FreeType(&mFT))
        throw gui::exception("gui::gl::font", "Error initializing FreeType !");

    FT_Face mFace;

    // Add some space between letters to prevent artifacts
    uint uiSpacing = 1;

    if (FT_New_Face(mFT, sFontFile.c_str(), 0, &mFace))
    {
        throw gui::exception("gui::gl::font", "Error loading font : \""+sFontFile+
            "\" : cannot load face."
        );
    }

    if (FT_Set_Char_Size(mFace, uiSize*64, uiSize*64, 96, 96))
    {
        throw gui::exception("gui::gl::font", "Error loading font : \""+sFontFile+
            "\" : cannot set font size."
        );
    }

    int iMaxHeight = 0, iMaxWidth = 0, iMaxBearingY = 0;

    const FT_Int32 iLoadFlags = FT_LOAD_RENDER | FT_LOAD_NO_HINTING;

    // Calculate maximum width, height and bearing
    for (uint cp = uiMinChar; cp <= uiMaxChar; ++cp)
    {
        if (FT_Load_Char(mFace, cp, iLoadFlags))
            continue;

        int iCharHeight = 2*(mFace->glyph->bitmap.rows << 6) - mFace->glyph->metrics.horiBearingY;
        if (iCharHeight > iMaxHeight)
            iMaxHeight = iCharHeight;

        if (mFace->glyph->metrics.horiBearingY > iMaxBearingY)
            iMaxBearingY = mFace->glyph->metrics.horiBearingY;

        int iCharWidth = std::max(
            int(mFace->glyph->bitmap.width) + int(mFace->glyph->metrics.horiBearingX >> 6),
            int(mFace->glyph->advance.x >> 6)
        );

        if (iCharWidth > iMaxWidth)
            iMaxWidth = iCharWidth;
    }

    iMaxBearingY = iMaxBearingY >> 6;
    iMaxHeight = iMaxHeight >> 6;

    // Calculate the size of the texture
    size_t uiTexSize = (iMaxWidth + uiSpacing)*(iMaxHeight + uiSpacing)*(uiMaxChar - uiMinChar + 1);

    uint uiTexSide = static_cast<uint>(::sqrt((float)uiTexSize));
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

    pTexture_ = std::make_shared<gl::material>(uiFinalWidth, uiFinalHeight);

    fTextureWidth_ = static_cast<float>(pTexture_->get_canvas_width());
    fTextureHeight_ = static_cast<float>(pTexture_->get_canvas_height());

    std::fill(pTexture_->get_data().begin(), pTexture_->get_data().end(), ub32color(0, 0, 0, 0));

    lCharacterList_.resize(uiMaxChar + 1);

    size_t x = 0, y = 0;
    character_info mCI;

    if (FT_HAS_KERNING(mFace))
        bKerning_ = true;

    if (FT_IS_SCALABLE(mFace))
    {
        FT_Fixed mScale = mFace->size->metrics.y_scale;
        fYOffset_ = FT_CEIL(FT_MulFix(mFace->descender, mScale));
    }
    else
    {
        fYOffset_ = FT_CEIL(mFace->size->metrics.descender);
    }

    if (bKerning_)
        mCI.lKerningInfo.resize(uiMaxChar + 1);

    for (uint cp = uiMinChar; cp <= uiMaxChar; ++cp)
    {
        mCI.uiCodePoint = cp;

        if (FT_Load_Char(mFace, cp, iLoadFlags))
        {
            gui::out << gui::warning << "gui::gl::font : Cannot load character " << cp
                << " in font \"" << sFontFile << "\"." << std::endl;
            continue;
        }

        size_t uiXBearing = std::max(0, int(mFace->glyph->metrics.horiBearingX >> 6));

        FT_Int iAdvance = std::max(int(uiXBearing + mFace->glyph->bitmap.width), int(mFace->glyph->advance.x >> 6));

        // If at end of row, jump to next line
        if (x + iAdvance > uiFinalWidth - 1)
        {
            y += iMaxHeight + uiSpacing;
            x = 0;
        }

        ub32color::chanel* sBuffer = mFace->glyph->bitmap.buffer;
        if (sBuffer)
        {
            int iYBearing = iMaxBearingY - (mFace->glyph->metrics.horiBearingY >> 6);

            for (int j = 0; j < int(mFace->glyph->bitmap.rows);  ++j)
            for (int i = 0; i < int(mFace->glyph->bitmap.width); ++i, ++sBuffer)
                pTexture_->set_pixel(x + i + uiXBearing, y + j + iYBearing, ub32color(255, 255, 255, *sBuffer));
        }

        if (bKerning_)
        {
            FT_Vector kern;
            unsigned int prev, next;
            for (uint cp2 = uiMinChar; cp2 <= uiMaxChar; ++cp2)
            {
                prev = FT_Get_Char_Index(mFace, cp);
                next = FT_Get_Char_Index(mFace, cp2);
                if (!FT_Get_Kerning(mFace, prev, next, FT_KERNING_UNFITTED, &kern))
                    mCI.lKerningInfo[cp2] = vector2f(kern.x >> 6, kern.y >> 6);
            }
        }

        mCI.mUVs.left   = x/float(uiFinalWidth);
        mCI.mUVs.top    = y/float(uiFinalHeight);
        mCI.mUVs.right  = (x + iAdvance)/float(uiFinalWidth);
        mCI.mUVs.bottom = (y + iMaxHeight)/float(uiFinalHeight);

        lCharacterList_[cp] = mCI;

        // Advance a column
        x += (iAdvance + uiSpacing);
    }

    // Get the width of a space ' ' (32) and tab '\t' (9)
    if (!FT_Load_Char(mFace, 32, iLoadFlags))
    {
        lCharacterList_[32].mUVs.left = 0.0f;
        lCharacterList_[32].mUVs.right = (mFace->glyph->advance.x >> 6)/float(uiFinalWidth);
        lCharacterList_[9].mUVs.left = 0.0f;
        lCharacterList_[9].mUVs.right = 4.0f*lCharacterList_[32].mUVs.right;
    }

    FT_Done_FreeType(mFT);

    pTexture_->premultiply_alpha();
    pTexture_->update_texture();
    pTexture_->clear_cache_data_();
}

uint font::get_size() const
{
    return uiSize_;
}

quad2f font::get_character_uvs(char32_t uiChar) const
{
    return lCharacterList_[uiChar].mUVs;
}

quad2f font::get_character_bounds(char32_t uiChar) const
{
    const float fCharWidth = get_character_width(uiChar);
    const float fCharHeight = get_character_height(uiChar);

    return quad2f(0.0f, fCharWidth, fYOffset_, fYOffset_ + fCharHeight);
}

float font::get_character_width(char32_t uiChar) const
{
    if (uiChar < uiMinChar || uiChar > uiMaxChar) return 0.0f;

    return lCharacterList_[uiChar].mUVs.width()*fTextureWidth_;
}

float font::get_character_height(char32_t uiChar) const
{
    if (uiChar < uiMinChar || uiChar > uiMaxChar) return 0.0f;

    return lCharacterList_[uiChar].mUVs.height()*fTextureHeight_;
}

float font::get_character_kerning(char32_t uiChar1, char32_t uiChar2) const
{
    if (uiChar1 < uiMinChar || uiChar1 > uiMaxChar || uiChar2 < uiMinChar || uiChar2 > uiMaxChar) return 0.0f;

    if (bKerning_)
        return lCharacterList_[uiChar1].lKerningInfo[uiChar2].x;
    else
        return 0.0f;
}

std::weak_ptr<gui::material> font::get_texture() const
{
    return pTexture_;
}
}
}
}
