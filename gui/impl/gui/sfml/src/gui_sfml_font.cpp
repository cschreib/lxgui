#include "lxgui/impl/gui_sfml_font.hpp"
#include "lxgui/impl/gui_sfml_material.hpp"
#include <lxgui/gui_manager.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/utils_string.hpp>

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
    for (uint cp = 32; cp <= 255; ++cp)
    {
        mFont_.getGlyph(cp, uiSizeSFML_, false);
    }

    sf::Image mData = mFont_.getTexture(uiSizeSFML_).copyToImage();
    sfml::material::premultiply_alpha(mData);
    pTexture_ = utils::refptr<sfml::material>(new sfml::material(mData));
}

quad2f font::get_character_uvs(char32_t uiChar) const
{
    if (uiChar < 32 || uiChar > 255) return quad2f{};
    quad2f mRect;
    const float fTexWidth = pTexture_->get_width();
    const float fTexHeight = pTexture_->get_height();
    const sf::IntRect& mSFRect = mFont_.getGlyph(uiChar, uiSizeSFML_, false).textureRect;
    mRect.left   = mSFRect.left / fTexWidth;
    mRect.right  = (mSFRect.left + mSFRect.width) / fTexWidth;
    mRect.top    = mSFRect.top / fTexHeight;
    mRect.bottom = (mSFRect.top + mSFRect.height) / fTexHeight;
    return mRect;
}

quad2f font::get_character_bounds(char32_t uiChar) const
{
    if (uiChar < 32 || uiChar > 255) return quad2f{};

    quad2f mRect;
    const sf::FloatRect& mSFRect = mFont_.getGlyph(uiChar, uiSizeSFML_, false).bounds;
    const float fYOffset = uiSize_;
    mRect.left   = 0.0f;
    mRect.right  = mSFRect.width;
    mRect.top    = mSFRect.top + fYOffset;
    mRect.bottom = mSFRect.top + mSFRect.height + fYOffset;
    return mRect;
}

float font::get_character_width(char32_t uiChar) const
{
    if (uiChar == 9)  return 4*mFont_.getGlyph(32, uiSizeSFML_, false).advance;
    if (uiChar < 32 || uiChar > 255) return 0.0f;
    return mFont_.getGlyph(uiChar, uiSizeSFML_, false).advance;
}

float font::get_character_height(char32_t uiChar) const
{
    if (uiChar < 32 || uiChar > 255) return 0.0f;
    return mFont_.getGlyph(uiChar, uiSizeSFML_, false).bounds.height;
}

float font::get_character_kerning(char32_t uiChar1, char32_t uiChar2) const
{
    if (uiChar1 < 32 || uiChar1 > 255) return 0.0f;
    if (uiChar2 < 32 || uiChar2 > 255) return 0.0f;
    return mFont_.getKerning(uiChar1, uiChar2, uiSizeSFML_);
}

utils::wptr<gui::material> font::get_texture() const
{
    return pTexture_;
}
}
}
