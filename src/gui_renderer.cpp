#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_atlas.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui {
namespace gui
{

void renderer::begin(std::shared_ptr<render_target> pTarget)
{
    uiBatchCount_ = 0;

    if (is_quad_batching_enabled())
    {
        pCurrentMaterial_ = nullptr;

        if (is_vertex_cache_enabled())
        {
            try
            {
                if (lQuadCache_[0].pCache == nullptr)
                {
                    for (std::size_t uiIndex = 0u; uiIndex < BATCHING_CACHE_CYCLE_SIZE; ++uiIndex)
                    {
                        lQuadCache_[uiIndex].pCache = create_vertex_cache(vertex_cache::type::QUADS);
                    }
                }
            }
            catch (const std::exception& e)
            {
                gui::out << gui::warning << e.what() << std::endl;
                gui::out << gui::warning << "gui::renderer : Failed to create caches for quad batching. "
                    "Vertex caches will be disabled." << std::endl;

                bVertexCacheEnabled_ = false;
            }
        }
    }

    begin_(std::move(pTarget));
}

void renderer::end()
{
    if (is_quad_batching_enabled())
    {
        flush_quad_batch();
        uiLastFrameBatchCount_ = uiBatchCount_;
    }

    end_();
}

void renderer::set_view(const matrix4f& mViewMatrix)
{
    if (is_quad_batching_enabled())
    {
        flush_quad_batch();
    }

    set_view_(mViewMatrix);
}

void renderer::render_quad(const quad& mQuad)
{
    render_quads(mQuad.mat.get(), {mQuad.v});
}

bool renderer::uses_same_texture_(const material* pMat1, const material* pMat2) const
{
    if (pMat1 == pMat2)
        return true;

    if (pMat1 && pMat2 && pMat1->uses_same_texture(*pMat2))
        return true;

    if (is_texture_vertex_color_supported())
    {
        if (pMat1 && pMat1->is_in_atlas() && !pMat2)
            return true;
        if (pMat2 && pMat2->is_in_atlas() && !pMat1)
            return true;
    }

    return false;
}

void renderer::render_quads(const material* pMaterial,
    const std::vector<std::array<vertex,4>>& lQuadList)
{
    if (lQuadList.empty())
        return;

    if (!is_quad_batching_enabled())
    {
        // Render immediately
        render_quads_(pMaterial, lQuadList);
        return;
    }

    if (!uses_same_texture_(pMaterial, pCurrentMaterial_))
    {
        // Render current batch and start a new one
        flush_quad_batch();
        pCurrentMaterial_ = pMaterial;
    }

    if (lQuadCache_[uiCurrentQuadCache_].lData.empty())
    {
        // Start a new batch
        pCurrentMaterial_ = pMaterial;
    }

    if (!pCurrentMaterial_)
    {
        // Previous quads had no material, override with the new one
        pCurrentMaterial_ = pMaterial;
    }

    // Add to the cache
    auto& mCache = lQuadCache_[uiCurrentQuadCache_];

    if (!pMaterial && is_texture_atlas_enabled() && is_texture_vertex_color_supported())
    {
        // To allow quads with no texture to enter the batch
        // with atlas textures, we just change their UV coordinates
        // to map to the first top-left pixel of the atlas, which is always white.
        mCache.lData.reserve(mCache.lData.size() + lQuadList.size());
        for (const auto& mOrigQuad : lQuadList)
        {
            mCache.lData.push_back(mOrigQuad);
            auto& mQuad = mCache.lData.back();
            mQuad[0].uvs = mQuad[1].uvs = mQuad[2].uvs = mQuad[3].uvs = vector2f(0.0f, 0.0f);
        }
    }
    else
    {
        mCache.lData.insert(mCache.lData.end(), lQuadList.begin(), lQuadList.end());
    }
}

void renderer::flush_quad_batch()
{
    auto& mCache = lQuadCache_[uiCurrentQuadCache_];
    if (mCache.lData.empty())
        return;

    if (mCache.pCache)
    {
        mCache.pCache->update(mCache.lData[0].data(), mCache.lData.size()*4);
        render_cache_(pCurrentMaterial_, *mCache.pCache, matrix4f::IDENTITY);
    }
    else
    {
        render_quads_(pCurrentMaterial_, mCache.lData);
    }

    mCache.lData.clear();
    pCurrentMaterial_ = nullptr;

    ++uiCurrentQuadCache_;
    if (uiCurrentQuadCache_ == BATCHING_CACHE_CYCLE_SIZE)
        uiCurrentQuadCache_ = 0u;

    ++uiBatchCount_;
}

void renderer::render_cache(const material* pMaterial, const vertex_cache& mCache,
    const matrix4f& mModelTransform)
{
    if (is_quad_batching_enabled())
    {
        flush_quad_batch();
    }

    render_cache_(pMaterial, mCache, mModelTransform);
}

bool renderer::is_quad_batching_enabled() const
{
    return bQuadBatchingEnabled_;
}

void renderer::set_quad_batching_enabled(bool bEnabled)
{
    bQuadBatchingEnabled_ = bEnabled;
}

std::shared_ptr<gui::material> renderer::create_material(const std::string& sFileName,
    material::filter mFilter)
{
    std::string sBackedName = utils::to_string(static_cast<std::size_t>(mFilter)) + '|' + sFileName;
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

namespace
{
    std::string hash_font_parameters(const std::string& sFontFile, std::size_t uiSize,
        std::size_t uiOutline, const std::vector<code_point_range>& lCodePoints,
        char32_t uiDefaultCodePoint)
    {
        std::string sFontName = sFontFile + "|s" + utils::to_string(uiSize);
        if (uiOutline > 0u)
            sFontName += "|o" + utils::to_string(uiOutline);

        for (const code_point_range& mRange : lCodePoints)
            sFontName += "|c" + utils::to_string(mRange.uiFirst) + "-" + utils::to_string(mRange.uiLast);

        sFontName += "|d" + utils::to_string(uiDefaultCodePoint);

        return sFontName;
    }
}

std::shared_ptr<gui::font> renderer::create_font(const std::string& sFontFile, std::size_t uiSize,
    std::size_t uiOutline, const std::vector<code_point_range>& lCodePoints,
    char32_t uiDefaultCodePoint)
{
    const std::string sFontName = hash_font_parameters(
        sFontFile, uiSize, uiOutline, lCodePoints, uiDefaultCodePoint);

    auto mIter = lFontList_.find(sFontName);
    if (mIter != lFontList_.end())
    {
        if (std::shared_ptr<gui::font> pLock = mIter->second.lock())
            return pLock;
        else
            lFontList_.erase(mIter);
    }

    // TODO: forward lCodePoints to create_font_()

    std::shared_ptr<gui::font> pFont = create_font_(
        sFontFile, uiSize, uiOutline, lCodePoints, uiDefaultCodePoint);

    lFontList_[sFontName] = pFont;
    return pFont;
}

bool renderer::is_texture_atlas_enabled() const
{
    return bTextureAtlasEnabled_ && is_texture_atlas_supported();
}

void renderer::set_texture_atlas_enabled(bool bEnabled)
{
    bTextureAtlasEnabled_ = bEnabled;
}

std::size_t renderer::get_texture_atlas_page_size() const
{
    if (uiTextureAtlasPageSize_ == 0u)
        return std::min<std::size_t>(4096u, get_texture_max_size());
    else
        return uiTextureAtlasPageSize_;
}

void renderer::set_texture_atlas_page_size(std::size_t uiPageSize)
{
    uiTextureAtlasPageSize_ = uiPageSize;
}

std::size_t renderer::get_num_texture_atlas_pages() const
{
    std::size_t uiCount = 0;

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
    bVertexCacheEnabled_ = true;
    bTextureAtlasEnabled_ = true;
    bQuadBatchingEnabled_ = true;
}

atlas& renderer::get_atlas_(const std::string& sAtlasCategory, material::filter mFilter)
{
    std::shared_ptr<gui::atlas> pAtlas;

    std::string sBakedAtlasName = utils::to_string(static_cast<std::size_t>(mFilter)) + '|' + sAtlasCategory;
    auto mIter = lAtlasList_.find(sBakedAtlasName);
    if (mIter != lAtlasList_.end())
    {
        pAtlas = mIter->second;
    }

    if (!pAtlas)
    {
        pAtlas = create_atlas_(mFilter);
        lAtlasList_[sBakedAtlasName] = pAtlas;
    }

    return *pAtlas;
}

std::shared_ptr<material> renderer::create_atlas_material(const std::string& sAtlasCategory,
    const std::string& sFileName, material::filter mFilter)
{
    if (!is_texture_atlas_enabled())
        return create_material(sFileName, mFilter);

    auto& mAtlas = get_atlas_(sAtlasCategory, mFilter);

    auto pTex = mAtlas.fetch_material(sFileName);
    if (pTex)
        return pTex;

    pTex = create_material(sFileName, mFilter);
    if (!pTex)
        return nullptr;

    auto pAddedTex = mAtlas.add_material(sFileName, *pTex);
    if (pAddedTex)
        return pAddedTex;
    else
        return pTex;
}

std::shared_ptr<font> renderer::create_atlas_font(const std::string& sAtlasCategory,
    const std::string& sFontFile, std::size_t uiSize, std::size_t uiOutline,
    const std::vector<code_point_range>& lCodePoints, char32_t uiDefaultCodePoint)
{
    if (!is_texture_atlas_enabled())
        return create_font(sFontFile, uiSize, uiOutline, lCodePoints, uiDefaultCodePoint);

    auto& mAtlas = get_atlas_(sAtlasCategory, material::filter::NONE);

    const std::string sFontName = hash_font_parameters(
        sFontFile, uiSize, uiOutline, lCodePoints, uiDefaultCodePoint);

    auto pFont = mAtlas.fetch_font(sFontName);
    if (pFont)
        return pFont;

    pFont = create_font(sFontFile, uiSize, uiOutline, lCodePoints, uiDefaultCodePoint);
    if (!pFont)
        return nullptr;

    if (mAtlas.add_font(sFontName, pFont))
        return pFont;

    lFontList_[sFontName] = pFont;
    return pFont;
}

std::shared_ptr<material> renderer::create_material(
    std::shared_ptr<render_target> pRenderTarget)
{
    return create_material(pRenderTarget, pRenderTarget->get_rect());
}

void renderer::notify_window_resized(const vector2ui&)
{
}

}
}
