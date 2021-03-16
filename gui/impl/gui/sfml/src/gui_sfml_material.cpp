#include "lxgui/impl/gui_sfml_material.hpp"
#include "lxgui/impl/gui_sfml_renderer.hpp"
#include <lxgui/gui_out.hpp>
#include <lxgui/gui_exception.hpp>
#include <lxgui/utils_string.hpp>

namespace lxgui {
namespace gui {
namespace sfml
{
material::material(uint uiWidth, uint uiHeight, bool bRenderTarget, wrap mWrap, filter mFilter) :
    mType_(type::TEXTURE)
{
    pTexData_ = std::unique_ptr<texture_data>(new texture_data());
    pTexData_->uiWidth_ = uiWidth;
    pTexData_->uiHeight_ = uiHeight;
    pTexData_->mWrap_ = mWrap;
    pTexData_->mFilter_ = mFilter;
    pTexData_->uiRealWidth_ = uiWidth;
    pTexData_->uiRealHeight_ = uiHeight;

    if (pTexData_->uiRealWidth_ > sf::Texture::getMaximumSize() ||
        pTexData_->uiRealHeight_ > sf::Texture::getMaximumSize())
    {
        throw gui::exception("gui::sfml::material", "Texture dimensions not supported by graphics card : ("+
            utils::to_string(pTexData_->uiRealWidth_)+" x "+
            utils::to_string(pTexData_->uiRealHeight_)+")."
        );
    }

    pTexData_->bRenderTarget_ = bRenderTarget;

    if (pTexData_->bRenderTarget_)
    {
        if (!pTexData_->mRenderTexture_.create(pTexData_->uiRealWidth_, pTexData_->uiRealHeight_))
        {
            throw gui::exception("gui::sfml::material", "Could not create render target with dimensions "+
                utils::to_string(uiWidth)+" x "+utils::to_string(uiHeight)+".");
        }
        pTexData_->mRenderTexture_.setSmooth(mFilter == filter::LINEAR);
        pTexData_->mRenderTexture_.setRepeated(mWrap == wrap::REPEAT);
    }
    else
    {
        if (!pTexData_->mTexture_.create(pTexData_->uiRealWidth_, pTexData_->uiRealHeight_))
        {
            throw gui::exception("gui::sfml::material", "Could not create texture with dimensions "+
                utils::to_string(uiWidth)+" x "+utils::to_string(uiHeight)+".");
        }
        pTexData_->mTexture_.setSmooth(mFilter == filter::LINEAR);
        pTexData_->mTexture_.setRepeated(mWrap == wrap::REPEAT);
    }
}

material::material(const sf::Image& mData, wrap mWrap, filter mFilter) :
    mType_(type::TEXTURE)
{
    pTexData_ = std::unique_ptr<texture_data>(new texture_data());

    pTexData_->bRenderTarget_ = false;
    pTexData_->mTexture_.loadFromImage(mData);
    pTexData_->mTexture_.setSmooth(mFilter == filter::LINEAR);
    pTexData_->mTexture_.setRepeated(mWrap == wrap::REPEAT);

    const uint uiWidth  = pTexData_->mTexture_.getSize().x;
    const uint uiHeight = pTexData_->mTexture_.getSize().y;
    pTexData_->uiWidth_ = uiWidth;
    pTexData_->uiHeight_ = uiHeight;
    pTexData_->mWrap_ = mWrap;
    pTexData_->mFilter_ = mFilter;
    pTexData_->uiRealWidth_ = uiWidth;
    pTexData_->uiRealHeight_ = uiHeight;
}

material::material(const std::string& sFileName, wrap mWrap, filter mFilter) :
    mType_(type::TEXTURE)
{
    pTexData_ = std::unique_ptr<texture_data>(new texture_data());

    pTexData_->bRenderTarget_ = false;
    sf::Image mData;
    if (!mData.loadFromFile(sFileName))
        throw utils::exception("gui::sfml::material", "loading failed: '"+sFileName+"'.");
    premultiply_alpha(mData);
    pTexData_->mTexture_.loadFromImage(mData);
    pTexData_->mTexture_.setSmooth(mFilter == filter::LINEAR);
    pTexData_->mTexture_.setRepeated(mWrap == wrap::REPEAT);

    const uint uiWidth  = pTexData_->mTexture_.getSize().x;
    const uint uiHeight = pTexData_->mTexture_.getSize().y;
    pTexData_->uiWidth_ = uiWidth;
    pTexData_->uiHeight_ = uiHeight;
    pTexData_->mWrap_ = mWrap;
    pTexData_->mFilter_ = mFilter;
    pTexData_->uiRealWidth_ = uiWidth;
    pTexData_->uiRealHeight_ = uiHeight;
}

material::material(const color& mColor) : mType_(type::COLOR)
{
    pColData_ = std::unique_ptr<color_data>(new color_data());
    pColData_->mColor_ = mColor;
}

material::type material::get_type() const
{
    return mType_;
}

color material::get_color() const
{
    return pColData_->mColor_;
}

void material::set_wrap(wrap mWrap)
{
    if (!pTexData_) return;

    pTexData_->mWrap_ = mWrap;

    if (pTexData_->bRenderTarget_)
        pTexData_->mRenderTexture_.setRepeated(mWrap == wrap::REPEAT);
    else
        pTexData_->mTexture_.setRepeated(mWrap == wrap::REPEAT);
}

void material::set_filter(filter mFilter)
{
    if (!pTexData_) return;

    pTexData_->mFilter_ = mFilter;

    if (pTexData_->bRenderTarget_)
        pTexData_->mRenderTexture_.setSmooth(mFilter == filter::LINEAR);
    else
        pTexData_->mTexture_.setSmooth(mFilter == filter::LINEAR);
}

void material::premultiply_alpha(sf::Image& mData)
{
    const uint uiWidth = mData.getSize().x;
    const uint uiHeight = mData.getSize().y;
    for (uint x = 0; x < uiWidth; ++x)
    for (uint y = 0; y < uiHeight; ++y)
    {
        sf::Color c = mData.getPixel(x, y);
        float a = c.a/255.0f;
        c.r *= a;
        c.g *= a;
        c.b *= a;
        mData.setPixel(x, y, c);
    }
}

float material::get_width() const
{
    switch (mType_)
    {
    case type::TEXTURE :
        return pTexData_->uiWidth_;
    case type::COLOR :
        return 1.0f;
    }

    return 1.0f;
}

float material::get_height() const
{
    switch (mType_)
    {
    case type::TEXTURE :
        return pTexData_->uiHeight_;
    case type::COLOR :
        return 1.0f;
    }

    return 1.0f;
}

float material::get_real_width() const
{
    switch (mType_)
    {
    case type::TEXTURE :
        return pTexData_->uiRealWidth_;
    case type::COLOR :
        return 1.0f;
    }

    return 1.0f;
}

float material::get_real_height() const
{
    switch (mType_)
    {
    case type::TEXTURE :
        return pTexData_->uiRealHeight_;
    case type::COLOR :
        return 1.0f;
    }

    return 1.0f;
}

bool material::set_dimensions(uint uiWidth, uint uiHeight)
{
    if (!pTexData_) return false;
    if (!pTexData_->bRenderTarget_) return false;

    if (uiWidth > sf::Texture::getMaximumSize() || uiHeight > sf::Texture::getMaximumSize())
        return false;

    if (uiWidth > pTexData_->uiRealWidth_ || uiHeight > pTexData_->uiRealHeight_)
    {
        pTexData_->uiWidth_      = uiWidth;
        pTexData_->uiHeight_     = uiHeight;
        // SFML is not efficient at resizing render texture, so use an exponential growth pattern
        // to avoid re-allocating a new render texture on every resize operation.
        if (uiWidth > pTexData_->uiRealWidth_)
            pTexData_->uiRealWidth_  = uiWidth + uiWidth/2;
        if (uiHeight > pTexData_->uiRealHeight_)
            pTexData_->uiRealHeight_ = uiHeight + uiHeight/2;

        if (!pTexData_->mRenderTexture_.create(pTexData_->uiRealWidth_, pTexData_->uiRealHeight_))
        {
            throw gui::exception("gui::sfml::material", "Could not create render target with dimensions "+
                utils::to_string(pTexData_->uiRealWidth_)+" x "+utils::to_string(pTexData_->uiRealHeight_)+".");
        }

        pTexData_->mRenderTexture_.setSmooth(pTexData_->mFilter_ == filter::LINEAR);
        pTexData_->mRenderTexture_.setRepeated(pTexData_->mWrap_ == wrap::REPEAT);

        return true;
    }
    else
    {
        pTexData_->uiWidth_  = uiWidth;
        pTexData_->uiHeight_ = uiHeight;
        return false;
    }
}

sf::RenderTexture* material::get_render_texture()
{
    if (!pTexData_) return nullptr;
    if (!pTexData_->bRenderTarget_) return nullptr;

    return &pTexData_->mRenderTexture_;
}

const sf::Texture* material::get_texture() const
{
    if (!pTexData_) return nullptr;

    if (pTexData_->bRenderTarget_)
        return &pTexData_->mRenderTexture_.getTexture();
    else
        return &pTexData_->mTexture_;
}
}
}
}
