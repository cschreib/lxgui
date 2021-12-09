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
scroll_frame::scroll_frame(manager& mManager) : frame(mManager)
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

void scroll_frame::on_script(const std::string& sScriptName, const event_data& mData)
{
    alive_checker mChecker(*this);
    frame::on_script(sScriptName, mData);
    if (!mChecker.is_alive())
        return;

    if (sScriptName == "OnSizeChanged")
        bRebuildScrollRenderTarget_ = true;
}

void scroll_frame::copy_from(const uiobject& mObj)
{
    frame::copy_from(mObj);

    const scroll_frame* pScrollFrame = down_cast<scroll_frame>(&mObj);
    if (!pScrollFrame)
        return;

    this->set_horizontal_scroll(pScrollFrame->get_horizontal_scroll());
    this->set_vertical_scroll(pScrollFrame->get_vertical_scroll());

    if (const frame* pOtherChild = pScrollFrame->get_scroll_child().get())
    {
        utils::observer_ptr<frame> pScrollChild = create_child(pOtherChild->get_object_type(),
                pOtherChild->get_raw_name(), {pScrollFrame->get_scroll_child()});
        pScrollChild->notify_loaded();

        if (pScrollChild)
            this->set_scroll_child(remove_child(pScrollChild));
    }
}

void scroll_frame::set_scroll_child(utils::owner_ptr<frame> pFrame)
{
    if (pScrollChild_)
    {
        pScrollChild_->set_renderer(nullptr);
        pScrollChild_->modify_point(anchor_point::TOPLEFT).mOffset = vector2f(
            lBorderList_.top_left() - vector2f(fHorizontalScroll_, fVerticalScroll_)
        );

        clear_strata_list_();
    }
    else if (!is_virtual() && !pScrollTexture_)
    {
        // Create the scroll texture
        auto pScrollTexture = utils::make_owned<texture>(get_manager());
        pScrollTexture->set_special();
        pScrollTexture->set_draw_layer("ARTWORK");
        pScrollTexture->set_name_and_parent("$parentScrollTexture", observer_from(this));

        if (!get_manager().add_uiobject(pScrollTexture))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to create scroll texture for \""+sName_+"\", "
                "but its name was already taken : \""+pScrollTexture->get_name()+"\". Skipped." << std::endl;
            return;
        }

        pScrollTexture->create_glue();
        pScrollTexture->set_all_points(observer_from(this));

        if (pScrollRenderTarget_)
            pScrollTexture->set_texture(pScrollRenderTarget_);

        pScrollTexture->notify_loaded();
        pScrollTexture_ = pScrollTexture;
        add_region(std::move(pScrollTexture));

        bRebuildScrollRenderTarget_ = true;
    }

    pScrollChild_ = pFrame;

    if (pScrollChild_)
    {
        pScrollChild_->set_parent(observer_from(this));

        add_child(std::move(pFrame));

        pScrollChild_->set_special();
        if (!is_virtual())
            pScrollChild_->set_renderer(observer_from(this));

        pScrollChild_->clear_all_points();

        pScrollChild_->set_point(anchor_data(
            anchor_point::TOPLEFT, "", vector2f(-fHorizontalScroll_, -fVerticalScroll_)));

        update_scroll_range_();
        bUpdateScrollRange_ = false;
    }

    bRedrawScrollRenderTarget_ = true;
}

