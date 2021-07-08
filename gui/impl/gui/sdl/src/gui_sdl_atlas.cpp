#include "lxgui/impl/gui_sdl_atlas.hpp"
#include "lxgui/impl/gui_sdl_renderer.hpp"
#include <lxgui/gui_out.hpp>
#include <lxgui/gui_exception.hpp>
#include <lxgui/utils_string.hpp>

#include <SDL.h>
#include <SDL_image.h>

struct SDL_SW_YUVTexture
{
    Uint32 format;
    Uint32 target_format;
    int w, h;
    Uint8 *pixels;

    /* These are just so we don't have to allocate them separately */
    Uint16 pitches[3];
    Uint8 *planes[3];

    /* This is a temporary surface in case we have to stretch copy */
    SDL_Surface *stretch;
    SDL_Surface *display;
};

typedef enum
{
    SDL_ScaleModeNearest, /**< nearest pixel sampling */
    SDL_ScaleModeLinear,  /**< linear filtering */
    SDL_ScaleModeBest     /**< anisotropic filtering */
} SDL_ScaleMode;

struct SDL_Texture
{
    const void *magic;
    Uint32 format;              /**< The pixel format of the texture */
    int access;                 /**< SDL_TextureAccess */
    int w;                      /**< The width of the texture */
    int h;                      /**< The height of the texture */
    int modMode;                /**< The texture modulation mode */
    SDL_BlendMode blendMode;    /**< The texture blend mode */
    SDL_ScaleMode scaleMode;    /**< The texture scale mode */
    Uint8 r, g, b, a;           /**< Texture modulation values */

    SDL_Renderer *renderer;

    /* Support for formats not supported directly by the renderer */
    SDL_Texture *native;
    SDL_SW_YUVTexture *yuv;
    void *pixels;
    int pitch;
    SDL_Rect locked_rect;
    SDL_Surface *locked_surface;  /**< Locked region exposed as a SDL surface */

    Uint32 last_command_generation; /* last command queue generation this texture was in. */

    void *driverdata;           /**< Driver specific texture representation */

    SDL_Texture *prev;
    SDL_Texture *next;
};

namespace lxgui {
namespace gui {
namespace sdl
{

atlas_page::atlas_page(SDL_Renderer* pRenderer, material::filter mFilter) :
    gui::atlas_page(mFilter), pRenderer_(pRenderer)
{
    SDL_RendererInfo mInfo;
    if (SDL_GetRendererInfo(pRenderer, &mInfo) != 0)
    {
        throw gui::exception("gui::sdl::atlas_page", "Could not get renderer information.");
    }

    if (mInfo.max_texture_width != 0)
    {
        uiSize_ = mInfo.max_texture_width;
    }
    else
    {
        uiSize_ = 128u;
    }

    // Set filtering
    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, mFilter == material::filter::NONE ? "0" : "1") == SDL_FALSE)
    {
        throw gui::exception("gui::sdl::atlas_page", "Could not set filtering hint");
    }

    pTexture_ = SDL_CreateTexture(pRenderer,
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
    const quad2f& mLocation)
{
    const sdl::material& mSDLMat = static_cast<const sdl::material&>(mMat);

    uint uiTexturePitch = 0;
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

    return std::make_shared<sdl::material>(pRenderer_, pTexture_, mLocation, mFilter_);
}

float atlas_page::get_width() const
{
    return uiSize_;
}

float atlas_page::get_height() const
{
    return uiSize_;
}

atlas::atlas(SDL_Renderer* pRenderer, material::filter mFilter) :
    gui::atlas(mFilter), pRenderer_(pRenderer) {}

std::unique_ptr<gui::atlas_page> atlas::create_page_() const
{
    return std::make_unique<sdl::atlas_page>(pRenderer_, mFilter_);
}

}
}
}
