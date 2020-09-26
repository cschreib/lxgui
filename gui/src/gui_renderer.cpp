#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_renderer_impl.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_sprite.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_out.hpp"

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

renderer::renderer(renderer_impl* pImpl) : pImpl_(pImpl)
{
    for (uint uiStrata = 0; uiStrata < lStrataList_.size(); ++uiStrata)
    {
        lStrataList_[uiStrata].mStrata = (frame_strata)uiStrata;
    }
}

void renderer::fire_redraw(frame_strata mStrata) const
{
    lStrataList_[(uint)mStrata].bRedraw = true;
}

void renderer::notify_rendered_frame(frame* pFrame, bool bRendered)
{
    if (!pFrame)
        return;

    if (pFrame->get_frame_strata() == frame_strata::PARENT)
    {
        throw gui::exception("gui::renderer", "cannot use PARENT strata for renderer");
    }

    auto& mStrata = lStrataList_[(uint)pFrame->get_frame_strata()];

    if (bRendered)
        add_to_strata_list_(mStrata, pFrame);
    else
        remove_from_strata_list_(mStrata, pFrame);
}

void renderer::notify_frame_strata_changed(frame* pFrame, frame_strata mOldStrata,
                                           frame_strata mNewStrata)
{
    if (mOldStrata == frame_strata::PARENT || mNewStrata == frame_strata::PARENT)
    {
        throw gui::exception("gui::renderer", "cannot use PARENT strata for renderer");
    }

    remove_from_strata_list_(lStrataList_[(uint)mOldStrata], pFrame);
    add_to_strata_list_(lStrataList_[(uint)mNewStrata], pFrame);
}

void renderer::notify_frame_level_changed(frame* pFrame, int iOldLevel, int iNewLevel)
{
    if (pFrame->get_frame_strata() == frame_strata::PARENT)
    {
        throw gui::exception("gui::renderer", "cannot use PARENT strata for renderer");
    }

    auto& mStrata = lStrataList_[(uint)pFrame->get_frame_strata()];
    auto& lLevelList = mStrata.lLevelList;

    auto mIterOld = lLevelList.find(iOldLevel);
    if (mIterOld != lLevelList.end())
        remove_from_level_list_(mIterOld->second, pFrame);

    auto mIterNew = lLevelList.find(iNewLevel);
    if (mIterNew == lLevelList.end())
    {
        mIterNew = lLevelList.insert(std::make_pair(iNewLevel, level{})).first;
        mIterNew->second.pStrata = &mStrata;
    }

    add_to_level_list_(mIterNew->second, pFrame);
}

void renderer::render_quad(const quad& mQuad) const
{
    pImpl_->render_quad(mQuad);
}

void renderer::render_quads(const quad& mQuad, const std::vector<std::array<vertex,4>>& lQuadList) const
{
    pImpl_->render_quads(mQuad, lQuadList);
}

sprite renderer::create_sprite(utils::refptr<material> pMat) const
{
    return sprite(this, pMat);
}

sprite renderer::create_sprite(utils::refptr<material> pMat, float fWidth, float fHeight) const
{
    return sprite(this, pMat, fWidth, fHeight);
}

sprite renderer::create_sprite(utils::refptr<material> pMat,
    float fU, float fV, float fWidth, float fHeight) const
{
    return sprite(this, pMat, fU, fV, fWidth, fHeight);
}

utils::refptr<material> renderer::create_material(const std::string& sFileName, material::filter mFilter) const
{
    return pImpl_->create_material(sFileName, mFilter);
}

utils::refptr<material> renderer::create_material(const color& mColor) const
{
    return pImpl_->create_material(mColor);
}

utils::refptr<material> renderer::create_material(utils::refptr<render_target> pRenderTarget) const
{
    return pImpl_->create_material(pRenderTarget);
}

utils::refptr<render_target> renderer::create_render_target(uint uiWidth, uint uiHeight) const
{
    return pImpl_->create_render_target(uiWidth, uiHeight);
}

utils::refptr<font> renderer::create_font(const std::string& sFontFile, uint uiSize) const
{
    return pImpl_->create_font(sFontFile, uiSize);
}

void renderer::add_to_strata_list_(strata& mStrata, frame* pFrame)
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

void renderer::remove_from_strata_list_(strata& mStrata, frame* pFrame)
{
    auto mIter = mStrata.lLevelList.find(pFrame->get_level());
    if (mIter == mStrata.lLevelList.end())
    {
        throw gui::exception("gui::renderer", "frame not found in this strata and level");
    }

    remove_from_level_list_(mIter->second, pFrame);
}

void renderer::add_to_level_list_(level& mLevel, frame* pFrame)
{
    mLevel.lFrameList.push_back(pFrame);
    fire_redraw(mLevel.pStrata->mStrata);
    bStrataListUpdated_ = true;
}

void renderer::remove_from_level_list_(level& mLevel, frame* pFrame)
{
    auto mIter = std::find(mLevel.lFrameList.begin(), mLevel.lFrameList.end(), pFrame);
    if (mIter == mLevel.lFrameList.end())
    {
        throw gui::exception("gui::renderer", "frame not found in this strata and level");
    }

    mLevel.lFrameList.erase(mIter);
    fire_redraw(mLevel.pStrata->mStrata);
    bStrataListUpdated_ = true;
}

void renderer::render_strata_(const strata& mStrata) const
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

void renderer::create_strata_render_target_(strata& mStrata, uint uiWidth, uint uiHeight)
{
    if (mStrata.pRenderTarget)
    {
        mStrata.pRenderTarget->set_dimensions(uiWidth, uiHeight);
    }
    else
    {
        mStrata.pRenderTarget = create_render_target(uiWidth, uiHeight);
    }

    mStrata.mSprite = create_sprite(create_material(mStrata.pRenderTarget));
}

void renderer::clear_strata_list_()
{
    for (auto& mStrata : lStrataList_)
    {
        mStrata.lLevelList.clear();
        mStrata.pRenderTarget = nullptr;
        mStrata.uiRedrawCount = 0u;
        mStrata.bRedraw = false;
    }

    bStrataListUpdated_ = true;
}

bool renderer::has_strata_list_changed_() const
{
    return bStrataListUpdated_;
}

void renderer::reset_strata_list_changed_flag_()
{
    bStrataListUpdated_ = false;
}

frame* renderer::find_hovered_frame_(int iX, int iY)
{
    frame* pHoveredFrame = nullptr;

    // Iterate through the frames in reverse order from rendering (frame on top goes first)
    for (const auto& mStrata : utils::range::reverse(lStrataList_))
    {
        for (const auto& mLevel : utils::range::reverse_value(mStrata.lLevelList))
        {
            for (auto* pFrame : utils::range::reverse(mLevel.lFrameList))
            {
                if (pFrame->is_mouse_enabled() && pFrame->is_visible() && pFrame->is_in_frame(iX, iY))
                {
                    pHoveredFrame = pFrame;
                    break;
                }
            }

            if (pHoveredFrame) break;
        }

        if (pHoveredFrame) break;
    }

    return pHoveredFrame;
}
}
}
