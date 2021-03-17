#include "lxgui/impl/gui_sfml_font.hpp"
#include "lxgui/impl/gui_sfml_material.hpp"
#include <lxgui/gui_exception.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/utils_string.hpp>

constexpr uint uiMinChar = 32;
constexpr uint uiMaxChar = 255;

namespace lxgui {
namespace gui {
namespace sfml
{
font::font(const std::string& sFontFile, uint uiSize) : uiSize_(uiSize), uiSizeSFML_(floor(uiSize_ * 96.0/72.0))
{
    if (!mFont_.loadFromFile(sFontFile))
    {
        throw gui::exception("gui::sfml::font", "Could not load font file '"+sFontFile+"'.");
    }

    // Need to request in advance the glyphs that we will use
    // in order for SFLM to draw them on its internal texture
    for (uint cp = uiMinChar; cp <= uiMaxChar; ++cp)
    {
        mFont_.getGlyph(cp, uiSizeSFML_, false);
    }

    sf::Image mData = mFont_.getTexture(uiSizeSFML_).copyToImage();
    sfml::material::premultiply_alpha(mData);
    pTexture_ = std::make_shared<sfml::material>(mData);
}

quad2f font::get_character_uvs(char32_t uiChar) const
{
    if (uiChar < uiMinChar || uiChar > uiMaxChar) return quad2f{};

    const float fTexWidth = pTexture_->get_width();
    const float fTexHeight = pTexture_->get_height();

    const sf::IntRect& mSFRect = mFont_.getGlyph(uiChar, uiSizeSFML_, false).textureRect;

    quad2f mRect;
    mRect.left   = mSFRect.left / fTexWidth;
    mRect.right  = (mSFRect.left + mSFRect.width) / fTexWidth;
    mRect.top    = mSFRect.top / fTexHeight;
    mRect.bottom = (mSFRect.top + mSFRect.height) / fTexHeight;
    return mRect;
}

quad2f font::get_character_bounds(char32_t uiChar) const
{
    if (uiChar < uiMinChar || uiChar > uiMaxChar) return quad2f{};

    const float fYOffset = uiSize_;

    const sf::FloatRect& mSFRect = mFont_.getGlyph(uiChar, uiSizeSFML_, false).bounds;

    quad2f mRect;
    mRect.left   = 0.0f;
    mRect.right  = mSFRect.width;
    mRect.top    = mSFRect.top + fYOffset;
    mRect.bottom = mSFRect.top + mSFRect.height + fYOffset;
    return mRect;
}

float font::get_character_width(char32_t uiChar) const
{
    if (uiChar == 9) return 4*mFont_.getGlyph(uiMinChar, uiSizeSFML_, false).advance;
    if (uiChar < uiMinChar || uiChar > uiMaxChar) return 0.0f;

    return mFont_.getGlyph(uiChar, uiSizeSFML_, false).advance;
}

float font::get_character_height(char32_t uiChar) const
{
    if (uiChar < uiMinChar || uiChar > uiMaxChar) return 0.0f;

    return mFont_.getGlyph(uiChar, uiSizeSFML_, false).bounds.height;
}

float font::get_character_kerning(char32_t uiChar1, char32_t uiChar2) const
{
    if (uiChar1 < uiMinChar || uiChar1 > uiMaxChar || uiChar2 < uiMinChar || uiChar2 > uiMaxChar) return 0.0f;

    return mFont_.getKerning(uiChar1, uiChar2, uiSizeSFML_);
}

std::weak_ptr<gui::material> font::get_texture() const
{
    return pTexture_;
}
}
}
}
