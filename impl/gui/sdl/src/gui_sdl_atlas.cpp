#include "lxgui/impl/gui_sdl_atlas.hpp"
#include "lxgui/impl/gui_sdl_renderer.hpp"
#include <lxgui/gui_out.hpp>
#include <lxgui/gui_exception.hpp>
#include <lxgui/utils_string.hpp>

#include <SDL.h>
#include <SDL_image.h>

namespace lxgui {
namespace gui {
namespace sdl
{

atlas_page::atlas_page(const renderer& mRenderer, material::filter mFilter) :
    gui::atlas_page(mFilter), mRenderer_(mRenderer)
{
    // Set filtering
    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, mFilter == material::filter::NONE ? "0" : "1") == SDL_FALSE)
    {
        throw gui::exception("gui::sdl::atlas_page", "Could not set filtering hint");
    }

    uiSize_ = mRenderer_.get_texture_atlas_page_size();

    pTexture_ = SDL_CreateTexture(mRenderer_.get_sdl_renderer(),
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        uiSize_, uiSize_);

    if (pTexture_ == nullptr)
    {
        throw gui::exception("gui::sdl::material", "Could not create texture with dimensions "+
            utils::to_string(uiSize_)+" x "+utils::to_string(uiSize_)+".");
    }
}

atlas_page::~atlas_page()
{
    if (pTexture_) SDL_DestroyTexture(pTexture_);
}

std::shared_ptr<gui::material> atlas_page::add_material_(const gui::material& mMat,
    const bounds2f& mLocation)
{
    const sdl::material& mSDLMat = static_cast<const sdl::material&>(mMat);

    std::size_t uiTexturePitch = 0;
    const ub32color* pTexturePixels = nullptr;

    try
    {
        pTexturePixels = mSDLMat.lock_pointer(&uiTexturePitch);

        SDL_Rect mUpdateRect;
        mUpdateRect.x = mLocation.left;
        mUpdateRect.y = mLocation.top;
        mUpdateRect.w = mLocation.width();
        mUpdateRect.h = mLocation.height();

        if (SDL_UpdateTexture(pTexture_, &mUpdateRect, pTexturePixels, uiTexturePitch*4) != 0)
        {
            throw gui::exception("sdl::atlas_page", "Failed to upload texture to atlas.");
        }

        mSDLMat.unlock_pointer();
        pTexturePixels = nullptr;
    }
    catch (...)
    {
        if (pTexturePixels) mSDLMat.unlock_pointer();
        throw;
    }

    return std::make_shared<sdl::material>(mRenderer_.get_sdl_renderer(),
        pTexture_, mLocation, mFilter_);
}

float atlas_page::get_width() const
{
    return uiSize_;
}

float atlas_page::get_height() const
{
    return uiSize_;
}

atlas::atlas(const renderer& mRenderer, material::filter mFilter) :
    gui::atlas(mRenderer, mFilter), mSDLRenderer_(mRenderer) {}

std::unique_ptr<gui::atlas_page> atlas::create_page_() const
{
    return std::make_unique<sdl::atlas_page>(mSDLRenderer_, mFilter_);
}

}
}
}
