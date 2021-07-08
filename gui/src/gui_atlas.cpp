#include "lxgui/gui_atlas.hpp"
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

bool atlas_page::empty() const
{
    for (const auto& pMat : lTextureList_)
    {
        if (std::shared_ptr<gui::material> pLock = pMat.second.lock())
            return false;
    }

    return true;
}

std::optional<quad2f> atlas_page::find_location_(float fWidth, float fHeight) const
{
    quad2f mStartQuad(0, fWidth, 0, fHeight);
    if (empty())
        return mStartQuad;

    const float fAtlasWidth = get_width();
    const float fAtlasHeight = get_height();

    std::vector<quad2f> lOccupiedSpace;
    lOccupiedSpace.reserve(lTextureList_.size());

    float fMaxWidth = 0.0f;
    float fMaxHeight = 0.0f;

    for (const auto& pMat : lTextureList_)
    {
        if (std::shared_ptr<gui::material> pLock = pMat.second.lock())
        {
            lOccupiedSpace.push_back(pLock->get_rect());
            fMaxWidth = std::max(fMaxWidth, lOccupiedSpace.back().right);
            fMaxHeight = std::max(fMaxHeight, lOccupiedSpace.back().bottom);
        }
    }

    float fBestArea = std::numeric_limits<float>::infinity();
    quad2f mBestQuad;

    for (const auto& mRectSource : lOccupiedSpace)
    {
        auto mTestPosition = [&](const vector2f& mPos)
        {
            const quad2f mTestQuad = mStartQuad + mPos;
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

atlas::atlas(material::filter mFilter) : mFilter_(mFilter) {}

std::shared_ptr<gui::material> atlas::fetch_material(const std::string& sFileName) const
{
    for (const auto& pPage : lPageList_)
    {
        auto pTex = pPage->fetch_material(sFileName);
        if (pTex)
            return pTex;
    }

    return nullptr;
}

std::shared_ptr<gui::material> atlas::add_material(const std::string& sFileName, const material& mMat) const
{
    try
    {
        for (const auto& lPage : lPageList_)
        {
            auto pTex = lPage->add_material(sFileName, mMat);
            if (pTex)
                return pTex;

            if (lPage->empty())
            {
                gui::out << gui::warning << "Could not fit '" << sFileName <<
                    "' on any atlas page." << std::endl;
                return nullptr;
            }
        }

        add_page_();
        auto pTex = lPageList_.back()->add_material(sFileName, mMat);
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

void atlas::add_page_() const
{
    lPageList_.push_back(create_page_());
}

}
}
