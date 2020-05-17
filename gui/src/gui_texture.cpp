#include "lxgui/gui_texture.hpp"

#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_filesystem.hpp>
#include <sstream>

namespace lxgui {
namespace gui
{
texture::texture(manager* pManager) : layered_region(pManager)
{
    lTexCoord_[0] = lTexCoord_[1] = lTexCoord_[3] = lTexCoord_[6] = 0.0f;
    lTexCoord_[2] = lTexCoord_[4] = lTexCoord_[5] = lTexCoord_[7] = 1.0f;
    lType_.push_back(CLASS_NAME);
}

std::string texture::serialize(const std::string& sTab) const
{
    std::ostringstream sStr;
    sStr << layered_region::serialize(sTab);

    if (!sTextureFile_.empty())
    {
        sStr << sTab << "  # File        : " << sTextureFile_ << "\n";
    }
    else if (!mGradient_.is_empty())
    {
        sStr << sTab << "  # Gradient    :\n";
        sStr << sTab << "  #-###\n";
        sStr << sTab << "  |   # min color   : " << mGradient_.get_min_color() << "\n";
        sStr << sTab << "  |   # max color   : " << mGradient_.get_max_color() << "\n";
        sStr << sTab << "  |   # orientation : ";
        switch (mGradient_.get_orientation())
        {
            case gradient::orientation::HORIZONTAL : sStr << "HORIZONTAL\n"; break;
            case gradient::orientation::VERTICAL :   sStr << "VERTICAL\n"; break;
            default : sStr << "<error>\n"; break;
        }
        sStr << sTab << "  #-###\n";
    }
    else
    {
        sStr << sTab << "  # Color       : " << mColor_ << "\n";
    }

    sStr << sTab << "  # Tex. coord. :\n";
    sStr << sTab << "  #-###\n";
    sStr << sTab << "  |   # top-left     : (" << lTexCoord_[0] << ", " << lTexCoord_[1] << ")\n";
    sStr << sTab << "  |   # top-right    : (" << lTexCoord_[2] << ", " << lTexCoord_[3] << ")\n";
    sStr << sTab << "  |   # bottom-right : (" << lTexCoord_[4] << ", " << lTexCoord_[5] << ")\n";
    sStr << sTab << "  |   # bottom-left  : (" << lTexCoord_[6] << ", " << lTexCoord_[7] << ")\n";
    sStr << sTab << "  #-###\n";
    sStr << sTab << "  # TexCModRect : " << bTexCoordModifiesRect_ << "\n";

    sStr << sTab << "  # Blend mode  : ";
    switch (mBlendMode_)
    {
        case blend_mode::NONE  : sStr << "NONE\n";  break;
        case blend_mode::BLEND : sStr << "BLEND\n"; break;
        case blend_mode::KEY   : sStr << "KEY\n";   break;
        case blend_mode::ADD   : sStr << "ADD\n";   break;
        case blend_mode::MOD   : sStr << "MOD\n";   break;
        default          : sStr << "<error>\n"; break;
    }

    sStr << sTab << "  # Filter      : ";
    switch (mFilter_)
    {
        case material::filter::NONE   : sStr << "NONE\n";   break;
        case material::filter::LINEAR : sStr << "LINEAR\n"; break;
        default                       : sStr << "<error>\n"; break;
    }

    sStr << sTab << "  # Desaturated : " << bIsDesaturated_ << "\n";

    return sStr.str();
}

void texture::render()
{
    if (bHasSprite_ && is_visible())
    {
        mSprite_.render_2v(
            lBorderList_.left,  lBorderList_.top,
            lBorderList_.right, lBorderList_.bottom
        );
    }
}

void texture::create_glue()
{
    create_glue_<lua_texture>();
}

void texture::copy_from(uiobject* pObj)
{
    uiobject::copy_from(pObj);

    texture* pTexture = dynamic_cast<texture*>(pObj);

    if (pTexture)
    {
        std::string sTexture = pTexture->get_texture();
        if (sTexture.empty())
        {
            const gradient& mGradient = pTexture->get_gradient();
            if (!mGradient.is_empty())
                this->set_gradient(mGradient);
            else
                this->set_color(pTexture->get_color());
        }
        else
            this->set_texture(sTexture);

        this->set_blend_mode(pTexture->get_blend_mode());
        this->set_tex_coord(pTexture->get_tex_coord());
        this->set_tex_coord_modifies_rect(pTexture->get_tex_coord_modifies_rect());
        this->set_desaturated(pTexture->is_desaturated());
    }
}

texture::blend_mode texture::get_blend_mode() const
{
    return mBlendMode_;
}

material::filter texture::get_filter_mode() const
{
    return mFilter_;
}

const color& texture::get_color() const
{
    return mColor_;
}

const gradient& texture::get_gradient() const
{
    return mGradient_;
}

const std::array<float,8>& texture::get_tex_coord() const
{
    return lTexCoord_;
}

bool texture::get_tex_coord_modifies_rect() const
{
    return bTexCoordModifiesRect_;
}

const std::string& texture::get_texture() const
{
    return sTextureFile_;
}

color texture::get_vertex_color() const
{
    if (bHasSprite_)
        return mSprite_.get_color();
    else
    {
        gui::out << gui::error << "gui::" << lType_.back() << " : "
            << "Trying to call get_vertex_color on an uninitialized texture : "+sName_+"." << std::endl;

        return color::EMPTY;
    }
}

bool texture::is_desaturated() const
{
    return bIsDesaturated_;
}

void texture::set_blend_mode(blend_mode mBlendMode)
{
    if (mBlendMode_ != mBlendMode)
    {
        mBlendMode_ = mBlendMode;
        notify_renderer_need_redraw();
    }
}

void texture::set_blend_mode(const std::string& sBlendMode)
{
    blend_mode mOldBlendMode = mBlendMode_;

    gui::out << gui::warning << "gui::" << lType_.back() << " : "
        << "texture::set_blend_mode is not yet implemented." << std::endl;

    if (sBlendMode == "BLEND")
        mBlendMode_ = blend_mode::BLEND;
    else if (sBlendMode == "ADD")
        mBlendMode_ = blend_mode::ADD;
    else if (sBlendMode == "MOD")
        mBlendMode_ = blend_mode::MOD;
    else if (sBlendMode == "KEY")
        mBlendMode_ = blend_mode::KEY;
    else if (sBlendMode == "NONE")
        mBlendMode_ = blend_mode::NONE;
    else
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Unknown blending : \"" << sBlendMode << "\". Using \"BLEND\"." << std::endl;

        mBlendMode_ = blend_mode::BLEND;
    }

