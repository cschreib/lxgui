#include "lxgui/gui_atlas.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui {
namespace gui
{

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

std::shared_ptr<gui::material> atlas_page::add_material(const std::string& sFileName, const material& mMat) const
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
            return true;
    }

    return false;
}

std::optional<quad2f> atlas_page::find_location_(float fWidth, float fHeight) const
{
    // TODO
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
