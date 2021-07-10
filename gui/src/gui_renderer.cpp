#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_atlas.hpp"
#include "lxgui/gui_atlas_default.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui {
namespace gui
{

void renderer::notify_window_resized(uint uiNewWidth, uint uiNewHeight)
{
}

std::shared_ptr<gui::material> renderer::create_material(const std::string& sFileName, material::filter mFilter) const
{
    std::string sBackedName = utils::to_string((int)mFilter) + '|' + sFileName;
    auto mIter = lTextureList_.find(sBackedName);
    if (mIter != lTextureList_.end())
    {
        if (std::shared_ptr<gui::material> pLock = mIter->second.lock())
            return pLock;
        else
            lTextureList_.erase(mIter);
    }

    try
    {
        std::shared_ptr<gui::material> pTex = create_material_(sFileName, mFilter);
        lTextureList_[sFileName] = pTex;
        return pTex;
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
    auto mIter = lFontList_.find(sFontName);
    if (mIter != lFontList_.end())
    {
        if (std::shared_ptr<gui::font> pLock = mIter->second.lock())
            return pLock;
        else
            lFontList_.erase(mIter);
    }

    std::shared_ptr<gui::font> pFont = create_font_(sFontFile, uiSize);
    lFontList_[sFontName] = pFont;
    return pFont;
}

bool renderer::is_texture_atlas_enabled() const
{
    return bTextureAtlasEnabled_;
}

void renderer::set_texture_atlas_enabled(bool bEnabled)
{
    bTextureAtlasEnabled_ = bEnabled;
}

uint renderer::get_texture_atlas_page_size() const
{
    if (uiTextureAtlasPageSize_ == 0u)
        return std::min(4096u, get_texture_max_size());
    else
        return uiTextureAtlasPageSize_;
}

void renderer::set_texture_atlas_page_size(uint uiPageSize)
{
    uiTextureAtlasPageSize_ = uiPageSize;
}

uint renderer::get_num_texture_atlas_pages() const
{
    uint uiCount = 0;

    for (const auto& mPage : lAtlasList_)
    {
        uiCount += mPage.second->get_num_pages();
    }

    return uiCount;
}

bool renderer::is_vertex_cache_enabled() const
{
    return bVertexCacheEnabled_ && is_vertex_cache_supported();
}

void renderer::set_vertex_cache_enabled(bool bEnabled)
{
    bVertexCacheEnabled_ = bEnabled;
}

void renderer::auto_detect_settings()
{
    bTextureAtlasEnabled_ = is_vertex_cache_supported();
}

std::shared_ptr<material> renderer::create_atlas_material(const std::string& sAtlasCategory,
    const std::string& sFileName, material::filter mFilter) const
{
    if (!is_texture_atlas_enabled())
        return create_material(sFileName, mFilter);

    std::shared_ptr<gui::atlas> pAtlas;

    std::string sBakedAtlasName = utils::to_string((int)mFilter) + '|' + sAtlasCategory;
    auto mIter = lAtlasList_.find(sBakedAtlasName);
    if (mIter != lAtlasList_.end())
    {
        pAtlas = mIter->second;
    }

    if (!pAtlas)
    {
        if (is_texture_atlas_natively_supported())
        {
            pAtlas = create_atlas_(mFilter);
        }
        else
        {
            pAtlas = std::make_shared<atlas_default>(*this, mFilter);
        }

        lAtlasList_[sBakedAtlasName] = pAtlas;
    }

    auto pTex = pAtlas->fetch_material(sFileName);
    if (pTex)
        return pTex;

    pTex = create_material(sFileName, mFilter);
    if (!pTex)
        return nullptr;

    auto pAddedTex = pAtlas->add_material(sFileName, *pTex);
    if (pAddedTex)
        return pAddedTex;
    else
        return pTex;
}

std::shared_ptr<material> renderer::create_material(
    std::shared_ptr<render_target> pRenderTarget) const
{
    return create_material(pRenderTarget, pRenderTarget->get_rect());
}

}
}