    if (mOldBlendMode != mBlendMode_)
        notify_renderer_need_redraw();
}

void texture::set_filter_mode(material::filter mFilter)
{
    if (mFilter_ != mFilter)
    {
        mFilter_ = mFilter;

        if (!sTextureFile_.empty() && bHasSprite_)
        {
            mSprite_ = pManager_->create_sprite(pManager_->create_material(sTextureFile_, mFilter_));
            mSprite_.set_texture_coords(lTexCoord_, true);
        }

        notify_renderer_need_redraw();
    }
}

void texture::set_filter_mode(const std::string& sFilter)
{
    material::filter mOldFilter = mFilter_;

    if (sFilter == "NONE")
        mFilter_ = material::filter::NONE;
    else if (sFilter == "LINEAR")
        mFilter_ = material::filter::LINEAR;
    else
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Unknown filtering : \"" << sFilter << "\". Using \"NONE\"." << std::endl;

        mFilter_ = material::filter::NONE;
    }

    if (mOldFilter != mFilter_)
        notify_renderer_need_redraw();
}

void texture::set_desaturated(bool bIsDesaturated)
{
    if (!bHasSprite_)
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            << "Trying to desaturate an uninitialized texture : " << sName_ << "." << std::endl;

        return;
    }

    if (bIsDesaturated_ != bIsDesaturated)
    {
        bIsDesaturated_ = bIsDesaturated;
        mSprite_.set_desaturated(bIsDesaturated_);

        notify_renderer_need_redraw();
    }
}

void texture::set_gradient(const gradient& mGradient)
{
    mColor_ = color::EMPTY;
    sTextureFile_ = "";
    mGradient_ = mGradient;
    mSprite_ = pManager_->create_sprite(
        pManager_->create_material(color::WHITE), 256, 256
    );

    if (mGradient_.get_orientation() == gradient::orientation::HORIZONTAL)
    {
        mSprite_.set_color(mGradient_.get_min_color(), 0);
        mSprite_.set_color(mGradient_.get_min_color(), 3);
        mSprite_.set_color(mGradient_.get_max_color(), 1);
        mSprite_.set_color(mGradient_.get_max_color(), 2);
    }
    else
    {
        mSprite_.set_color(mGradient_.get_min_color(), 0);
        mSprite_.set_color(mGradient_.get_min_color(), 1);
        mSprite_.set_color(mGradient_.get_max_color(), 2);
        mSprite_.set_color(mGradient_.get_max_color(), 3);
    }

    notify_renderer_need_redraw();
}

