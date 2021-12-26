#include "lxgui/gui_atlas.hpp"
#include "lxgui/gui_vertex.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_exception.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui {
namespace gui
{

atlas_page::atlas_page(material::filter mFilter) : mFilter_(mFilter) {}

std::shared_ptr<material> atlas_page::fetch_material(const std::string& sFileName) const
{
    auto mIter = lTextureList_.find(sFileName);
    if (mIter != lTextureList_.end())
    {
        if (std::shared_ptr<gui::material> pLock = mIter->second.lock())
            return pLock;
        else
            lTextureList_.erase(mIter);
    }

    return nullptr;
}

std::shared_ptr<gui::material> atlas_page::add_material(const std::string& sFileName,
    const material& mMat)
{
    try
    {
        const auto mRect = mMat.get_rect();
        const auto mLocation = find_location_(mRect.width(), mRect.height());
        if (!mLocation.has_value())
            return nullptr;

        std::shared_ptr<gui::material> pTex = add_material_(mMat, mLocation.value());
        lTextureList_[sFileName] = pTex;
        return pTex;
    }
    catch (const std::exception& e)
    {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<font> atlas_page::fetch_font(const std::string& sFontName) const
{
    auto mIter = lFontList_.find(sFontName);
    if (mIter != lFontList_.end())
    {
        if (std::shared_ptr<gui::font> pLock = mIter->second.lock())
            return pLock;
        else
            lFontList_.erase(mIter);
    }

    return nullptr;
}

bool atlas_page::add_font(const std::string& sFontName,
    std::shared_ptr<gui::font> pFont)
{
    try
    {
        if (const auto pMat = pFont->get_texture().lock())
        {
            const auto mRect = pMat->get_rect();
            const auto mLocation = find_location_(mRect.width(), mRect.height());
            if (!mLocation.has_value())
                return false;

            std::shared_ptr<gui::material> pTex = add_material_(*pMat, mLocation.value());
            pFont->update_texture(pTex);

            lFontList_[sFontName] = pFont;
            return true;
        }
        else
            return false;
    }
    catch (const std::exception& e)
    {
        gui::out << gui::warning << e.what() << std::endl;
        return false;
    }
}

bool atlas_page::empty() const
{
    for (const auto& pMat : lTextureList_)
    {
        if (std::shared_ptr<gui::material> pLock = pMat.second.lock())
            return false;
    }

    for (const auto& pFont : lFontList_)
    {
        if (std::shared_ptr<gui::font> pLock = pFont.second.lock())
            return false;
    }

    return true;
}

std::optional<bounds2f> atlas_page::find_location_(float fWidth, float fHeight) const
{
    constexpr float fPadding = 1.0f; // pixels

    bounds2f mStartQuad(0, fWidth, 0, fHeight);
    if (empty())
        return mStartQuad;

    const float fAtlasWidth = get_width();
    const float fAtlasHeight = get_height();

    std::vector<bounds2f> lOccupiedSpace;
    lOccupiedSpace.reserve(lTextureList_.size());

    float fMaxWidth = 0.0f;
    float fMaxHeight = 0.0f;

    auto apply_padding = [&](bounds2f mRect)
    {
        mRect.right += fPadding;
        mRect.bottom += fPadding;
        return mRect;
    };

    for (const auto& pMat : lTextureList_)
    {
        if (std::shared_ptr<gui::material> pLock = pMat.second.lock())
        {
            lOccupiedSpace.push_back(apply_padding(pLock->get_rect()));
            fMaxWidth = std::max(fMaxWidth, lOccupiedSpace.back().right);
            fMaxHeight = std::max(fMaxHeight, lOccupiedSpace.back().bottom);
        }
    }

    for (const auto& pFont : lFontList_)
    {
        if (std::shared_ptr<gui::font> pLock = pFont.second.lock())
        {
            lOccupiedSpace.push_back(apply_padding(pLock->get_texture().lock()->get_rect()));
            fMaxWidth = std::max(fMaxWidth, lOccupiedSpace.back().right);
            fMaxHeight = std::max(fMaxHeight, lOccupiedSpace.back().bottom);
        }
    }

    float fBestArea = std::numeric_limits<float>::infinity();
    bounds2f mBestQuad;

    for (const auto& mRectSource : lOccupiedSpace)
    {
        auto mTestPosition = [&](const vector2f& mPos)
        {
            const bounds2f mTestQuad = mStartQuad + mPos;
            if (mTestQuad.right > fAtlasWidth || mTestQuad.bottom > fAtlasHeight)
                return;

            const float fNewMaxWidth = std::max(fMaxWidth, mTestQuad.right);
            const float fNewMaxHeight = std::max(fMaxHeight, mTestQuad.bottom);
            const float fNewArea = fNewMaxWidth*fNewMaxHeight;

            if (fNewArea >= fBestArea)
                return;

            for (const auto& mRectOther : lOccupiedSpace)
            {
                if (mTestQuad.overlaps(mRectOther))
                    return;
            }

            fBestArea = fNewArea;
            mBestQuad = mTestQuad;
        };

        mTestPosition(mRectSource.top_right());
        mTestPosition(mRectSource.bottom_left());
    }

    if (std::isfinite(fBestArea))
        return mBestQuad;
    else
        return std::nullopt;
}

atlas::atlas(const renderer& mRenderer, material::filter mFilter) :
    mRenderer_(mRenderer), mFilter_(mFilter) {}

std::shared_ptr<gui::material> atlas::fetch_material(const std::string& sFileName) const
{
    for (const auto& mPageItem : lPageList_)
    {
        auto pTex = mPageItem.pPage->fetch_material(sFileName);
        if (pTex)
            return pTex;
    }

    return nullptr;
}

std::shared_ptr<gui::material> atlas::add_material(const std::string& sFileName, const material& mMat) const
{
    try
    {
        for (const auto& mPageItem : lPageList_)
        {
            auto pTex = mPageItem.pPage->add_material(sFileName, mMat);
            if (pTex)
                return pTex;

            if (mPageItem.pPage->empty())
            {
                gui::out << gui::warning << "Could not fit texture '" << sFileName <<
                    "' on any atlas page." << std::endl;
                return nullptr;
            }
        }

        add_page_();
        auto pTex = lPageList_.back().pPage->add_material(sFileName, mMat);
        if (pTex)
            return pTex;

        return nullptr;
    }
    catch (const std::exception& e)
    {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

std::shared_ptr<gui::font> atlas::fetch_font(const std::string& sFontName) const
{
    for (const auto& mPageItem : lPageList_)
    {
        auto pFont = mPageItem.pPage->fetch_font(sFontName);
        if (pFont)
            return pFont;
    }

    return nullptr;
}

bool atlas::add_font(const std::string& sFontName, std::shared_ptr<gui::font> pFont) const
{
    try
    {
        for (const auto& mPageItem : lPageList_)
        {
            if (mPageItem.pPage->add_font(sFontName, pFont))
                return true;

            if (mPageItem.pPage->empty())
            {
                gui::out << gui::warning << "Could not fit font '" << sFontName <<
                    "' on any atlas page." << std::endl;
                return false;
            }
        }

        add_page_();
        if (lPageList_.back().pPage->add_font(sFontName, pFont))
            return true;

        return false;
    }
    catch (const std::exception& e)
    {
        gui::out << gui::warning << e.what() << std::endl;
        return false;
    }
}

std::size_t atlas::get_num_pages() const
{
    return lPageList_.size();
}

void atlas::add_page_() const
{
    page_item mPage;
    mPage.pPage = create_page_();

    // Add a white pixel as the first material in the atlas.
    // This can be used for optimizing quad batching, to render
    // quads with no texture.
    ub32color mPixel(255,255,255,255);
    auto pTex = mRenderer_.create_material(vector2ui(1u, 1u), &mPixel);
    mPage.pNoTextureMat = mPage.pPage->add_material("", *pTex);

    lPageList_.push_back(std::move(mPage));
}

}
}
