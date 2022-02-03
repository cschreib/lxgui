#include "lxgui/impl/gui_sfml_material.hpp"
#include "lxgui/impl/gui_sfml_renderer.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_exception.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui {
namespace gui {
namespace sfml
{
material::material(const vector2ui& mDimensions, bool bRenderTarget, wrap mWrap, filter mFilter) :
    gui::material(false), mDimensions_(mDimensions), mCanvasDimensions_(mDimensions),
    mWrap_(mWrap), mFilter_(mFilter)
{
    if (mDimensions_.x > sf::Texture::getMaximumSize() ||
        mDimensions_.y > sf::Texture::getMaximumSize())
    {
        throw gui::exception("gui::sfml::material", "Texture dimensions not supported by graphics card : ("+
            utils::to_string(mDimensions_.x)+" x "+
            utils::to_string(mDimensions_.y)+")."
        );
    }

    bRenderTarget_ = bRenderTarget;

    if (bRenderTarget_)
    {
        if (!mRenderTexture_.create(mDimensions_.x, mDimensions_.y))
        {
            throw gui::exception("gui::sfml::material", "Could not create render target with dimensions "+
                utils::to_string(mDimensions_.x)+" x "+utils::to_string(mDimensions_.y)+".");
        }
        mRenderTexture_.setSmooth(mFilter == filter::LINEAR);
        mRenderTexture_.setRepeated(mWrap == wrap::REPEAT);
    }
    else
    {
        if (!mTexture_.create(mDimensions_.x, mDimensions_.y))
        {
            throw gui::exception("gui::sfml::material", "Could not create texture with dimensions "+
                utils::to_string(mDimensions_.x)+" x "+utils::to_string(mDimensions_.y)+".");
        }
        mTexture_.setSmooth(mFilter == filter::LINEAR);
        mTexture_.setRepeated(mWrap == wrap::REPEAT);
    }

    mRect_ = bounds2f(0, mDimensions_.x, 0, mDimensions_.y);
}

material::material(const sf::Image& mData, wrap mWrap, filter mFilter) : gui::material(false)
{
    bRenderTarget_ = false;
    mTexture_.loadFromImage(mData);
    mTexture_.setSmooth(mFilter == filter::LINEAR);
    mTexture_.setRepeated(mWrap == wrap::REPEAT);

    mDimensions_ = vector2ui(mTexture_.getSize().x, mTexture_.getSize().y);
    mCanvasDimensions_ = mDimensions_;
    mWrap_ = mWrap;
    mFilter_ = mFilter;

    mRect_ = bounds2f(0, mDimensions_.x, 0, mDimensions_.y);
}

material::material(const std::string& sFileName, wrap mWrap, filter mFilter) : gui::material(false)
{
    bRenderTarget_ = false;
    sf::Image mData;
    if (!mData.loadFromFile(sFileName))
        throw utils::exception("gui::sfml::material", "loading failed: '"+sFileName+"'.");

    premultiply_alpha(mData);
    mTexture_.loadFromImage(mData);
    mTexture_.setSmooth(mFilter == filter::LINEAR);
    mTexture_.setRepeated(mWrap == wrap::REPEAT);

    mDimensions_ = vector2ui(mTexture_.getSize().x, mTexture_.getSize().y);
    mCanvasDimensions_ = mDimensions_;
    mWrap_ = mWrap;
    mFilter_ = mFilter;

    mRect_ = bounds2f(0, mDimensions_.x, 0, mDimensions_.y);
}

material::material(const sf::Texture& mTexture, const bounds2f& mLocation, filter mFilter) :
    gui::material(true)
{
    mRect_ = mLocation;
    mFilter_ = mFilter;
    pAtlasTexture_ = &mTexture;
}

void material::set_wrap(wrap mWrap)
{
    if (pAtlasTexture_)
    {
        throw gui::exception("gui::sfml::material",
            "A material in an atlas cannot change its wrapping mode.");
    }

    mWrap_ = mWrap;

    if (bRenderTarget_)
        mRenderTexture_.setRepeated(mWrap == wrap::REPEAT);
    else
        mTexture_.setRepeated(mWrap == wrap::REPEAT);
}

void material::set_filter(filter mFilter)
{
    if (pAtlasTexture_)
    {
        throw gui::exception("gui::sfml::material",
            "A material in an atlas cannot change its filtering.");
    }

    mFilter_ = mFilter;

    if (bRenderTarget_)
        mRenderTexture_.setSmooth(mFilter == filter::LINEAR);
    else
        mTexture_.setSmooth(mFilter == filter::LINEAR);
}

material::filter material::get_filter() const
{
    return mFilter_;
}

void material::update_texture(const ub32color* pData)
{
    if (bRenderTarget_)
        throw gui::exception("gui::sfml::material", "A render texture cannot be updated.");

    if (pAtlasTexture_)
        throw gui::exception("gui::sfml::material", "A material in an atlas cannot be updated.");

    mTexture_.update(reinterpret_cast<const sf::Uint8*>(pData),
        mRect_.width(), mRect_.height(), mRect_.left, mRect_.top);
}

void material::premultiply_alpha(sf::Image& mData)
{
    const std::size_t uiWidth = mData.getSize().x;
    const std::size_t uiHeight = mData.getSize().y;
    for (std::size_t x = 0; x < uiWidth; ++x)
    for (std::size_t y = 0; y < uiHeight; ++y)
    {
        sf::Color c = mData.getPixel(x, y);
        float a = c.a/255.0f;
        c.r *= a;
        c.g *= a;
        c.b *= a;
        mData.setPixel(x, y, c);
    }
}

bounds2f material::get_rect() const
{
    return mRect_;
}

vector2ui material::get_canvas_dimensions() const
{
    if (pAtlasTexture_)
        return vector2ui(pAtlasTexture_->getSize().x, pAtlasTexture_->getSize().y);
    else
        return mCanvasDimensions_;
}

bool material::uses_same_texture(const gui::material& mOther) const
{
    return pAtlasTexture_ &&
        pAtlasTexture_ == static_cast<const sfml::material&>(mOther).pAtlasTexture_;
}

bool material::set_dimensions(const vector2ui& mDimensions)
{
    if (pAtlasTexture_)
    {
        throw gui::exception("gui::sfml::material", "A material in an atlas cannot be resized.");
    }

    if (!bRenderTarget_)
        return false;

    if (mDimensions.x > sf::Texture::getMaximumSize() || mDimensions.y > sf::Texture::getMaximumSize())
        return false;

    mDimensions_ = mDimensions;
    mRect_       = bounds2f(0, mDimensions_.x, 0, mDimensions_.y);

    if (mDimensions_.x > mCanvasDimensions_.x || mDimensions_.y > mCanvasDimensions_.y)
    {
        // SFML is not efficient at resizing render texture, so use an exponential growth pattern
        // to avoid re-allocating a new render texture on every resize operation.
        if (mDimensions_.x > mCanvasDimensions_.x)
            mCanvasDimensions_.x  = mDimensions_.x + mDimensions_.x/2;
        if (mDimensions_.y > mCanvasDimensions_.y)
            mCanvasDimensions_.y = mDimensions_.y + mDimensions_.y/2;

        if (!mRenderTexture_.create(mCanvasDimensions_.x, mCanvasDimensions_.y))
        {
            throw gui::exception("gui::sfml::material", "Could not create render target with dimensions "+
                utils::to_string(mCanvasDimensions_.x)+" x "+utils::to_string(mCanvasDimensions_.y)+".");
        }

        mRenderTexture_.setSmooth(mFilter_ == filter::LINEAR);
        mRenderTexture_.setRepeated(mWrap_ == wrap::REPEAT);

        return true;
    }
    else
    {
        return false;
    }
}

sf::RenderTexture* material::get_render_texture()
{
    if (!bRenderTarget_) return nullptr;

    return &mRenderTexture_;
}

const sf::Texture* material::get_texture() const
{
    if (bRenderTarget_)
        return &mRenderTexture_.getTexture();
    else if (pAtlasTexture_)
        return pAtlasTexture_;
    else
        return &mTexture_;
}
}
}
}
