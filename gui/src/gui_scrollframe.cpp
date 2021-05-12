#include "lxgui/gui_scrollframe.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"

namespace lxgui {
namespace gui
{
scroll_frame::scroll_frame(manager* pManager) : frame(pManager)
{
    lType_.push_back(CLASS_NAME);
}

scroll_frame::~scroll_frame()
{
    // Make sure the scroll child is destroyed now.
    // It relies on this scroll_frame still being alive
    // when being destroyed, but the scroll_frame destructor
    // would be called before its inherited frame destructor
    // (which would otherwise take care of destroying the scroll child).
    if (pScrollChild_)
        remove_child(pScrollChild_);
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

void scroll_frame::on_script(const std::string& sScriptName, event* pEvent)
{
    alive_checker mChecker(this);
    frame::on_script(sScriptName, pEvent);
    if (!mChecker.is_alive())
        return;

    if (sScriptName == "OnSizeChanged")
        bRebuildScrollRenderTarget_ = true;
}

void scroll_frame::copy_from(uiobject* pObj)
{
    frame::copy_from(pObj);

    scroll_frame* pScrollFrame = down_cast<scroll_frame>(pObj);
    if (!pScrollFrame)
        return;

    this->set_horizontal_scroll(pScrollFrame->get_horizontal_scroll());
    this->set_vertical_scroll(pScrollFrame->get_vertical_scroll());

    auto* pOtherChild = pScrollFrame->get_scroll_child();
    if (pOtherChild)
    {
        frame* pScrollChild = create_child(pOtherChild->get_object_type(),
                pOtherChild->get_raw_name(), {pOtherChild});
        pScrollChild->notify_loaded();

        if (pScrollChild)
            this->set_scroll_child(remove_child(pScrollChild));
    }
}

void scroll_frame::set_scroll_child(std::unique_ptr<frame> pFrame)
{
    if (pScrollChild_)
    {
        pScrollChild_->set_renderer(nullptr);
        pScrollChild_->modify_point(anchor_point::TOPLEFT)->set_abs_offset(
            lBorderList_.top_left() - vector2f(fHorizontalScroll_, fVerticalScroll_)
        );

        clear_strata_list_();
    }
    else if (!is_virtual() && !pScrollTexture_)
    {
        // Create the scroll texture
        std::unique_ptr<texture> pScrollTexture(new texture(pManager_));
        pScrollTexture->set_special();
        pScrollTexture->set_draw_layer("ARTWORK");
        pScrollTexture->set_name_and_parent("$parentScrollTexture", this);

        if (!pManager_->add_uiobject(pScrollTexture.get()))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to create scroll texture for \""+sName_+"\", "
                "but its name was already taken : \""+pScrollTexture->get_name()+"\". Skipped." << std::endl;
            return;
        }

        pScrollTexture->create_glue();
        pScrollTexture->set_all_points(this);

        if (pScrollRenderTarget_)
            pScrollTexture->set_texture(pScrollRenderTarget_);

        pScrollTexture->notify_loaded();
        pScrollTexture_ = pScrollTexture.get();
        add_region(std::move(pScrollTexture));

        bRebuildScrollRenderTarget_ = true;
    }

    pScrollChild_ = pFrame.get();

    if (pScrollChild_)
    {
        pScrollChild_->set_parent(this);

        add_child(std::move(pFrame));

        pScrollChild_->set_special();
        if (!is_virtual())
            pScrollChild_->set_renderer(this);
        pScrollChild_->clear_all_points();
        pScrollChild_->set_abs_point(anchor_point::TOPLEFT, "", anchor_point::TOPLEFT, -fHorizontalScroll_, -fVerticalScroll_);

        fHorizontalScrollRange_ = pScrollChild_->get_abs_width() - fAbsWidth_;
        if (fHorizontalScrollRange_ < 0) fHorizontalScrollRange_ = 0;

        fVerticalScrollRange_ = pScrollChild_->get_abs_height() - fAbsHeight_;
        if (fVerticalScrollRange_ < 0) fVerticalScrollRange_ = 0;

        if (!is_virtual())
        {
            alive_checker mChecker(this);
            on_script("OnScrollRangeChanged");
            if (!mChecker.is_alive())
                return;
        }

        bUpdateScrollRange_ = false;
    }

    bRedrawScrollRenderTarget_ = true;
}

frame* scroll_frame::get_scroll_child()
{
    return pScrollChild_;
}

void scroll_frame::set_horizontal_scroll(float fHorizontalScroll)
{
    if (fHorizontalScroll_ != fHorizontalScroll)
    {
        fHorizontalScroll_ = fHorizontalScroll;
        lQueuedEventList_.push_back("OnHorizontalScroll");

        pScrollChild_->modify_point(anchor_point::TOPLEFT)->set_abs_offset(-fHorizontalScroll_, -fVerticalScroll_);
        bRedrawScrollRenderTarget_ = true;
    }
}

float scroll_frame::get_horizontal_scroll() const
{
    return fHorizontalScroll_;
}

float scroll_frame::get_horizontal_scroll_range() const
{
    return fHorizontalScrollRange_;
}

void scroll_frame::set_vertical_scroll(float fVerticalScroll)
{
    if (fVerticalScroll_ != fVerticalScroll)
    {
        fVerticalScroll_ = fVerticalScroll;
        lQueuedEventList_.push_back("OnVerticalScroll");

        pScrollChild_->modify_point(anchor_point::TOPLEFT)->set_abs_offset(-fHorizontalScroll_, -fVerticalScroll_);
        bRedrawScrollRenderTarget_ = true;
    }
}

float scroll_frame::get_vertical_scroll() const
{
    return fVerticalScroll_;
}

float scroll_frame::get_vertical_scroll_range() const
{
    return fVerticalScrollRange_;
}

void scroll_frame::update(float fDelta)
{
    float fOldChildWidth = 0;
    float fOldChildHeight = 0;

    if (pScrollChild_)
    {
        fOldChildWidth = pScrollChild_->get_abs_width();
        fOldChildHeight = pScrollChild_->get_abs_height();
    }

    alive_checker mChecker(this);
    frame::update(fDelta);
    if (!mChecker.is_alive())
        return;

    if (pScrollChild_ && (fOldChildWidth != pScrollChild_->get_abs_width() ||
        fOldChildHeight != pScrollChild_->get_abs_height()))
    {
        bUpdateScrollRange_ = true;
        bRedrawScrollRenderTarget_ = true;
    }

    if (is_visible())
    {
        if (bRebuildScrollRenderTarget_ && pScrollTexture_)
        {
            rebuild_scroll_render_target_();
            bRebuildScrollRenderTarget_ = false;
            bRedrawScrollRenderTarget_ = true;
        }

        if (bUpdateScrollRange_)
        {
            update_scroll_range_();
            bUpdateScrollRange_ = false;
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
    fHorizontalScrollRange_ = pScrollChild_->get_abs_width() - fAbsWidth_;
    if (fHorizontalScrollRange_ < 0) fHorizontalScrollRange_ = 0;

    fVerticalScrollRange_ = pScrollChild_->get_abs_height() - fAbsHeight_;
    if (fVerticalScrollRange_ < 0) fVerticalScrollRange_ = 0;

    if (!is_virtual())
    {
        alive_checker mChecker(this);
        on_script("OnScrollRangeChanged");
        if (!mChecker.is_alive())
            return;
    }
}

void scroll_frame::update_scroll_child_input_()
{
    float fX = fMousePosX_ - lBorderList_.left;
    float fY = fMousePosY_ - lBorderList_.top;

    update_mouse_in_frame_();
    if (bMouseInScrollTexture_)
    {
        frame* pHoveredFrame = find_hovered_frame_(fX, fY);

        if (pHoveredFrame != pHoveredScrollChild_)
        {
            if (pHoveredScrollChild_)
                pHoveredScrollChild_->notify_mouse_in_frame(false, fX, fY);

            pHoveredScrollChild_ = pHoveredFrame;
        }

        if (pHoveredScrollChild_)
            pHoveredScrollChild_->notify_mouse_in_frame(true, fX, fY);
    }
    else if (pHoveredScrollChild_)
    {
        pHoveredScrollChild_->notify_mouse_in_frame(false, fX, fY);
        pHoveredScrollChild_ = nullptr;
    }
}

void scroll_frame::notify_scaling_factor_updated()
{
    uiobject::notify_scaling_factor_updated();

    rebuild_scroll_render_target_();
}

void scroll_frame::rebuild_scroll_render_target_()
{
    if (fAbsWidth_ == 0 || fAbsHeight_ == 0)
        return;

    float fFactor = pManager_->get_interface_scaling_factor();
    float fScaledWidth = fAbsWidth_*fFactor;
    float fScaledHeight = fAbsHeight_*fFactor;

    if (pScrollRenderTarget_)
    {
        pScrollRenderTarget_->set_dimensions(fScaledWidth, fScaledHeight);

        std::array<float,4> lTexCoords;
        lTexCoords[0] = 0.0f; lTexCoords[1] = 0.0f;
        lTexCoords[2] = fScaledWidth/pScrollRenderTarget_->get_real_width();
        lTexCoords[3] = fScaledHeight/pScrollRenderTarget_->get_real_height();
        pScrollTexture_->set_tex_coord(lTexCoords);
        bUpdateScrollRange_ = true;
    }
    else
    {
        auto* pRenderer = pManager_->get_renderer();
        pScrollRenderTarget_ = pRenderer->create_render_target(fScaledWidth, fScaledHeight);

        if (pScrollRenderTarget_)
            pScrollTexture_->set_texture(pScrollRenderTarget_);
    }
}

void scroll_frame::render_scroll_strata_list_()
{
    pManager_->begin(pScrollRenderTarget_);
    pScrollRenderTarget_->clear(color::EMPTY);

    for (const auto& mStrata : lStrataList_)
    {
        render_strata_(mStrata);
    }

    pManager_->end();
}

bool scroll_frame::is_in_frame(float fX, float fY) const
{
    if (pScrollTexture_)
        return frame::is_in_frame(fX, fY) || pScrollTexture_->is_in_region(fX, fY);
    else
        return frame::is_in_frame(fX, fY);
}

void scroll_frame::notify_mouse_in_frame(bool bMouseInFrame, float fX, float fY)
{
    frame::notify_mouse_in_frame(bMouseInFrame, fX, fY);
    bMouseInScrollTexture_ = (bMouseInFrame && pScrollTexture_ && pScrollTexture_->is_in_region(fX, fY));
}

void scroll_frame::notify_strata_needs_redraw(frame_strata mStrata) const
{
    frame_renderer::notify_strata_needs_redraw(mStrata);

    bRedrawScrollRenderTarget_ = true;
    notify_renderer_need_redraw();
}

void scroll_frame::create_glue()
{
    create_glue_<lua_scroll_frame>();
}

void scroll_frame::notify_rendered_frame(frame* pFrame, bool bRendered)
{
    if (!pFrame)
        return;

    frame_renderer::notify_rendered_frame(pFrame, bRendered);

    if (!bRendered)
    {
        if (pFrame == pHoveredScrollChild_)
        {
            pHoveredScrollChild_->notify_mouse_in_frame(false, 0, 0);
            pHoveredScrollChild_ = nullptr;
        }
    }

    bRedrawScrollRenderTarget_ = true;
}

float scroll_frame::get_target_width() const
{
    return get_apparent_width();
}

float scroll_frame::get_target_height() const
{
    return get_apparent_height();
}

void scroll_frame::update_borders_() const
{
    bool bPositionUpdated = bUpdateBorders_;
    frame::update_borders_();

    if (bPositionUpdated && pScrollChild_)
        pScrollChild_->notify_borders_need_update();
}

}
}
