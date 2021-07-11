#include "lxgui/impl/gui_sdl_material.hpp"
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

int material::get_premultiplied_alpha_blend_mode()
{
    static const SDL_BlendMode mBlend = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ONE,
        SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        SDL_BLENDOPERATION_ADD,
        SDL_BLENDFACTOR_ONE,
        SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        SDL_BLENDOPERATION_ADD);

    return (int)mBlend;
}

material::material(SDL_Renderer* pRenderer, uint uiWidth, uint uiHeight,
    bool bRenderTarget, wrap mWrap, filter mFilter) : pRenderer_(pRenderer), bIsOwner_(true)
{
    SDL_RendererInfo mInfo;
    if (SDL_GetRendererInfo(pRenderer, &mInfo) != 0)
    {
        throw gui::exception("gui::sdl::material", "Could not get renderer information.");
    }

    if (mInfo.max_texture_width != 0)
    {
        if (uiWidth > (uint)mInfo.max_texture_width || uiHeight > (uint)mInfo.max_texture_height)
        {
            throw gui::exception("gui::sdl::material",
                "Texture dimensions not supported by hardware: ("+
                utils::to_string(uiWidth)+" x "+utils::to_string(uiHeight)+").");
        }
    }

    // Set filtering
    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, mFilter == filter::NONE ? "0" : "1") == SDL_FALSE)
    {
        throw gui::exception("gui::sdl::material", "Could not set filtering hint");
    }

    pTexture_ = SDL_CreateTexture(pRenderer,
        SDL_PIXELFORMAT_ABGR8888,
        bRenderTarget ? SDL_TEXTUREACCESS_TARGET : SDL_TEXTUREACCESS_STREAMING,
        uiWidth, uiHeight);

    if (pTexture_ == nullptr)
    {
        throw gui::exception("gui::sdl::material", "Could not create "+
            std::string(bRenderTarget ? "render target" : "texture")+" with dimensions "+
            utils::to_string(uiWidth)+" x "+utils::to_string(uiHeight)+".");
    }

    int iTextureRealWidth = 0, iTextureRealHeight = 0, iAccess = 0;
    Uint32 uiTextureFormat = 0;
    SDL_QueryTexture(pTexture_, &uiTextureFormat, &iAccess,
        &iTextureRealWidth, &iTextureRealHeight);

    uiWidth_ = uiWidth;
    uiHeight_ = uiHeight;
    mWrap_ = mWrap;
    mFilter_ = mFilter;
    uiRealWidth_ = iTextureRealWidth;
    uiRealHeight_ = iTextureRealHeight;
    bRenderTarget_ = bRenderTarget;

    mRect_ = quad2f(0, uiWidth_, 0, uiHeight_);
}

material::material(SDL_Renderer* pRenderer, const std::string& sFileName,
    bool bPreMultipliedAlphaSupported, wrap mWrap, filter mFilter) :
    pRenderer_(pRenderer), bIsOwner_(true)
{
    // Load file
    SDL_Surface* pSurface = IMG_Load(sFileName.c_str());
    if (pSurface == nullptr)
    {
        throw gui::exception("gui::sdl::material", "Could not load image file "+sFileName+".");
    }

    // Convert to RGBA 32bit
    SDL_Surface* pConvertedSurface = SDL_ConvertSurfaceFormat(pSurface, SDL_PIXELFORMAT_ABGR8888, 0);
    SDL_FreeSurface(pSurface);
    if (pConvertedSurface == NULL)
    {
        throw gui::exception("gui::sdl::material", "Could convert image file "+
            sFileName+" to RGBA format.");
    }

    // Pre-multiply alpha
    if (bPreMultipliedAlphaSupported)
        premultiply_alpha(pConvertedSurface);

    const uint uiWidth  = pConvertedSurface->w;
    const uint uiHeight = pConvertedSurface->h;

    // Set filtering
    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, mFilter == filter::NONE ? "0" : "1") == SDL_FALSE)
    {
        throw gui::exception("gui::sdl::material", "Could not set filtering hint");
    }

    // Create streamable texture
    pTexture_ = SDL_CreateTexture(pRenderer, SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING, uiWidth, uiHeight);
    if (pTexture_ == nullptr)
    {
        SDL_FreeSurface(pConvertedSurface);
        throw gui::exception("gui::sdl::material", "Could not create texture with dimensions "+
            utils::to_string(uiWidth)+" x "+utils::to_string(uiHeight)+".");
    }

    // Copy data into the texture
    uint uiPitch = 0;
    ub32color* pTexturePixels = lock_pointer(&uiPitch);
    const ub32color* pSurfacePixelsStart = reinterpret_cast<const ub32color*>(pConvertedSurface->pixels);
    const ub32color* pSurfacePixelsEnd = pSurfacePixelsStart + uiPitch * uiHeight;
    std::copy(pSurfacePixelsStart, pSurfacePixelsEnd, pTexturePixels);
    unlock_pointer();

    int iTextureRealWidth = 0, iTextureRealHeight = 0, iAccess = 0;
    Uint32 uiTextureFormat = 0;
    SDL_QueryTexture(pTexture_, &uiTextureFormat, &iAccess,
        &iTextureRealWidth, &iTextureRealHeight);

    uiWidth_ = uiWidth;
    uiHeight_ = uiHeight;
    mWrap_ = mWrap;
    mFilter_ = mFilter;
    uiRealWidth_ = iTextureRealWidth;
    uiRealHeight_ = iTextureRealHeight;
    bRenderTarget_ = false;

    mRect_ = quad2f(0, uiWidth_, 0, uiHeight_);
}

