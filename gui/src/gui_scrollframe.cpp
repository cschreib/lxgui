#include "lxgui/gui_scrollframe.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/gui_out.hpp"

namespace lxgui {
namespace gui
{
scroll_frame::scroll_frame(manager* pManager) : frame(pManager)
{

    lType_.push_back(CLASS_NAME);
}

scroll_frame::~scroll_frame()
{
}

bool scroll_frame::can_use_script(const std::string& sScriptName) const
{
    if (frame::can_use_script(sScriptName))
        return true;
    else if ((sScriptName == "OnHorizontalScroll") ||
        (sScriptName == "OnScrollRangeChanged") ||
        (sScriptName == "OnVerticalScroll"))
        return true;
    else
        return false;
}

void scroll_frame::on(const std::string& sScriptName, event* pEvent)
{
    frame::on(sScriptName, pEvent);

    if (sScriptName == "SizeChanged")
        bRebuildScrollRenderTarget_ = true;
}

void scroll_frame::copy_from(uiobject* pObj)
{
    frame::copy_from(pObj);

    scroll_frame* pScrollFrame = dynamic_cast<scroll_frame*>(pObj);

    if (pScrollFrame)
    {
        this->set_horizontal_scroll(pScrollFrame->get_horizontal_scroll());
        this->set_vertical_scroll(pScrollFrame->get_vertical_scroll());

        if (pScrollFrame->get_scroll_child())
        {
            frame* pScrollChild = pManager_->create_frame(pScrollFrame->get_scroll_child()->get_object_type());
            if (pScrollChild)
            {
                pScrollChild->set_parent(this);
                if (this->is_virtual())
                    pScrollChild->set_virtual();
                pScrollChild->set_name(pScrollFrame->get_scroll_child()->get_raw_name());
                if (!pManager_->add_uiobject(pScrollChild))
                {
                    gui::out << gui::warning << "gui::" << lType_.back() << " : "
                        "Trying to add \""+pScrollChild->get_name()+"\" to \""+sName_+
                        "\", but its name was already taken : \""+pScrollChild->get_name()+"\". Skipped." << std::endl;
                    delete pScrollChild;
                }
                else
                {
                    pScrollChild->create_glue();
                    this->add_child(pScrollChild);
                    pScrollChild->copy_from(pScrollFrame->get_scroll_child());
                    pScrollChild->notify_loaded();

                    this->set_scroll_child(pScrollChild);
                }
            }
        }
    }
}

void scroll_frame::set_scroll_child(frame* pFrame)
{
    if (pScrollChild_)
    {
        pScrollChild_->set_manually_rendered(false);
        pScrollChild_->modify_point(anchor_point::TOPLEFT)->set_abs_offset(
            lBorderList_.top_left() - vector2i(iHorizontalScroll_, iVerticalScroll_)
        );

        lScrollChildList_.clear();
        lScrollStrataList_.clear();
    }
    else if (!is_virtual() && !pScrollTexture_)
    {
        // create_ the scroll texture
        pScrollTexture_ = new texture(pManager_);
        pScrollTexture_->set_special();
        pScrollTexture_->set_parent(this);
        pScrollTexture_->set_draw_layer("ARTWORK");
        pScrollTexture_->set_name("$parentScrollTexture");

        if (!pManager_->add_uiobject(pScrollTexture_))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to create scroll texture for \""+sName_+"\",\n"
                "but its name was already taken : \""+pScrollTexture_->get_name()+"\". Skipped." << std::endl;
            delete pScrollTexture_; pScrollTexture_ = nullptr;
            return;
        }

        pScrollTexture_->create_glue();
        add_region(pScrollTexture_);

        pScrollTexture_->set_all_points(this);

        if (pScrollRenderTarget_)
            pScrollTexture_->set_texture(pScrollRenderTarget_);

        pScrollTexture_->notify_loaded();

        bRebuildScrollRenderTarget_ = true;
    }

    pScrollChild_ = pFrame;

