#include "lxgui/gui_framerenderer.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_range.hpp>

namespace lxgui {
namespace gui
{
// For debugging only
uint count_frames(const std::array<strata,8>& lStrataList)
{
    uint uiCount = 0;
    for (uint uiStrata = 0; uiStrata < lStrataList.size(); ++uiStrata)
    {
        for (const auto& mLevel : utils::range::value(lStrataList[uiStrata].lLevelList))
        {
            uiCount += mLevel.lFrameList.size();
        }
    }

    return uiCount;
}

// For debugging only
void print_frames(const std::array<strata,8>& lStrataList)
{
    for (uint uiStrata = 0; uiStrata < lStrataList.size(); ++uiStrata)
    {
        if (lStrataList[uiStrata].lLevelList.empty()) continue;
        gui::out << "strata[" << uiStrata << "]" << std::endl;
        for (const auto& mLevel : lStrataList[uiStrata].lLevelList)
        {
            if (mLevel.second.lFrameList.empty()) continue;
            gui::out << "  level[" << mLevel.first << "]" << std::endl;
            for (const auto* pFrame : mLevel.second.lFrameList)
                gui::out << "    " << pFrame << " " << pFrame->get_name() << std::endl;
        }
    }
}

void frame_renderer::notify_strata_needs_redraw_(const strata& mStrata) const
{
    mStrata.bRedraw = true;
}

void frame_renderer::notify_strata_needs_redraw(frame_strata mStrata) const
{
    notify_strata_needs_redraw_(lStrataList_[(uint)mStrata]);
}

void frame_renderer::notify_rendered_frame(frame* pFrame, bool bRendered)
{
    if (!pFrame)
        return;

    if (pFrame->get_frame_strata() == frame_strata::PARENT)
    {
        throw gui::exception("gui::frame_renderer", "cannot use PARENT strata for renderer");
    }

    const auto mFrameStrata = pFrame->get_frame_strata();
    auto& mStrata = lStrataList_[(uint)mFrameStrata];

    if (bRendered)
        add_to_strata_list_(mStrata, pFrame);
    else
        remove_from_strata_list_(mStrata, pFrame);

    notify_strata_needs_redraw_(mStrata);
}

void frame_renderer::notify_frame_strata_changed(frame* pFrame, frame_strata mOldStrata,
                                           frame_strata mNewStrata)
{
    if (mOldStrata == frame_strata::PARENT || mNewStrata == frame_strata::PARENT)
    {
        throw gui::exception("gui::frame_renderer", "cannot use PARENT strata for renderer");
    }

    auto& mOld = lStrataList_[(uint)mOldStrata];
    auto& mNew = lStrataList_[(uint)mNewStrata];
    remove_from_strata_list_(mOld, pFrame);
    add_to_strata_list_(mNew, pFrame);

    notify_strata_needs_redraw_(mOld);
    notify_strata_needs_redraw_(mNew);
}

void frame_renderer::notify_frame_level_changed(frame* pFrame, int iOldLevel, int iNewLevel)
{
    if (pFrame->get_frame_strata() == frame_strata::PARENT)
    {
        throw gui::exception("gui::frame_renderer", "cannot use PARENT strata for renderer");
    }

    const auto mFrameStrata = pFrame->get_frame_strata();
    auto& mStrata = lStrataList_[(uint)mFrameStrata];
    auto& lLevelList = mStrata.lLevelList;

    auto mIterOld = lLevelList.find(iOldLevel);
    if (mIterOld != lLevelList.end())
    {
        remove_from_level_list_(mIterOld->second, pFrame);
        if (mIterOld->second.lFrameList.empty())
            mStrata.lLevelList.erase(mIterOld);
    }

    auto mIterNew = lLevelList.find(iNewLevel);
    if (mIterNew == lLevelList.end())
    {
        mIterNew = lLevelList.insert(std::make_pair(iNewLevel, level{})).first;
        mIterNew->second.pStrata = &mStrata;
    }

    add_to_level_list_(mIterNew->second, pFrame);

    notify_strata_needs_redraw_(mStrata);
}

void frame_renderer::add_to_strata_list_(strata& mStrata, frame* pFrame)
{
    int iNewLevel = pFrame->get_level();
    auto mIterNew = mStrata.lLevelList.find(iNewLevel);
    if (mIterNew == mStrata.lLevelList.end())
    {
        mIterNew = mStrata.lLevelList.insert(std::make_pair(iNewLevel, level{})).first;
        mIterNew->second.pStrata = &mStrata;
    }

    add_to_level_list_(mIterNew->second, pFrame);
}

void frame_renderer::remove_from_strata_list_(strata& mStrata, frame* pFrame)
{
    auto mIter = mStrata.lLevelList.find(pFrame->get_level());
    if (mIter == mStrata.lLevelList.end())
    {
        throw gui::exception("gui::frame_renderer", "frame not found in this strata and level");
    }

    remove_from_level_list_(mIter->second, pFrame);

    if (mIter->second.lFrameList.empty())
        mStrata.lLevelList.erase(mIter);
}

void frame_renderer::add_to_level_list_(level& mLevel, frame* pFrame)
{
    mLevel.lFrameList.push_back(pFrame);
    notify_strata_needs_redraw_(*mLevel.pStrata);
    bStrataListUpdated_ = true;
}

void frame_renderer::remove_from_level_list_(level& mLevel, frame* pFrame)
{
    auto mIter = std::find(mLevel.lFrameList.begin(), mLevel.lFrameList.end(), pFrame);
    if (mIter == mLevel.lFrameList.end())
    {
        throw gui::exception("gui::frame_renderer", "frame not found in this strata and level");
    }

    mLevel.lFrameList.erase(mIter);
    notify_strata_needs_redraw_(*mLevel.pStrata);
    bStrataListUpdated_ = true;
}

void frame_renderer::render_strata_(const strata& mStrata) const
{
    for (const auto& mLevel : utils::range::value(mStrata.lLevelList))
    {
        for (auto* pFrame : mLevel.lFrameList)
        {
            if (!pFrame->is_newly_created())
                pFrame->render();
        }
    }

    ++mStrata.uiRedrawCount;
}

void frame_renderer::clear_strata_list_()
{
    for (auto& mStrata : lStrataList_)
    {
        mStrata.lLevelList.clear();
        mStrata.pRenderTarget = nullptr;
        mStrata.uiRedrawCount = 0u;
        mStrata.bRedraw = true;
    }

    bStrataListUpdated_ = true;
}

bool frame_renderer::has_strata_list_changed_() const
{
    return bStrataListUpdated_;
}

void frame_renderer::reset_strata_list_changed_flag_()
{
    bStrataListUpdated_ = false;
}

frame* frame_renderer::find_hovered_frame_(float fX, float fY)
{
    // Iterate through the frames in reverse order from rendering (frame on top goes first)
    for (const auto& mStrata : utils::range::reverse(lStrataList_))
    {
        for (const auto& mLevel : utils::range::reverse_value(mStrata.lLevelList))
        {
            for (auto* pFrame : utils::range::reverse(mLevel.lFrameList))
            {
                if (pFrame->is_mouse_enabled() && pFrame->is_visible() && pFrame->is_in_frame(fX, fY))
                    return pFrame;
            }
        }
    }

    return nullptr;
}
}
}