material::material(SDL_Renderer* pRenderer, SDL_Texture* pTexture, const quad2f& mRect,
    filter mFilter) : pRenderer_(pRenderer), mRect_(mRect),
    mFilter_(mFilter), pTexture_(pTexture), bIsOwner_(false)
{
    int iTextureRealWidth = 0, iTextureRealHeight = 0, iAccess = 0;
    Uint32 uiTextureFormat = 0;
    SDL_QueryTexture(pTexture_, &uiTextureFormat, &iAccess,
        &iTextureRealWidth, &iTextureRealHeight);

    uiWidth_ = mRect_.width();
    uiHeight_ = mRect_.height();
    uiRealWidth_ = iTextureRealWidth;
    uiRealHeight_ = iTextureRealHeight;
}

material::~material() noexcept
{
    if (pTexture_ && bIsOwner_) SDL_DestroyTexture(pTexture_);
}

void material::set_wrap(wrap mWrap)
{
    if (!bIsOwner_)
    {
        throw gui::exception("gui::sdl::material",
            "A material in an atlas cannot change its wrapping mode.");
    }

    mWrap_ = mWrap;
}

material::wrap material::get_wrap() const
{
    return mWrap_;
}

void material::set_filter(filter mFilter)
{
    if (!bIsOwner_)
    {
        throw gui::exception("gui::sdl::material",
            "A material in an atlas cannot change its filtering.");
    }

    mFilter_ = mFilter;
}

material::filter material::get_filter() const
{
    return mFilter_;
}

void material::premultiply_alpha(SDL_Surface* pSurface)
{
    ub32color* pPixelData = reinterpret_cast<ub32color*>(pSurface->pixels);

    const uint uiArea = pSurface->w * pSurface->h;
    for (uint i = 0; i < uiArea; ++i)
    {
        float a = pPixelData[i].a/255.0f;
        pPixelData[i].r *= a;
        pPixelData[i].g *= a;
        pPixelData[i].b *= a;
    }
}

quad2f material::get_rect() const
{
    return mRect_;
}

float material::get_canvas_width() const
{
    return uiRealWidth_;
}

float material::get_canvas_height() const
{
    return uiRealHeight_;
}

bool material::uses_same_texture(const gui::material& mOther) const
{
    return pTexture_ == static_cast<const sdl::material&>(mOther).pTexture_;
}

bool material::set_dimensions(uint uiWidth, uint uiHeight)
{
    if (!bIsOwner_)
    {
        throw gui::exception("gui::sdl::material", "A material in an atlas cannot be resized.");
    }

    if (!bRenderTarget_) return false;

    SDL_RendererInfo mInfo;
    if (SDL_GetRendererInfo(pRenderer_, &mInfo) != 0)
    {
        throw gui::exception("gui::sdl::material", "Could not get renderer information.");
    }

    if (mInfo.max_texture_width != 0)
    {
        if (uiWidth > (uint)mInfo.max_texture_width || uiHeight > (uint)mInfo.max_texture_height)
        {
            return false;
        }
    }

    uiWidth_  = uiWidth;
    uiHeight_ = uiHeight;
    mRect_    = quad2f(0, uiWidth_, 0, uiHeight_);

    if (uiWidth > uiRealWidth_ || uiHeight > uiRealHeight_)
    {
        // SDL is not efficient at resizing render texture, so use an exponential growth pattern
        // to avoid re-allocating a new render texture on every resize operation.
        if (uiWidth > uiRealWidth_)
            uiRealWidth_  = uiWidth + uiWidth/2;
        if (uiHeight > uiRealHeight_)
            uiRealHeight_ = uiHeight + uiHeight/2;

        SDL_DestroyTexture(pTexture_);
        pTexture_ = SDL_CreateTexture(pRenderer_, SDL_PIXELFORMAT_ABGR8888,
            SDL_TEXTUREACCESS_TARGET, uiRealWidth_, uiRealHeight_);

        if (pTexture_ == nullptr)
        {
            throw gui::exception("gui::sdl::material", "Could not create render target "
                "with dimensions "+utils::to_string(uiWidth)+" x "+utils::to_string(uiHeight)+".");
        }

        return true;
    }
    else
    {
        return false;
    }
}

const ub32color* material::lock_pointer(uint* pPitch) const
{
    void* pPixelData = nullptr;
    int iPitch = 0;
    if (SDL_LockTexture(pTexture_, nullptr, &pPixelData, &iPitch) != 0)
    {
        throw gui::exception("gui::sdl::material", "Could not lock texture for copying pixels.");
    }

    if (pPitch)
        *pPitch = static_cast<uint>(iPitch) / sizeof(ub32color);

    return reinterpret_cast<ub32color*>(pPixelData);
}

ub32color* material::lock_pointer(uint* pPitch)
{
    if (!bIsOwner_)
    {
        throw gui::exception("gui::sdl::material", "A material in an atlas cannot update its data.");
    }

    return const_cast<ub32color*>(const_cast<const material*>(this)->lock_pointer(pPitch));
}

void material::unlock_pointer() const
{
    SDL_UnlockTexture(pTexture_);
}

SDL_Texture* material::get_render_texture()
{
    if (!bRenderTarget_) return nullptr;

    return pTexture_;
}

SDL_Texture* material::get_texture() const
{
    return pTexture_;
}

SDL_Renderer* material::get_renderer()
{
    return pRenderer_;
}

}
}
}