    if (pScrollChild_)
    {
        if (pScrollChild_->get_parent() != this)
        {
            pScrollChild_->set_parent(this);
            add_child(pScrollChild_);
        }

        pScrollChild_->set_manually_rendered(true, this);
        pScrollChild_->clear_all_points();
        pScrollChild_->set_abs_point(anchor_point::TOPLEFT, "", anchor_point::TOPLEFT, -iHorizontalScroll_, -iVerticalScroll_);

        add_to_scroll_child_list_(pScrollChild_);

        iHorizontalScrollRange_ = int(pScrollChild_->get_abs_width()) - int(uiAbsWidth_);
        if (iHorizontalScrollRange_ < 0) iHorizontalScrollRange_ = 0;

        iVerticalScrollRange_ = int(pScrollChild_->get_abs_height()) - int(uiAbsHeight_);
        if (iVerticalScrollRange_ < 0) iVerticalScrollRange_ = 0;

        on("ScrollRangeChanged");

        bUpdateScrollRange_ = false;
    }

    bRebuildScrollStrataList_ = true;
    fire_redraw();
}

frame* scroll_frame::get_scroll_child()
{
    return pScrollChild_;
}

void scroll_frame::set_horizontal_scroll(int iHorizontalScroll)
{
    if (iHorizontalScroll_ != iHorizontalScroll)
    {
        iHorizontalScroll_ = iHorizontalScroll;
        lQueuedEventList_.push_back("HorizontalScroll");

        pScrollChild_->modify_point(anchor_point::TOPLEFT)->set_abs_offset(-iHorizontalScroll_, -iVerticalScroll_);
        fire_redraw();
    }
}

int scroll_frame::get_horizontal_scroll() const
{
    return iHorizontalScroll_;
}

int scroll_frame::get_horizontal_scroll_range() const
{
    return iHorizontalScrollRange_;
}

void scroll_frame::set_vertical_scroll(int iVerticalScroll)
{
    if (iVerticalScroll_ != iVerticalScroll)
    {
        iVerticalScroll_ = iVerticalScroll;
        lQueuedEventList_.push_back("VerticalScroll");

        pScrollChild_->modify_point(anchor_point::TOPLEFT)->set_abs_offset(-iHorizontalScroll_, -iVerticalScroll_);
        fire_redraw();
    }
}

int scroll_frame::get_vertical_scroll() const
{
    return iVerticalScroll_;
}

int scroll_frame::get_vertical_scroll_range() const
{
    return iVerticalScrollRange_;
}

void scroll_frame::update(float fDelta)
{
    uint uiOldChildWidth = 0;
    uint uiOldChildHeight = 0;

    if (pScrollChild_)
    {
        uiOldChildWidth = pScrollChild_->get_abs_width();
        uiOldChildHeight = pScrollChild_->get_abs_height();
    }

    frame::update(fDelta);

    if (pScrollChild_ && (uiOldChildWidth != pScrollChild_->get_abs_width() ||
        uiOldChildHeight != pScrollChild_->get_abs_height()))
    {
        bUpdateScrollRange_ = true;
        fire_redraw();
    }

    if (is_visible())
    {
        if (bRebuildScrollRenderTarget_ && pScrollTexture_)
        {
            rebuild_scroll_render_target_();
            bRebuildScrollRenderTarget_ = false;
            fire_redraw();
        }

        if (bUpdateScrollRange_)
        {
            update_scroll_range_();
            bUpdateScrollRange_ = false;
        }

        if (bRebuildScrollStrataList_)
        {
            rebuild_scroll_strata_list_();
            bRebuildScrollStrataList_ = false;
            fire_redraw();
        }

        if (pScrollChild_)
            update_scroll_child_input_();

        if (pScrollChild_ && pScrollRenderTarget_ && bRedrawScrollRenderTarget_)
        {
            render_scroll_strata_list_();
            bRedrawScrollRenderTarget_ = false;
        }
    }
}

void scroll_frame::update_scroll_range_()
{
    iHorizontalScrollRange_ = int(pScrollChild_->get_abs_width()) - int(uiAbsWidth_);
    if (iHorizontalScrollRange_ < 0) iHorizontalScrollRange_ = 0;

    iVerticalScrollRange_ = int(pScrollChild_->get_abs_height()) - int(uiAbsHeight_);
    if (iVerticalScrollRange_ < 0) iVerticalScrollRange_ = 0;

    on("ScrollRangeChanged");
}

void scroll_frame::update_scroll_child_input_()
{
    int iX = iMousePosX_ - lBorderList_.left;
    int iY = iMousePosY_ - lBorderList_.top;

    if (bMouseInScrollTexture_)
    {
        frame* pHoveredFrame = nullptr;
        for (const auto& mStrata : utils::range::reverse_value(lScrollStrataList_))
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

        if (pHoveredFrame != pHoveredScrollChild_)
        {
            if (pHoveredScrollChild_)
                pHoveredScrollChild_->notify_mouse_in_frame(false, iX, iY);

            pHoveredScrollChild_ = pHoveredFrame;
        }

        if (pHoveredScrollChild_)
            pHoveredScrollChild_->notify_mouse_in_frame(true, iX, iY);
    }
    else if (pHoveredScrollChild_)
    {
        pHoveredScrollChild_->notify_mouse_in_frame(false, iX, iY);
        pHoveredScrollChild_ = nullptr;
    }
}

void scroll_frame::rebuild_scroll_render_target_()
{
    if (uiAbsWidth_ == 0 || uiAbsHeight_ == 0)
        return;

    if (pScrollRenderTarget_)
    {
        pScrollRenderTarget_->set_dimensions(uiAbsWidth_, uiAbsHeight_);
        std::array<float,4> lTexCoords;
        lTexCoords[0] = 0.0f; lTexCoords[1] = 0.0f;
        lTexCoords[2] = float(uiAbsWidth_)/pScrollRenderTarget_->get_real_width();
        lTexCoords[3] = float(uiAbsHeight_)/pScrollRenderTarget_->get_real_height();
        pScrollTexture_->set_tex_coord(lTexCoords);
        bUpdateScrollRange_ = true;
    }
    else
    {
        pScrollRenderTarget_ = pManager_->create_render_target(uiAbsWidth_, uiAbsHeight_);

        if (pScrollRenderTarget_)
            pScrollTexture_->set_texture(pScrollRenderTarget_);
    }
}

void scroll_frame::rebuild_scroll_strata_list_()
{
    lScrollStrataList_.clear();

    for (auto* pFrame : utils::range::value(lScrollChildList_))
    {
        lScrollStrataList_[pFrame->get_frame_strata()].
            lLevelList[pFrame->get_frame_level()].
                lFrameList.push_back(pFrame);
    }
}

void scroll_frame::render_scroll_strata_list_()
{
    pManager_->begin(pScrollRenderTarget_);
    pScrollRenderTarget_->clear(color::EMPTY);

    for (const auto& mStrata : utils::range::value(lScrollStrataList_))
    {
        for (const auto& mLevel : utils::range::value(mStrata.lLevelList))
        {
            for (auto* pFrame : mLevel.lFrameList)
            {
                if (!pFrame->is_newly_created())
                    pFrame->render();
            }
        }
    }

    pManager_->end();
}

bool scroll_frame::is_in_frame(int iX, int iY) const
{
    if (pScrollTexture_)
        return frame::is_in_frame(iX, iY) || pScrollTexture_->is_in_region(iX, iY);
    else
        return frame::is_in_frame(iX, iY);
}

void scroll_frame::notify_mouse_in_frame(bool bMouseInFrame, int iX, int iY)
{
    frame::notify_mouse_in_frame(bMouseInFrame, iX, iY);
    bMouseInScrollTexture_ = (bMouseInFrame && pScrollTexture_ && pScrollTexture_->is_in_region(iX, iY));
}

void scroll_frame::fire_redraw() const
{
    bRedrawScrollRenderTarget_ = true;
    notify_renderer_need_redraw();
}

void scroll_frame::notify_child_strata_changed(frame* pChild)
{
    if (pChild == pScrollChild_)
        bRebuildScrollStrataList_ = true;
    else
    {
        if (pParentFrame_)
            pParentFrame_->notify_child_strata_changed(this);
        else
            pManager_->fire_build_strata_list();
    }
}

void scroll_frame::create_glue()
{
    create_glue_<lua_scroll_frame>();
}

void scroll_frame::add_to_scroll_child_list_(frame* pChild)
{
    lScrollChildList_[pChild->get_id()] = pChild;
    for (auto* pSubChild : pChild->get_children())
        add_to_scroll_child_list_(pSubChild);
}

void scroll_frame::remove_from_scroll_child_list_(frame* pChild)
{
    lScrollChildList_.erase(pChild->get_id());
    for (auto* pSubChild : pChild->get_children())
        remove_from_scroll_child_list_(pSubChild);
}

void scroll_frame::notify_manually_rendered_object_(uiobject* pObject, bool bManuallyRendered)
{
    frame* pFrame = dynamic_cast<frame*>(pObject);
    if (pFrame)
    {
        if (bManuallyRendered)
            add_to_scroll_child_list_(pFrame);
        else
            remove_from_scroll_child_list_(pFrame);

        bRebuildScrollStrataList_ = true;
    }
}
}
}