void texture::set_tex_coord(const std::array<float,4>& lCoordinates)
{
    if (bHasSprite_)
    {
        mSprite_.set_texture_rect(lCoordinates, true);
        lTexCoord_ = mSprite_.get_texture_coords(true);
        notify_renderer_need_redraw();
    }
    else
    {
        gui::out << gui::error << "gui::" << lType_.back() << " : "
            << "Trying to call set_tex_coord on an uninitialized texture : " << sName_ << "." << std::endl;
    }
}

void texture::set_tex_coord(const std::array<float,8>& lCoordinates)
{
    if (bHasSprite_)
    {
        mSprite_.set_texture_coords(lCoordinates, true);
        lTexCoord_ = lCoordinates;
        notify_renderer_need_redraw();
    }
    else
    {
        gui::out << gui::error << "gui::" << lType_.back() << " : "
            << "Trying to call set_tex_coord on an uninitialized texture : " << sName_ << "." << std::endl;
    }
}

void texture::set_tex_coord_modifies_rect(bool bTexCoordModifiesRect)
{
    if (bTexCoordModifiesRect_ != bTexCoordModifiesRect)
    {
        bTexCoordModifiesRect_ = bTexCoordModifiesRect;
        notify_renderer_need_redraw();
    }
}

void texture::set_texture(const std::string& sFile)
{
    mGradient_ = gradient();
    mColor_ = color::EMPTY;
    sTextureFile_ = sFile;

    bHasSprite_ = false;

    if (sTextureFile_.empty())
        return;

    if (utils::file_exists(sTextureFile_))
    {
        mSprite_ = pManager_->create_sprite(pManager_->create_material(sTextureFile_, mFilter_));
        mSprite_.set_texture_coords(lTexCoord_, true);
    }
    else
    {
        gui::out << gui::error << "gui::" << lType_.back() << " : "
            << "Cannot find file \"" << sFile << "\" for \"" << sName_
            << "\".\nUsing white texture instead." << std::endl;

        mSprite_ = pManager_->create_sprite(pManager_->create_material(color::WHITE), 256, 256);
    }

    bHasSprite_ = true;
    notify_renderer_need_redraw();
}

void texture::set_texture(utils::refptr<render_target> pRenderTarget)
{
    mGradient_ = gradient();
    mColor_ = color::EMPTY;
    sTextureFile_ = "";

    if (pRenderTarget)
        mSprite_ = pManager_->create_sprite(pManager_->create_material(pRenderTarget));
    else
    {
        gui::out << gui::error << "gui::" << lType_.back() << " : "
            << "Cannot create a texture with a null RenterTarget.\n"
            "Using white texture instead." << std::endl;

        mSprite_ = pManager_->create_sprite(pManager_->create_material(color::WHITE), 256, 256);
    }

    bHasSprite_ = true;
    notify_renderer_need_redraw();
}

void texture::set_color(const color& mColor)
{
    mGradient_ = gradient();
    sTextureFile_ = "";

    mColor_ = mColor;
    mSprite_ = pManager_->create_sprite(pManager_->create_material(mColor), 256, 256);
    bHasSprite_ = true;

    notify_renderer_need_redraw();
}

void texture::set_sprite(sprite mSprite)
{
    mGradient_ = gradient();
    sTextureFile_ = "";

    mSprite_ = std::move(mSprite);

    set_abs_dimensions(mSprite_.get_width(), mSprite_.get_height());

    lTexCoord_ = mSprite_.get_texture_coords(true);

    bHasSprite_ = true;
    notify_renderer_need_redraw();
}

void texture::set_vertex_color(const color& mColor)
{
    if (bHasSprite_)
    {
        mSprite_.set_color(mColor);
        notify_renderer_need_redraw();
    }
    else
    {
        gui::out << gui::error << "gui::" << lType_.back() << " : "
            << "Trying to set vertex color of an uninitialized texture : " << sName_ << "." << std::endl;
    }
}
}
}
