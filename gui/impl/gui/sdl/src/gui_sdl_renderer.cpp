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

void renderer::render_quad(const quad& mQuad) const
{
    std::shared_ptr<sdl::material> pMat = std::static_pointer_cast<sdl::material>(mQuad.mat);

    // Note: SDL only supports axis-aligned rects for quad shapes
    const SDL_Rect mDestQuad = {
        static_cast<int>(mQuad.v[0].pos.x),
        static_cast<int>(mQuad.v[0].pos.y),
        static_cast<int>(mQuad.v[2].pos.x - mQuad.v[0].pos.x),
        static_cast<int>(mQuad.v[2].pos.y - mQuad.v[0].pos.y)
    };

    if (pMat->get_type() == sdl::material::type::TEXTURE)
    {
        const float fTexWidth = pMat->get_real_width();
        const float fTexHeight = pMat->get_real_height();

        // Note: SDL only supports axis-aligned rects for UV coordinates
        const SDL_Rect mSrcQuad = {
            static_cast<int>(mQuad.v[0].uvs.x*fTexWidth),
            static_cast<int>(mQuad.v[0].uvs.y*fTexHeight),
            static_cast<int>((mQuad.v[2].uvs.x - mQuad.v[0].uvs.x)*fTexWidth),
            static_cast<int>((mQuad.v[2].uvs.y - mQuad.v[0].uvs.y)*fTexHeight)
        };

        if (mQuad.v[0].col == mQuad.v[1].col && mQuad.v[0].col == mQuad.v[2].col &&
            mQuad.v[0].col == mQuad.v[3].col)
        {
            const auto mColor = mQuad.v[0].col;

            SDL_SetTextureColorMod(pMat->get_texture(),
                mColor.r*mColor.a*255, mColor.g*mColor.a*255, mColor.b*mColor.a*255);
            SDL_SetTextureAlphaMod(pMat->get_texture(), mColor.a*255);

            SDL_RenderCopy(pRenderer_, pMat->get_texture(), &mSrcQuad, &mDestQuad);
        }
        else
        {
            throw gui::exception("sdl::renderer", "Per-vertex color with texture is not supported.");
        }
    }
    else
    {
        if (mQuad.v[0].col == mQuad.v[1].col && mQuad.v[0].col == mQuad.v[2].col &&
            mQuad.v[0].col == mQuad.v[3].col)
        {
            // Same color for all vertices
            const auto& mColor = mQuad.v[0].col * pMat->get_color();
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
                premultiply_alpha(mQuad.v[0].col * pMat->get_color()),
                premultiply_alpha(mQuad.v[1].col * pMat->get_color()),
                premultiply_alpha(mQuad.v[2].col * pMat->get_color()),
                premultiply_alpha(mQuad.v[3].col * pMat->get_color())
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

void renderer::render_quads(const quad& mQuad, const std::vector<std::array<vertex,4>>& lQuadList) const
{
    std::shared_ptr<sdl::material> pMat = std::static_pointer_cast<sdl::material>(mQuad.mat);

    for (uint k = 0; k < lQuadList.size(); ++k)
    {
        const auto& lVertexList = lQuadList[k];

        // Note: SDL only supports axis-aligned rects for quad shapes
        const SDL_Rect mDestQuad = {
            static_cast<int>(lVertexList[0].pos.x),
            static_cast<int>(lVertexList[0].pos.y),
            static_cast<int>(lVertexList[2].pos.x - lVertexList[0].pos.x),
            static_cast<int>(lVertexList[2].pos.y - lVertexList[0].pos.y)
        };

        if (pMat->get_type() == sdl::material::type::TEXTURE)
        {
            const float fTexWidth = pMat->get_real_width();
            const float fTexHeight = pMat->get_real_height();

            // Note: SDL only supports axis-aligned rects for UV coordinates
            const SDL_Rect mSrcQuad = {
                static_cast<int>(lVertexList[0].uvs.x*fTexWidth),
                static_cast<int>(lVertexList[0].uvs.y*fTexHeight),
                static_cast<int>((lVertexList[2].uvs.x - lVertexList[0].uvs.x)*fTexWidth),
                static_cast<int>((lVertexList[2].uvs.y - lVertexList[0].uvs.y)*fTexHeight)
            };

            // Note: SDL does not support per-vertex color; just use the first vertex.
            const auto& mColor = lVertexList[0].col;

            SDL_SetTextureColorMod(pMat->get_texture(),
                mColor.r*mColor.a*255, mColor.g*mColor.a*255, mColor.b*mColor.a*255);
            SDL_SetTextureAlphaMod(pMat->get_texture(), mColor.a*255);

            SDL_RenderCopy(pRenderer_, pMat->get_texture(), &mSrcQuad, &mDestQuad);
        }
        else
        {
            // Note: SDL does not support per-vertex color; just use the first vertex.
            const auto mColor = lVertexList[0].col * pMat->get_color();

            SDL_SetRenderDrawColor(pRenderer_,
                mColor.r*mColor.a*255, mColor.g*mColor.a*255, mColor.b*mColor.a*255, mColor.a*255);

            SDL_RenderFillRect(pRenderer_, &mDestQuad);
        }
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
