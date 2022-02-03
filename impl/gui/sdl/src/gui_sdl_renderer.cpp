#include "lxgui/impl/gui_sdl_renderer.hpp"
#include "lxgui/impl/gui_sdl_atlas.hpp"
#include "lxgui/impl/gui_sdl_material.hpp"
#include "lxgui/impl/gui_sdl_rendertarget.hpp"
#include "lxgui/impl/gui_sdl_font.hpp"
#include <lxgui/gui_quad.hpp>
#include <lxgui/gui_matrix4.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/gui_exception.hpp>
#include <lxgui/utils_string.hpp>

#include <SDL.h>
#include <SDL_image.h>

namespace lxgui {
namespace gui {
namespace sdl
{

renderer::renderer(SDL_Renderer* pRenderer, bool bInitialiseSDLImage) : pRenderer_(pRenderer)
{
    int iWindowWidth, iWindowHeight;
    SDL_GetRendererOutputSize(pRenderer_, &iWindowWidth, &iWindowHeight);
    mWindowDimensions_ = vector2ui(iWindowWidth, iWindowHeight);

    render_target::check_availability(pRenderer);

    if (bInitialiseSDLImage)
    {
        int iImgFlags = IMG_INIT_PNG;
        if ((IMG_Init(iImgFlags) & iImgFlags) == 0)
        {
            throw gui::exception("gui::sdl::renderer", "Could not initialise SDL_image: "+
                std::string(IMG_GetError())+".");
        }
    }

    // Get maximum texture size
    SDL_RendererInfo mInfo;
    if (SDL_GetRendererInfo(pRenderer, &mInfo) != 0)
    {
        throw gui::exception("gui::sdl::renderer", "Could not get renderer information.");
    }

    uiTextureMaxSize_ = std::min(mInfo.max_texture_width, mInfo.max_texture_height);
    if (uiTextureMaxSize_ == 0)
        uiTextureMaxSize_ = 1024u;

    // Check if we can do pre-multiplied alpha
    SDL_Texture* pTexture = SDL_CreateTexture(pRenderer_, SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING, 4u, 4u);

    bPreMultipliedAlphaSupported_ = (SDL_SetTextureBlendMode(pTexture,
        (SDL_BlendMode)material::get_premultiplied_alpha_blend_mode()) == 0);

    SDL_DestroyTexture(pTexture);
}

std::string renderer::get_name() const
{
    SDL_RendererInfo mRendererInfo;
    SDL_GetRendererInfo(pRenderer_, &mRendererInfo);
    return std::string("SDL (") + mRendererInfo.name + ")";
}

void renderer::begin_(std::shared_ptr<gui::render_target> pTarget)
{
    if (pCurrentTarget_)
        throw gui::exception("gui::sdl::renderer", "Missing call to end()");

    if (pTarget)
    {
        pCurrentTarget_ = std::static_pointer_cast<sdl::render_target>(pTarget);
        pCurrentTarget_->begin();

        mTargetViewMatrix_ = pCurrentTarget_->get_view_matrix();
    }
    else
    {
        mTargetViewMatrix_ = matrix4f::view(vector2f(mWindowDimensions_));
    }

    mViewMatrix_ = mTargetViewMatrix_;
}

void renderer::end_()
{
    if (pCurrentTarget_)
        pCurrentTarget_->end();

    pCurrentTarget_ = nullptr;
}

void renderer::set_view_(const matrix4f& mViewMatrix)
{
    mRawViewMatrix_ = mViewMatrix;
    mViewMatrix_ = mViewMatrix*matrix4f::invert(mTargetViewMatrix_);
}

matrix4f renderer::get_view() const
{
    return mRawViewMatrix_;
}

color premultiply_alpha(const color& mColor, bool bPreMultipliedAlphaSupported)
{
    if (bPreMultipliedAlphaSupported)
        return color(mColor.r*mColor.a, mColor.g*mColor.a, mColor.b*mColor.a, mColor.a);
    else
        return mColor;
}

color interpolate_color(const color& mColor1, const color& mColor2, float fInterp)
{
    return color(
        mColor1.r*(1.0f - fInterp) + mColor2.r*fInterp,
        mColor1.g*(1.0f - fInterp) + mColor2.g*fInterp,
        mColor1.b*(1.0f - fInterp) + mColor2.b*fInterp,
        mColor1.a*(1.0f - fInterp) + mColor2.a*fInterp
    );
}

ub32color to_ub32color(const color& mColor)
{
    return {
        static_cast<unsigned char>(mColor.r*255),
        static_cast<unsigned char>(mColor.g*255),
        static_cast<unsigned char>(mColor.b*255),
        static_cast<unsigned char>(mColor.a*255)
    };
}

struct sdl_render_data
{
    SDL_Rect mDestQuad = {};
    SDL_Rect mDestDisplayQuad = {};
    SDL_Rect mSrcQuad = {};
    SDL_Point mCenter = {};
    int iAngle = 0;
    SDL_RendererFlip mFlip = SDL_FLIP_NONE;
};

sdl_render_data make_rects(const std::array<vertex,4>& lVertexList,
    float fTexWidth, float fTexHeight)
{
    sdl_render_data mData;

    // First, re-order vertices as top-left, top-right, bottom-right, bottom-left
    std::array<std::size_t,4> lIDs = {0u, 1u, 2u, 3u};
    if (lVertexList[0].pos.x < lVertexList[1].pos.x)
    {
        if (lVertexList[1].pos.y < lVertexList[2].pos.y)
        {
            // No rotation and no flip
        }
        else
        {
            // No rotation and flip Y
            lIDs = {3u, 2u, 1u, 0u};
        }
    }
    else if (lVertexList[0].pos.y < lVertexList[1].pos.y)
    {
        if (lVertexList[1].pos.x > lVertexList[2].pos.x)
        {
            // Rotated 90 degrees clockwise
            lIDs = {3u, 0u, 1u, 2u};
        }
        else
        {
            // Rotated 90 degrees clockwise and flip X
            lIDs = {0u, 3u, 2u, 1u};
        }
    }
    else if (lVertexList[0].pos.x > lVertexList[1].pos.x)
    {
        if (lVertexList[1].pos.y > lVertexList[2].pos.y)
        {
            // Rotated 180 degrees
            lIDs = {2u, 3u, 0u, 1u};
        }
        else
        {
            // No rotation and flip X
            lIDs = {1u, 0u, 3u, 2u};
        }
    }
    else if (lVertexList[0].pos.y > lVertexList[1].pos.y)
    {
        if (lVertexList[1].pos.x < lVertexList[2].pos.x)
        {
            // Rotated 90 degrees counter-clockwise
            lIDs = {1u, 2u, 3u, 0u};
        }
        else
        {
            // Rotated 90 degrees counter-clockwise and flip X
            lIDs = {2u, 1u, 0u, 3u};
        }
    }

    // Now, re-order UV coordinates as top-left, top-right, bottom-right, bottom-left
    // and figure out the required rotation and flipping to render as requested.
    int iWidth = static_cast<int>(std::round(lVertexList[lIDs[2]].pos.x - lVertexList[lIDs[0]].pos.x));
    int iHeight = static_cast<int>(std::round(lVertexList[lIDs[2]].pos.y - lVertexList[lIDs[0]].pos.y));
    int iUVIndex1 = 0;
    int iUVIndex2 = 2;

    mData.mDestDisplayQuad.x = static_cast<int>(std::round(lVertexList[lIDs[0]].pos.x));
    mData.mDestDisplayQuad.y = static_cast<int>(std::round(lVertexList[lIDs[0]].pos.y));
    mData.mDestDisplayQuad.w = iWidth;
    mData.mDestDisplayQuad.h = iHeight;

    mData.mDestQuad.x = static_cast<int>(std::round(lVertexList[lIDs[0]].pos.x));
    mData.mDestQuad.y = static_cast<int>(std::round(lVertexList[lIDs[0]].pos.y));

    bool bAxisSwapped = false;
    if (lVertexList[lIDs[0]].uvs.x < lVertexList[lIDs[1]].uvs.x)
    {
        if (lVertexList[lIDs[1]].uvs.y < lVertexList[lIDs[2]].uvs.y)
        {
            // No rotation and no flip
        }
        else
        {
            // No rotation and flip Y
            mData.mFlip = SDL_FLIP_VERTICAL;
            iUVIndex1 = 3;
            iUVIndex2 = 1;
        }
    }
    else if (lVertexList[lIDs[0]].uvs.y < lVertexList[lIDs[1]].uvs.y)
    {
        bAxisSwapped = true;

        if (lVertexList[lIDs[1]].uvs.x > lVertexList[lIDs[2]].uvs.x)
        {
            // Rotated 90 degrees clockwise
            mData.iAngle = -90;
            iUVIndex1 = 3;
            iUVIndex2 = 1;
            mData.mDestQuad.y += iHeight;
        }
        else
        {
            // Rotated 90 degrees clockwise and flip X
            mData.mFlip = SDL_FLIP_HORIZONTAL;
            mData.iAngle = -90;
            iUVIndex1 = 0;
            iUVIndex2 = 2;
            mData.mDestQuad.y += iHeight;
        }
    }
    else if (lVertexList[lIDs[0]].uvs.x > lVertexList[lIDs[1]].uvs.x)
    {
        if (lVertexList[lIDs[1]].uvs.y > lVertexList[lIDs[2]].uvs.y)
        {
            // Rotated 180 degrees
            mData.iAngle = 180;
            iUVIndex1 = 2;
            iUVIndex2 = 0;
            mData.mDestQuad.x += iWidth;
            mData.mDestQuad.y += iHeight;
        }
        else
        {
            // No rotation and flip X
            mData.mFlip = SDL_FLIP_HORIZONTAL;
            iUVIndex1 = 1;
            iUVIndex2 = 3;
        }
    }
    else if (lVertexList[lIDs[0]].uvs.y > lVertexList[lIDs[1]].uvs.y)
    {
        bAxisSwapped = true;

        if (lVertexList[lIDs[1]].uvs.x < lVertexList[lIDs[2]].uvs.x)
        {
            // Rotated 90 degrees counter-clockwise
            mData.iAngle = 90;
            iUVIndex1 = 1;
            iUVIndex2 = 3;
            mData.mDestQuad.x += iWidth;
        }
        else
        {
            // Rotated 90 degrees counter-clockwise and flip X
            mData.mFlip = SDL_FLIP_HORIZONTAL;
            mData.iAngle = 90;
            iUVIndex1 = 2;
            iUVIndex2 = 0;
            mData.mDestQuad.x += iWidth;
        }
    }

    if (bAxisSwapped)
        std::swap(iWidth, iHeight);

    mData.mSrcQuad = SDL_Rect{
        (int)std::round(lVertexList[lIDs[iUVIndex1]].uvs.x*fTexWidth),
        (int)std::round(lVertexList[lIDs[iUVIndex1]].uvs.y*fTexHeight),
        (int)std::round((lVertexList[lIDs[iUVIndex2]].uvs.x - lVertexList[lIDs[iUVIndex1]].uvs.x)*fTexWidth),
        (int)std::round((lVertexList[lIDs[iUVIndex2]].uvs.y - lVertexList[lIDs[iUVIndex1]].uvs.y)*fTexHeight)
    };

    mData.mDestQuad.w = iWidth;
    mData.mDestQuad.h = iHeight;

    mData.mCenter = {0, 0};

    return mData;
}

void renderer::render_quad_(const sdl::material* pMat, const std::array<vertex,4>& lVertexList)
{
    auto lViewList = lVertexList;
    for (auto& v : lViewList)
    {
        v.pos = v.pos*mViewMatrix_;
    }

    if (pMat)
    {
        SDL_Texture* pTexture = pMat->get_texture();
        const vector2ui mTexDims = pMat->get_canvas_dimensions();
        const int iTexWidth = static_cast<int>(mTexDims.x);
        const int iTexHeight = static_cast<int>(mTexDims.y);

        // Build the source and destination rect, figuring out rotation and flipping
        const sdl_render_data mData = make_rects(lViewList, iTexWidth, iTexHeight);

        if (mData.mDestQuad.w == 0 || mData.mDestQuad.h == 0) return;

        if (bPreMultipliedAlphaSupported_)
        {
            if (SDL_SetTextureBlendMode(pTexture,
                (SDL_BlendMode)material::get_premultiplied_alpha_blend_mode()) != 0)
            {
                throw gui::exception("gui::sdl::renderer", "Could not set texture blend mode.");
            }
        }
        else
        {
            if (SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND) != 0)
            {
                throw gui::exception("gui::sdl::renderer", "Could not set texture blend mode.");
            }
        }

        if (lViewList[0].col == lViewList[1].col && lViewList[0].col == lViewList[2].col &&
            lViewList[0].col == lViewList[3].col)
        {
            const auto mColor = lViewList[0].col;

            if (bPreMultipliedAlphaSupported_)
            {
                SDL_SetTextureColorMod(pTexture,
                    mColor.r*mColor.a*255, mColor.g*mColor.a*255, mColor.b*mColor.a*255);
            }
            else
            {
                SDL_SetTextureColorMod(pTexture, mColor.r*255, mColor.g*255, mColor.b*255);
            }

            SDL_SetTextureAlphaMod(pTexture, mColor.a*255);

            bool bCoordsAllInTexture = mData.mSrcQuad.x >= 0 && mData.mSrcQuad.y >= 0 &&
                mData.mSrcQuad.x + mData.mSrcQuad.w <= iTexWidth &&
                mData.mSrcQuad.y + mData.mSrcQuad.h <= iTexHeight;

            bool bOnePixel = (iTexHeight == 1 || mData.mDestQuad.h == 1) &&
                             (iTexWidth == 1 || mData.mDestQuad.w == 1);

            if (pMat->get_wrap() == material::wrap::CLAMP || bCoordsAllInTexture || bOnePixel)
            {
                // Single texture copy, or clamped wrap
                SDL_RenderCopyEx(pRenderer_, pTexture, &mData.mSrcQuad, &mData.mDestQuad,
                    mData.iAngle, &mData.mCenter, mData.mFlip);
            }
            else
            {
                // Repeat wrap; SDL does not support this natively, so we have to
                // do the repeating ourselves.
                const bool bAxisSwapped = std::abs(mData.iAngle) == 90;
                const float fXFactor = float(mData.mDestDisplayQuad.w)/
                    float(bAxisSwapped ? mData.mSrcQuad.h : mData.mSrcQuad.w);
                const float fYFactor = float(mData.mDestDisplayQuad.h)/
                    float(bAxisSwapped ? mData.mSrcQuad.w : mData.mSrcQuad.h);

                int iSY = mData.mSrcQuad.y;
                while (iSY < mData.mSrcQuad.y + mData.mSrcQuad.h)
                {
                    const int iSYClamped = iSY % iTexHeight + (iSY < 0 ? iTexHeight : 0);
                    int iTempSHeight = iTexHeight - iSYClamped;
                    const int iSY1 = iSY - mData.mSrcQuad.y;
                    if (iSY1 + iTempSHeight > mData.mSrcQuad.h)
                        iTempSHeight = mData.mSrcQuad.h - iSY1;
                    const int iSY2 = iSY1 + iTempSHeight;

                    int iSX = mData.mSrcQuad.x;
                    while (iSX < mData.mSrcQuad.x + mData.mSrcQuad.w)
                    {
                        const int iSXClamped = iSX % iTexWidth + (iSX < 0 ? iTexWidth : 0);
                        int iTempSWidth = iTexWidth - iSXClamped;
                        const int iSX1 = iSX - mData.mSrcQuad.x;
                        if (iSX1 + iTempSWidth > mData.mSrcQuad.w)
                            iTempSWidth = mData.mSrcQuad.w - iSX1;
                        const int iSX2 = iSX1 + iTempSWidth;

                        int iDX = mData.mDestDisplayQuad.x +
                            static_cast<int>((bAxisSwapped ? iSY1 : iSX1)*fXFactor);
                        int iDY = mData.mDestDisplayQuad.y +
                            static_cast<int>((bAxisSwapped ? iSX1 : iSY1)*fYFactor);

                        int iTempDWidth = mData.mDestDisplayQuad.x +
                            static_cast<int>((bAxisSwapped ? iSY2 : iSX2)*fXFactor) - iDX;
                        int iTempDHeight = mData.mDestDisplayQuad.y +
                            static_cast<int>((bAxisSwapped ? iSX2 : iSY2)*fYFactor) - iDY;

                        if (mData.iAngle == 90)
                            iDX += iTempDWidth;
                        else if (mData.iAngle == -90)
                            iDY += iTempDHeight;
                        else if (mData.iAngle == 180)
                        {
                            iDX += iTempDWidth;
                            iDY += iTempDHeight;
                        }

                        if (bAxisSwapped)
                            std::swap(iTempDWidth, iTempDHeight);

                        const SDL_Rect mSrcQuad{iSXClamped, iSYClamped, iTempSWidth, iTempSHeight};
                        const SDL_Rect mDestQuad{iDX, iDY, iTempDWidth, iTempDHeight};

                        SDL_RenderCopyEx(pRenderer_, pTexture, &mSrcQuad, &mDestQuad,
                            mData.iAngle, &mData.mCenter, mData.mFlip);

                        iSX += iTempSWidth;
                    }

                    iSY += iTempSHeight;
                }
            }
        }
        else
        {
            throw gui::exception("sdl::renderer", "Per-vertex color with texture is not supported.");
        }
    }
    else
    {
        // Note: SDL only supports axis-aligned rects for quad shapes
        const SDL_Rect mDestQuad = {
            static_cast<int>(lViewList[0].pos.x),
            static_cast<int>(lViewList[0].pos.y),
            static_cast<int>(lViewList[2].pos.x - lViewList[0].pos.x),
            static_cast<int>(lViewList[2].pos.y - lViewList[0].pos.y)
        };

        if (mDestQuad.w == 0 || mDestQuad.h == 0) return;

        if (lViewList[0].col == lViewList[1].col && lViewList[0].col == lViewList[2].col &&
            lViewList[0].col == lViewList[3].col)
        {
            // Same color for all vertices
            const auto& mColor = lViewList[0].col;
            if (bPreMultipliedAlphaSupported_)
            {
                SDL_SetRenderDrawBlendMode(pRenderer_,
                    (SDL_BlendMode)material::get_premultiplied_alpha_blend_mode());
                SDL_SetRenderDrawColor(pRenderer_,
                    mColor.r*mColor.a*255, mColor.g*mColor.a*255, mColor.b*mColor.a*255,
                    mColor.a*255);
            }
            else
            {
                SDL_SetRenderDrawBlendMode(pRenderer_, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(pRenderer_,
                    mColor.r*255, mColor.g*255, mColor.b*255, mColor.a*255);
            }

            SDL_RenderFillRect(pRenderer_, &mDestQuad);
        }
        else
        {
            // Different colors for each vertex; SDL does not support this natively.
            // We have to create a temporary texture, do the bilinear interpolation ourselves,
            // and draw that.
            const color lColorQuad[4] = {
                premultiply_alpha(lViewList[0].col, bPreMultipliedAlphaSupported_),
                premultiply_alpha(lViewList[1].col, bPreMultipliedAlphaSupported_),
                premultiply_alpha(lViewList[2].col, bPreMultipliedAlphaSupported_),
                premultiply_alpha(lViewList[3].col, bPreMultipliedAlphaSupported_)
            };

            sdl::material mTempMat(pRenderer_, vector2ui(mDestQuad.w, mDestQuad.h), false);
            ub32color* pPixelData = mTempMat.lock_pointer();
            for (int y = 0; y < mDestQuad.h; ++y)
            for (int x = 0; x < mDestQuad.w; ++x)
            {
                const color lColY1 = interpolate_color(
                    lColorQuad[0], lColorQuad[3], y/float(mDestQuad.h - 1));
                const color lColY2 = interpolate_color(
                    lColorQuad[1], lColorQuad[2], y/float(mDestQuad.h - 1));
                pPixelData[y * mDestQuad.w + x] = to_ub32color(
                    interpolate_color(lColY1, lColY2, x/float(mDestQuad.w - 1)));
            }
            mTempMat.unlock_pointer();

            const SDL_Rect mSrcQuad = { 0, 0, mDestQuad.w, mDestQuad.h };

            SDL_RenderCopy(pRenderer_, mTempMat.get_texture(), &mSrcQuad, &mDestQuad);
        }
    }
}

void renderer::render_quads_(const gui::material* pMaterial,
    const std::vector<std::array<vertex,4>>& lQuadList)
{
    const sdl::material* pMat = static_cast<const sdl::material*>(pMaterial);

    for (std::size_t k = 0; k < lQuadList.size(); ++k)
    {
        render_quad_(pMat, lQuadList[k]);
    }
}

void renderer::render_cache_(const gui::material*, const gui::vertex_cache&, const matrix4f&)
{
    throw gui::exception("gui::sdl::renderer", "SDL does not support vertex caches.");
}

SDL_Renderer* renderer::get_sdl_renderer() const
{
    return pRenderer_;
}

std::shared_ptr<gui::material> renderer::create_material_(const std::string& sFileName, material::filter mFilter)
{
    return std::make_shared<sdl::material>(
        pRenderer_, sFileName, bPreMultipliedAlphaSupported_, material::wrap::REPEAT, mFilter
    );
}

std::shared_ptr<gui::atlas> renderer::create_atlas_(material::filter mFilter)
{
    return std::make_shared<sdl::atlas>(*this, mFilter);
}

std::size_t renderer::get_texture_max_size() const
{
    return uiTextureMaxSize_;
}

bool renderer::is_texture_atlas_supported() const
{
    return true;
}

bool renderer::is_texture_vertex_color_supported() const
{
    return false;
}

bool renderer::is_vertex_cache_supported() const
{
    return false;
}

std::shared_ptr<gui::material> renderer::create_material(const vector2ui& mDimensions,
    const ub32color* pPixelData, material::filter mFilter)
{
    std::shared_ptr<sdl::material> pTex = std::make_shared<sdl::material>(
        pRenderer_, mDimensions, false, material::wrap::REPEAT, mFilter);

    std::size_t uiPitch = 0u;
    ub32color* pTexData = pTex->lock_pointer(&uiPitch);

    for (std::size_t uiY = 0u; uiY < mDimensions.y; ++uiY)
    {
        const ub32color* pPixelDataRow = pPixelData + uiY*mDimensions.x;
        ub32color* pTexDataRow = pTexData + uiY*uiPitch;
        std::copy(pPixelDataRow, pPixelDataRow + mDimensions.x, pTexDataRow);
    }

    pTex->unlock_pointer();

    return std::move(pTex);
}

std::shared_ptr<gui::material> renderer::create_material(
    std::shared_ptr<gui::render_target> pRenderTarget, const bounds2f& mLocation)
{
    auto pTex = std::static_pointer_cast<sdl::render_target>(pRenderTarget)->get_material().lock();
    if (mLocation == pRenderTarget->get_rect())
    {
        return std::move(pTex);
    }
    else
    {
        return std::make_shared<sdl::material>(pRenderer_,
            pTex->get_render_texture(), mLocation, pTex->get_filter());
    }
}

std::shared_ptr<gui::render_target> renderer::create_render_target(
    const vector2ui& mDimensions, material::filter mFilter)
{
    return std::make_shared<sdl::render_target>(pRenderer_, mDimensions, mFilter);
}

std::shared_ptr<gui::font> renderer::create_font_(const std::string& sFontFile, std::size_t uiSize,
    std::size_t uiOutline, const std::vector<code_point_range>& lCodePoints,
    char32_t uiDefaultCodePoint)
{
    return std::make_shared<sdl::font>(pRenderer_, sFontFile, uiSize, uiOutline,
        lCodePoints, uiDefaultCodePoint, bPreMultipliedAlphaSupported_);
}

std::shared_ptr<gui::vertex_cache> renderer::create_vertex_cache(gui::vertex_cache::type)
{
    throw gui::exception("gui::sdl::renderer", "SDL does not support vertex caches.");
}

void renderer::notify_window_resized(const vector2ui& mNewDimensions)
{
    mWindowDimensions_ = mNewDimensions;
}

}
}
}