void scroll_frame::set_horizontal_scroll(float fHorizontalScroll)
{
    if (fHorizontalScroll_ != fHorizontalScroll)
    {
        fHorizontalScroll_ = fHorizontalScroll;
        lQueuedEventList_.push_back("OnHorizontalScroll");

        pScrollChild_->modify_point(anchor_point::TOPLEFT).mOffset =
            vector2f(-fHorizontalScroll_, -fVerticalScroll_);
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

        pScrollChild_->modify_point(anchor_point::TOPLEFT).mOffset =
            vector2f(-fHorizontalScroll_, -fVerticalScroll_);
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
    vector2f mOldChildSize;
    if (pScrollChild_)
        mOldChildSize = pScrollChild_->get_apparent_dimensions();

    alive_checker mChecker(*this);
    frame::update(fDelta);
    if (!mChecker.is_alive())
        return;

    if (pScrollChild_ && mOldChildSize != pScrollChild_->get_apparent_dimensions())
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
    const vector2f mApparentSize = get_apparent_dimensions();
    const vector2f mChildApparentSize = pScrollChild_->get_apparent_dimensions();

    fHorizontalScrollRange_ = mChildApparentSize.x - mApparentSize.x;
    fVerticalScrollRange_ = mChildApparentSize.y - mApparentSize.y;

    if (fHorizontalScrollRange_ < 0) fHorizontalScrollRange_ = 0;
    if (fVerticalScrollRange_ < 0) fVerticalScrollRange_ = 0;

    if (!is_virtual())
    {
        alive_checker mChecker(*this);
        on_script("OnScrollRangeChanged");
        if (!mChecker.is_alive())
            return;
    }
}

void scroll_frame::update_scroll_child_input_()
{
    const vector2f mOffset = mMousePos_ - lBorderList_.top_left();

    update_mouse_in_frame_();
    if (bMouseInScrollTexture_)
    {
        utils::observer_ptr<frame> pHoveredFrame = find_hovered_frame_(mOffset);

        if (pHoveredFrame != pHoveredScrollChild_)
        {
            if (pHoveredScrollChild_)
                pHoveredScrollChild_->notify_mouse_in_frame(false, mOffset);

            pHoveredScrollChild_ = pHoveredFrame;
        }

        if (pHoveredScrollChild_)
            pHoveredScrollChild_->notify_mouse_in_frame(true, mOffset);
    }
    else if (pHoveredScrollChild_)
    {
        pHoveredScrollChild_->notify_mouse_in_frame(false, mOffset);
        pHoveredScrollChild_ = nullptr;
    }
}

void scroll_frame::notify_scaling_factor_updated()
{
    frame::notify_scaling_factor_updated();

    bRebuildScrollRenderTarget_ = true;
}

void scroll_frame::rebuild_scroll_render_target_()
{
    const vector2f mApparentSize = get_apparent_dimensions();

    if (mApparentSize.x <= 0 || mApparentSize.y <= 0)
        return;

    float fFactor = get_manager().get_interface_scaling_factor();
    vector2ui mScaledSize = vector2ui(
        std::round(mApparentSize.x*fFactor), std::round(mApparentSize.y*fFactor));

    if (pScrollRenderTarget_)
    {
        pScrollRenderTarget_->set_dimensions(mScaledSize);
        pScrollTexture_->set_tex_rect(std::array<float,4>{0.0f, 0.0f, 1.0f, 1.0f});
        bUpdateScrollRange_ = true;
    }
    else
    {
        auto& mRenderer = get_manager().get_renderer();
        pScrollRenderTarget_ = mRenderer.create_render_target(mScaledSize);

        if (pScrollRenderTarget_)
            pScrollTexture_->set_texture(pScrollRenderTarget_);
    }
}

void scroll_frame::render_scroll_strata_list_()
{
    get_manager().begin(pScrollRenderTarget_);
    pScrollRenderTarget_->clear(color::EMPTY);

    for (const auto& mStrata : lStrataList_)
    {
        render_strata_(mStrata);
    }

    get_manager().end();
}

bool scroll_frame::is_in_frame(const vector2f& mPosition) const
{
    if (pScrollTexture_)
        return frame::is_in_frame(mPosition) || pScrollTexture_->is_in_region(mPosition);
    else
        return frame::is_in_frame(mPosition);
}

void scroll_frame::notify_mouse_in_frame(bool bMouseInFrame, const vector2f& mMousePos)
{
    frame::notify_mouse_in_frame(bMouseInFrame, mMousePos);
    bMouseInScrollTexture_ = (bMouseInFrame && pScrollTexture_ && pScrollTexture_->is_in_region(mMousePos));
}

void scroll_frame::notify_strata_needs_redraw(frame_strata mStrata) const
{
    frame_renderer::notify_strata_needs_redraw(mStrata);

    bRedrawScrollRenderTarget_ = true;
    notify_renderer_need_redraw();
}

void scroll_frame::create_glue()
{
    create_glue_(this);
}

void scroll_frame::notify_rendered_frame(const utils::observer_ptr<frame>& pFrame, bool bRendered)
{
    if (!pFrame)
        return;

    frame_renderer::notify_rendered_frame(pFrame, bRendered);

    if (!bRendered)
    {
        if (pFrame == pHoveredScrollChild_)
        {
            pHoveredScrollChild_->notify_mouse_in_frame(false, vector2f::ZERO);
            pHoveredScrollChild_ = nullptr;
        }
    }

    bRedrawScrollRenderTarget_ = true;
}

vector2f scroll_frame::get_target_dimensions() const
{
    return get_apparent_dimensions();
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
