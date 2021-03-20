#include "lxgui/impl/gui_sdl_renderer.hpp"
#include "lxgui/impl/gui_sdl_material.hpp"
#include "lxgui/impl/gui_sdl_rendertarget.hpp"
#include "lxgui/impl/gui_sdl_font.hpp"
#include <lxgui/gui_sprite.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/gui_exception.hpp>
#include <lxgui/utils_string.hpp>

#include <SDL.h>

namespace lxgui {
namespace gui {
namespace sdl
{

renderer::renderer(SDL_Renderer* pRenderer) : pRenderer_(pRenderer)
{
    render_target::check_availability(pRenderer);
}

void renderer::begin(std::shared_ptr<gui::render_target> pTarget) const
{
    if (pCurrentTarget_)
        throw gui::exception("gui::sdl::renderer", "Missing call to end()");

    if (pTarget)
    {
        pCurrentTarget_ = std::static_pointer_cast<sdl::render_target>(pTarget);
        pCurrentTarget_->begin();
    }
}

void renderer::end() const
{
    if (pCurrentTarget_)
        pCurrentTarget_->end();

    pCurrentTarget_ = nullptr;
}

color premultiply_alpha(const color& mColor)
{
    return color(mColor.r*mColor.a, mColor.g*mColor.a, mColor.b*mColor.a, mColor.a);
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
    SDL_Rect mDestQuad;
    SDL_Rect mSrcQuad;
    double dAngle = 0.0;
    SDL_RendererFlip mFlip = SDL_FLIP_NONE;
};

sdl_render_data make_rects(const std::array<vertex,4>& lVertexList,
    float fTexWidth, float fTexHeight)
{
    sdl_render_data mData;

    // First, re-order vertices as top-left, top-right, bottom-right, bottom-left
    std::array<std::size_t,4> lIds = {0u, 1u, 2u, 3u};
    if (lVertexList[0].pos.x < lVertexList[1].pos.x)
    {
        if (lVertexList[1].pos.y < lVertexList[2].pos.y)
        {
            // No rotation and no flip
        }
        else
        {
            // No rotation and flip Y
            lIds = {3u, 2u, 1u, 0u};
        }
    }
    else if (lVertexList[0].pos.y < lVertexList[1].pos.y)
    {
        // std::swap(iWidth, iHeight);

        if (lVertexList[1].pos.x > lVertexList[2].pos.x)
        {
            // Rotated 90 degrees clockwise
            lIds = {3u, 0u, 1u, 2u};
        }
        else
        {
            // Rotated 90 degrees clockwise and flip X
            lIds = {0u, 3u, 2u, 1u};
        }
    }
    else if (lVertexList[0].pos.x > lVertexList[1].pos.x)
    {
        if (lVertexList[1].pos.y > lVertexList[2].pos.y)
        {
            // Rotated 180 degrees
            lIds = {2u, 3u, 0u, 1u};
        }
        else
        {
            // No rotation and flip X
            lIds = {1u, 0u, 3u, 2u};
        }
    }
    else if (lVertexList[0].pos.y > lVertexList[1].pos.y)
    {
        // std::swap(iWidth, iHeight);

        if (lVertexList[1].pos.x < lVertexList[2].pos.x)
        {
            // Rotated 90 degrees counter-clockwise
            lIds = {1u, 2u, 3u, 0u};
        }
        else
        {
            // Rotated 90 degrees counter-clockwise and flip X
            lIds = {2u, 1u, 0u, 3u};
        }
    }

    // Now, re-order UV coordinates as top-left, top-right, bottom-right, bottom-left
    // and figure out the required rotation and flipping to render as requested.
    int iWidth = static_cast<int>(std::abs(lVertexList[2].pos.x - lVertexList[0].pos.x));
    int iHeight = static_cast<int>(std::abs(lVertexList[2].pos.y - lVertexList[0].pos.y));
    int iUVIndex1 = 0;
    int iUVIndex2 = 2;

    if (lVertexList[lIds[0]].uvs.x < lVertexList[lIds[1]].uvs.x)
    {
        if (lVertexList[lIds[1]].uvs.y < lVertexList[lIds[2]].uvs.y)
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
    else if (lVertexList[lIds[0]].uvs.y < lVertexList[lIds[1]].uvs.y)
    {
        std::swap(iWidth, iHeight);

        if (lVertexList[lIds[1]].uvs.x > lVertexList[lIds[2]].uvs.x)
        {
            // Rotated 90 degrees clockwise
            mData.dAngle = -90.0;
            iUVIndex1 = 3;
            iUVIndex2 = 1;
        }
        else
        {
            // Rotated 90 degrees clockwise and flip X
            mData.mFlip = SDL_FLIP_HORIZONTAL;
            mData.dAngle = -90.0;
            iUVIndex1 = 0;
            iUVIndex2 = 2;
        }
    }
    else if (lVertexList[lIds[0]].uvs.x > lVertexList[lIds[1]].uvs.x)
    {
        if (lVertexList[lIds[1]].uvs.y > lVertexList[lIds[2]].uvs.y)
        {
            // Rotated 180 degrees
            mData.dAngle = 180.0;
            iUVIndex1 = 2;
            iUVIndex2 = 0;
        }
        else
        {
            // No rotation and flip X
            mData.mFlip = SDL_FLIP_HORIZONTAL;
            iUVIndex1 = 1;
            iUVIndex2 = 3;
        }
    }
    else if (lVertexList[lIds[0]].uvs.y > lVertexList[lIds[1]].uvs.y)
    {
        std::swap(iWidth, iHeight);

        if (lVertexList[lIds[1]].uvs.x < lVertexList[lIds[2]].uvs.x)
        {
            // Rotated 90 degrees counter-clockwise
            mData.dAngle = 90.0;
            iUVIndex1 = 1;
            iUVIndex2 = 3;
        }
        else
        {
            // Rotated 90 degrees counter-clockwise and flip X
            mData.mFlip = SDL_FLIP_HORIZONTAL;
            mData.dAngle = 90.0;
            iUVIndex1 = 2;
            iUVIndex2 = 0;
        }
    }

    mData.mSrcQuad = SDL_Rect{
        static_cast<int>(lVertexList[lIds[iUVIndex1]].uvs.x*fTexWidth),
        static_cast<int>(lVertexList[lIds[iUVIndex1]].uvs.y*fTexHeight),
        static_cast<int>((lVertexList[lIds[iUVIndex2]].uvs.x - lVertexList[lIds[iUVIndex1]].uvs.x)*fTexWidth),
        static_cast<int>((lVertexList[lIds[iUVIndex2]].uvs.y - lVertexList[lIds[iUVIndex1]].uvs.y)*fTexHeight)
    };

    const int iCenterX = static_cast<int>((lVertexList[0].pos.x + lVertexList[2].pos.x)/2.0f);
    const int iCenterY = static_cast<int>((lVertexList[0].pos.y + lVertexList[2].pos.y)/2.0f);

    mData.mDestQuad.x = iCenterX - iWidth/2;
    mData.mDestQuad.y = iCenterY - iHeight/2;
    mData.mDestQuad.w = iWidth;
    mData.mDestQuad.h = iHeight;

    return mData;
}

void renderer::render_quad(std::shared_ptr<sdl::material> pMat,
    const std::array<vertex,4>& lVertexList) const
{
    if (pMat->get_type() == sdl::material::type::TEXTURE)
    {
        const float fTexWidth = pMat->get_real_width();
        const float fTexHeight = pMat->get_real_height();

        const sdl_render_data mData = make_rects(lVertexList, fTexWidth, fTexHeight);

        if (lVertexList[0].col == lVertexList[1].col && lVertexList[0].col == lVertexList[2].col &&
            lVertexList[0].col == lVertexList[3].col)
        {
            const auto mColor = lVertexList[0].col;

            SDL_SetTextureColorMod(pMat->get_texture(),
                mColor.r*mColor.a*255, mColor.g*mColor.a*255, mColor.b*mColor.a*255);
            SDL_SetTextureAlphaMod(pMat->get_texture(), mColor.a*255);

            SDL_RenderCopyEx(pRenderer_, pMat->get_texture(), &mData.mSrcQuad, &mData.mDestQuad,
                mData.dAngle, nullptr, mData.mFlip);
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
            static_cast<int>(lVertexList[0].pos.x),
            static_cast<int>(lVertexList[0].pos.y),
            static_cast<int>(lVertexList[2].pos.x - lVertexList[0].pos.x),
            static_cast<int>(lVertexList[2].pos.y - lVertexList[0].pos.y)
        };

        if (lVertexList[0].col == lVertexList[1].col && lVertexList[0].col == lVertexList[2].col &&
            lVertexList[0].col == lVertexList[3].col)
        {
            // Same color for all vertices
            const auto& mColor = lVertexList[0].col * pMat->get_color();
            SDL_SetRenderDrawBlendMode(pRenderer_,
                (SDL_BlendMode)material::get_premultiplied_alpha_blend_mode());
            SDL_SetRenderDrawColor(pRenderer_,
                mColor.r*mColor.a*255, mColor.g*mColor.a*255, mColor.b*mColor.a*255, mColor.a*255);

            SDL_RenderFillRect(pRenderer_, &mDestQuad);
        }
        else
        {
            // Different colors for each vertex; SDL does not support this natively.
            // We have to create a temporary texture, do the bilinear interpolation ourselves,
            // and draw that.
            const color lColorQuad[4] = {
                premultiply_alpha(lVertexList[0].col * pMat->get_color()),
                premultiply_alpha(lVertexList[1].col * pMat->get_color()),
                premultiply_alpha(lVertexList[2].col * pMat->get_color()),
                premultiply_alpha(lVertexList[3].col * pMat->get_color())
            };

            sdl::material pTempMat(pRenderer_, mDestQuad.w, mDestQuad.h, false);
            ub32color* pPixelData = pTempMat.lock_pointer();
            for (int y = 0; y < mDestQuad.h; ++y)
            for (int x = 0; x < mDestQuad.w; ++x)
            {
                const color lColY1 = interpolate_color(lColorQuad[0], lColorQuad[3], y/float(mDestQuad.h - 1));
                const color lColY2 = interpolate_color(lColorQuad[1], lColorQuad[2], y/float(mDestQuad.h - 1));
                pPixelData[y * mDestQuad.w + x] = to_ub32color(
                    interpolate_color(lColY1, lColY2, x/float(mDestQuad.w - 1)));
            }
            pTempMat.unlock_pointer();

            const SDL_Rect mSrcQuad = { 0, 0, mDestQuad.w, mDestQuad.h };

            SDL_RenderCopy(pRenderer_, pTempMat.get_texture(), &mSrcQuad, &mDestQuad);
        }
    }
}

void renderer::render_quad(const quad& mQuad) const
{
    std::shared_ptr<sdl::material> pMat = std::static_pointer_cast<sdl::material>(mQuad.mat);

    render_quad(pMat, mQuad.v);
}

void renderer::render_quads(const quad& mQuad, const std::vector<std::array<vertex,4>>& lQuadList) const
{
    std::shared_ptr<sdl::material> pMat = std::static_pointer_cast<sdl::material>(mQuad.mat);

    for (uint k = 0; k < lQuadList.size(); ++k)
    {
        render_quad(pMat, lQuadList[k]);
    }
}

std::shared_ptr<gui::material> renderer::create_material(const std::string& sFileName, material::filter mFilter) const
{
    std::string sBackedName = utils::to_string((int)mFilter) + '|' + sFileName;
    std::map<std::string, std::weak_ptr<gui::material>>::iterator iter = lTextureList_.find(sBackedName);
    if (iter != lTextureList_.end())
    {
        if (std::shared_ptr<gui::material> pLock = iter->second.lock())
            return pLock;
        else
            lTextureList_.erase(iter);
    }

    try
    {
        std::shared_ptr<gui::material> pTex = std::make_shared<sdl::material>(
            pRenderer_, sFileName, material::wrap::REPEAT, mFilter
        );

        lTextureList_[sFileName] = pTex;
        return pTex;
    }
    catch (const std::exception& e)
    {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<gui::material> renderer::create_material(const color& mColor) const
{
    return std::make_shared<sdl::material>(pRenderer_, mColor);
}

std::shared_ptr<gui::material> renderer::create_material(std::shared_ptr<gui::render_target> pRenderTarget) const
{
    try
    {
        return std::static_pointer_cast<sdl::render_target>(pRenderTarget)->get_material().lock();
    }
    catch (const std::exception& e)
    {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<gui::render_target> renderer::create_render_target(uint uiWidth, uint uiHeight) const
{
    try
    {
        return std::make_shared<sdl::render_target>(pRenderer_, uiWidth, uiHeight);
    }
    catch (const std::exception& e)
    {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<gui::font> renderer::create_font(const std::string& sFontFile, uint uiSize) const
{
    std::string sFontName = sFontFile + "|" + utils::to_string(uiSize);
    std::map<std::string, std::weak_ptr<gui::font>>::iterator iter = lFontList_.find(sFontName);
    if (iter != lFontList_.end())
    {
        if (std::shared_ptr<gui::font> pLock = iter->second.lock())
            return pLock;
        else
            lFontList_.erase(iter);
    }

    std::shared_ptr<gui::font> pFont = std::make_shared<sdl::font>(pRenderer_, sFontFile, uiSize);
    lFontList_[sFontName] = pFont;
    return pFont;
}

}
}
}
