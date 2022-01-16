#include "lxgui/gui_uiroot.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/input_window.hpp"
#include "lxgui/input_dispatcher.hpp"
#include "lxgui/utils_range.hpp"

#include <lxgui/utils_std.hpp>

// #define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

namespace lxgui {
namespace gui
{

uiroot::uiroot(utils::control_block& mBlock, manager& mManager) :
    event_receiver(mBlock, mManager.get_event_emitter()),
    frame_container(mManager, mObjectRegistry_, this),
    mManager_(mManager), mRenderer_(mManager.get_renderer())
{
    mScreenDimensions_ = get_manager().get_window().get_dimensions();

    register_event("WINDOW_RESIZED");
    register_event("MOUSE_MOVED");
    register_event("MOUSE_WHEEL");
    register_event("MOUSE_PRESSED");
    register_event("MOUSE_RELEASED");
    register_event("MOUSE_DRAG_START");
    register_event("MOUSE_DRAG_STOP");
    register_event("KEY_PRESSED");
    register_event("KEY_RELEASED");
    register_event("TEXT_ENTERED");
}

uiroot::~uiroot()
{
    // Must be done before we destroy the registry
    clear_frames_();
}

vector2f uiroot::get_target_dimensions() const
{
    return vector2f(mScreenDimensions_)/get_manager().get_interface_scaling_factor();
}

void uiroot::render() const
{
    mRenderer_.set_view(matrix4f::view(get_target_dimensions()));

    if (bEnableCaching_)
    {
        mRenderer_.render_quad(mScreenQuad_);
    }
    else
    {
        for (const auto& mStrata : lStrataList_)
        {
            render_strata_(mStrata);
        }
    }
}

void uiroot::create_caching_render_target_()
{
    try
    {
        if (pRenderTarget_)
            pRenderTarget_->set_dimensions(mScreenDimensions_);
        else
            pRenderTarget_ = mRenderer_.create_render_target(mScreenDimensions_);
    }
    catch (const utils::exception& e)
    {
        gui::out << gui::error << "gui::uiroot : "
            << "Unable to create render_target for GUI caching :\n" << e.get_description() << std::endl;

        bEnableCaching_ = false;
        return;
    }

    vector2f mScaledDimensions = get_target_dimensions();

    mScreenQuad_.mat = mRenderer_.create_material(pRenderTarget_);
    mScreenQuad_.v[0].pos = vector2f::ZERO;
    mScreenQuad_.v[1].pos = vector2f(mScaledDimensions.x, 0);
    mScreenQuad_.v[2].pos = mScaledDimensions;
    mScreenQuad_.v[3].pos = vector2f(0, mScaledDimensions.y);

    mScreenQuad_.v[0].uvs = mScreenQuad_.mat->get_canvas_uv(vector2f(0, 0), true);
    mScreenQuad_.v[1].uvs = mScreenQuad_.mat->get_canvas_uv(vector2f(1, 0), true);
    mScreenQuad_.v[2].uvs = mScreenQuad_.mat->get_canvas_uv(vector2f(1, 1), true);
    mScreenQuad_.v[3].uvs = mScreenQuad_.mat->get_canvas_uv(vector2f(0, 1), true);
}

void uiroot::create_strata_cache_render_target_(strata& mStrata)
{
    if (mStrata.pRenderTarget)
        mStrata.pRenderTarget->set_dimensions(mScreenDimensions_);
    else
        mStrata.pRenderTarget = mRenderer_.create_render_target(mScreenDimensions_);

    vector2f mScaledDimensions = get_target_dimensions();

    mStrata.mQuad.mat = mRenderer_.create_material(mStrata.pRenderTarget);
    mStrata.mQuad.v[0].pos = vector2f::ZERO;
    mStrata.mQuad.v[1].pos = vector2f(mScaledDimensions.x, 0);
    mStrata.mQuad.v[2].pos = mScaledDimensions;
    mStrata.mQuad.v[3].pos = vector2f(0, mScaledDimensions.y);

    mStrata.mQuad.v[0].uvs = mStrata.mQuad.mat->get_canvas_uv(vector2f(0, 0), true);
    mStrata.mQuad.v[1].uvs = mStrata.mQuad.mat->get_canvas_uv(vector2f(1, 0), true);
    mStrata.mQuad.v[2].uvs = mStrata.mQuad.mat->get_canvas_uv(vector2f(1, 1), true);
    mStrata.mQuad.v[3].uvs = mStrata.mQuad.mat->get_canvas_uv(vector2f(0, 1), true);
}

void uiroot::update(float fDelta)
{
    // Update logics on root frames from parent to children.
    for (auto& mFrame : get_root_frames())
    {
        mFrame.update(fDelta);
    }

    // Removed destroyed frames
    garbage_collect();

    bool bRedraw = has_strata_list_changed_();
    reset_strata_list_changed_flag_();

    if (bRedraw)
        notify_hovered_frame_dirty();

    if (bEnableCaching_)
    {
        DEBUG_LOG(" Redraw strata...");

        try
        {
            for (auto& mStrata : lStrataList_)
            {
                if (mStrata.bRedraw)
                {
                    if (!mStrata.pRenderTarget)
                        create_strata_cache_render_target_(mStrata);

                    if (mStrata.pRenderTarget)
                    {
                        mRenderer_.begin(mStrata.pRenderTarget);

                        vector2f mView = vector2f(mStrata.pRenderTarget->get_canvas_dimensions())/
                            get_manager().get_interface_scaling_factor();

                        mRenderer_.set_view(matrix4f::view(mView));

                        mStrata.pRenderTarget->clear(color::EMPTY);
                        render_strata_(mStrata);

                        mRenderer_.end();
                    }

                    bRedraw = true;
                }

                mStrata.bRedraw = false;
            }

            if (!pRenderTarget_)
                create_caching_render_target_();

            if (bRedraw && pRenderTarget_)
            {
                mRenderer_.begin(pRenderTarget_);

                vector2f mView = vector2f(pRenderTarget_->get_canvas_dimensions())/
                    get_manager().get_interface_scaling_factor();

                mRenderer_.set_view(matrix4f::view(mView));

                pRenderTarget_->clear(color::EMPTY);

                for (auto& mStrata : lStrataList_)
                {
                    mRenderer_.render_quad(mStrata.mQuad);
                }

                mRenderer_.end();
            }
        }
        catch (const utils::exception& e)
        {
            gui::out << gui::error << "gui::uiroot : "
                << "Unable to create render_target for strata :\n"
                << e.get_description() << std::endl;

            bEnableCaching_ = false;
        }
    }
}

void uiroot::toggle_caching()
{
    bEnableCaching_ = !bEnableCaching_;

    if (bEnableCaching_)
    {
        for (auto& mStrata : lStrataList_)
            mStrata.bRedraw = true;
    }
}

void uiroot::enable_caching(bool bEnable)
{
    if (bEnableCaching_ != bEnable)
        toggle_caching();
}

bool uiroot::is_caching_enabled() const
{
    return bEnableCaching_;
}

void uiroot::on_event(const event& mEvent)
{
    if (mEvent.get_name() == "WINDOW_RESIZED")
    {
        // Update internal window size
        mScreenDimensions_ = vector2ui(mEvent.get<std::uint32_t>(0), mEvent.get<std::uint32_t>(1));

        // Notify all frames anchored to the window edges
        for (auto& mFrame : get_root_frames())
        {
            mFrame.notify_borders_need_update();
            mFrame.notify_renderer_need_redraw();
        }

        // Resize caching render targets
        if (pRenderTarget_)
            create_caching_render_target_();

        for (auto& mStrata : lStrataList_)
        {
            if (mStrata.pRenderTarget)
                create_strata_cache_render_target_(mStrata);
        }

        notify_hovered_frame_dirty();
    }
    else if (mEvent.get_name() == "MOUSE_MOVED")
    {
        notify_hovered_frame_dirty();

        if (pMovedObject_ || pSizedObject_)
        {
            DEBUG_LOG(" Moved object...");
            mMouseMovement_ += vector2f(mEvent.get<float>(0), mEvent.get<float>(1));
        }

        if (pMovedObject_)
        {
            switch (mConstraint_)
            {
                case constraint::NONE :
                    pMovedAnchor_->mOffset = mMovementStartPosition_ + mMouseMovement_;
                    break;
                case constraint::X :
                    pMovedAnchor_->mOffset = mMovementStartPosition_ +
                        vector2f(mMouseMovement_.x, 0.0f);
                    break;
                case constraint::Y :
                    pMovedAnchor_->mOffset = mMovementStartPosition_ +
                        vector2f(0.0f, mMouseMovement_.y);
                    break;
                default : break;
            }

            if (mApplyConstraintFunc_)
                mApplyConstraintFunc_();

            // As a result of applying constraints, object may have been deleted,
            // so check again before use
            if (pMovedObject_)
                pMovedObject_->notify_borders_need_update();
        }
        else if (pSizedObject_)
        {
            float fWidth;
            if (bResizeFromRight_)
                fWidth = std::max(0.0f, mResizeStart_.x + mMouseMovement_.x);
            else
                fWidth = std::max(0.0f, mResizeStart_.x - mMouseMovement_.x);

            float fHeight;
            if (bResizeFromBottom_)
                fHeight = std::max(0.0f, mResizeStart_.y + mMouseMovement_.y);
            else
                fHeight = std::max(0.0f, mResizeStart_.y - mMouseMovement_.y);

            if (bResizeWidth_ && bResizeHeight_)
                pSizedObject_->set_dimensions(vector2f(fWidth, fHeight));
            else if (bResizeWidth_)
                pSizedObject_->set_width(fWidth);
            else if (bResizeHeight_)
                pSizedObject_->set_height(fHeight);
        }

        if (pDraggedFrame_)
        {
            event_data mData;
            mData.add(mEvent.get(2));
            mData.add(mEvent.get(3));
            pDraggedFrame_->on_script("OnDragMove", mData);
        }
    }
    else if (mEvent.get_name() == "MOUSE_WHEEL")
    {
        const auto mMousePos = get_manager().get_input_dispatcher().get_mouse_position();

        utils::observer_ptr<frame> pHoveredFrame = find_hovered_frame(mMousePos,
            [](const frame& mFrame)
            {
                return mFrame.is_mouse_wheel_enabled();
            }
        );

        event_data mData;
        mData.add(mEvent.get(0));
        pHoveredFrame->on_script("OnMouseWheel", mData);
    }
    else if (mEvent.get_name() == "MOUSE_DRAG_START")
    {
        const auto mMousePos = get_manager().get_input_dispatcher().get_mouse_position();

        utils::observer_ptr<frame> pHoveredFrame = find_hovered_frame(mMousePos,
            [](const frame& mFrame)
            {
                return mFrame.is_mouse_click_enabled();
            }
        );

        if (!pHoveredFrame)
            return;

        if (auto* pRegion = pHoveredFrame->get_title_region().get();
            pRegion && pRegion->is_in_region(mMousePos))
        {
            pHoveredFrame->start_moving();
        }

        std::string sMouseButton = std::string(input::get_mouse_button_codename(
            mEvent.get<input::mouse_button>(0)));

        if (pHoveredFrame->is_registered_for_drag(sMouseButton))
        {
            event_data mData;
            mData.add(sMouseButton);
            mData.add(mMousePos.x);
            mData.add(mMousePos.y);

            pDraggedFrame_ = std::move(pHoveredFrame);
            pDraggedFrame_->on_script("OnDragStart", mData);
        }
    }
    else if (mEvent.get_name() == "MOUSE_DRAG_STOP")
    {
        stop_moving();

        if (pDraggedFrame_)
        {
            pDraggedFrame_->on_script("OnDragStop");
            pDraggedFrame_ = nullptr;
        }

        const auto mMousePos = get_manager().get_input_dispatcher().get_mouse_position();

        utils::observer_ptr<frame> pHoveredFrame = find_hovered_frame(mMousePos,
            [](const frame& mFrame)
            {
                return mFrame.is_mouse_click_enabled();
            }
        );

        if (!pHoveredFrame)
            return;

        std::string sMouseButton = std::string(input::get_mouse_button_codename(
            mEvent.get<input::mouse_button>(0)));

        if (pHoveredFrame->is_registered_for_drag(sMouseButton))
        {
            event_data mData;
            mData.add(sMouseButton);
            mData.add(mMousePos.x);
            mData.add(mMousePos.y);

            pHoveredFrame->on_script("OnReceiveDrag", mData);
        }
    }
    else if (mEvent.get_name() == "MOUSE_PRESSED" || mEvent.get_name() == "MOUSE_RELEASED" ||
        mEvent.get_name() == "MOUSE_DOUBLE_CLICKED")
    {
        const auto mMousePos = get_manager().get_input_dispatcher().get_mouse_position();

        utils::observer_ptr<frame> pHoveredFrame = find_hovered_frame(mMousePos,
            [](const frame& mFrame)
            {
                return mFrame.is_mouse_click_enabled();
            }
        );

        if (mEvent.get_name() == "MOUSE_PRESSED")
        {
            if (!pHoveredFrame || pHoveredFrame != get_focussed_frame())
                clear_focus();
        }

        if (!pHoveredFrame)
            return;

        event_data mData;
        mData.add(std::string(input::get_mouse_button_codename(
            mEvent.get<input::mouse_button>(0))));
        mData.add(mMousePos.x);
        mData.add(mMousePos.y);

        if (mEvent.get_name() == "MOUSE_PRESSED")
        {
            if (auto* pTopLevel = pHoveredFrame->get_top_level_parent().get())
                pTopLevel->raise();

            pHoveredFrame->on_script("OnMouseDown", mData);
        }
        else if (mEvent.get_name() == "MOUSE_RELEASED")
        {
            pHoveredFrame->on_script("OnMouseUp", mData);
        }
        else if (mEvent.get_name() == "MOUSE_DOUBLE_CLICKED")
        {
            pHoveredFrame->on_script("OnDoubleClicked", mData);
        }
    }
    else if (mEvent.get_name() == "KEY_PRESSED" || mEvent.get_name() == "KEY_RELEASED")
    {
        input::key mKey = mEvent.get<input::key>(0);
        std::string sKeyName = std::string(input::get_key_codename(mKey));

        // First, give priority to the focussed frame
        utils::observer_ptr<frame> pTopmostFrame = get_focussed_frame();

        // TODO:
        // If no focussed frame, look top-down for a frame that captures this key
        // if (!pTopmostFrame)
        // {
        //     pTopmostFrame = find_topmost_frame(
        //         [&](const frame& mFrame)
        //         {
        //             return mFrame.is_key_captured(sKeyName);
        //         }
        //     );
        // }

        // If a frame is found, capture input and return
        if (pTopmostFrame)
        {
            event_data mData;
            mData.add(mEvent.get(0));
            mData.add(sKeyName);

            if (mEvent.get_name() == "KEY_PRESSED")
                pTopmostFrame->on_script("OnKeyDown", mData);
            else if (mEvent.get_name() == "KEY_RELEASED")
                pTopmostFrame->on_script("OnKeyUp", mData);

            return;
        }

        // If no frame is found, try the keybinder
        // TODO: I don't like this design, too tightly coupled
        try
        {
            if (get_keybinder().on_key_down(mKey, get_manager().get_input_dispatcher()))
                return;
        }
        catch (const std::exception& e)
        {
            event mEvent("LUA_ERROR");
            mEvent.add(e.what());
            get_manager().get_event_emitter().fire_event(mEvent);
            return;
        }

        // If no keybinding, just forward to the world
        get_manager().get_world_event_emitter().fire_event(mEvent);
    }
    else if (mEvent.get_name() == "TEXT_ENTERED")
    {
        if (auto pFocus = get_focussed_frame())
        {
            std::uint32_t uiChar = mEvent.get<std::uint32_t>(0);
            event_data mData;
            mData.add(utils::unicode_to_utf8(utils::ustring(1, uiChar)));
            mData.add(uiChar);

            pFocus->on_script("OnChar", mData);
        }
    }
}

void uiroot::notify_scaling_factor_updated()
{
    for (auto& mFrame : get_root_frames())
    {
        mFrame.notify_scaling_factor_updated();
    }

    if (pRenderTarget_)
        create_caching_render_target_();

    for (auto& mStrata : lStrataList_)
    {
        if (mStrata.pRenderTarget)
            create_strata_cache_render_target_(mStrata);
    }
}

void uiroot::update_hovered_frame_()
{
    const auto mMousePos = get_manager().get_input_dispatcher().get_mouse_position();

    utils::observer_ptr<frame> pHoveredFrame = find_hovered_frame(mMousePos,
        [](const frame& mFrame)
        {
            return mFrame.is_mouse_move_enabled();
        }
    );

    set_hovered_frame_(std::move(pHoveredFrame), mMousePos);
}

void uiroot::notify_hovered_frame_dirty()
{
    update_hovered_frame_();
}

void uiroot::start_moving(utils::observer_ptr<uiobject> pObj, anchor* pAnchor,
    constraint mConstraint, std::function<void()> mApplyConstraintFunc)
{
    pSizedObject_ = nullptr;
    pMovedObject_ = pObj;
    mMouseMovement_ = vector2f::ZERO;

    if (pMovedObject_)
    {
        mConstraint_ = mConstraint;
        mApplyConstraintFunc_ = mApplyConstraintFunc;
        if (pAnchor)
        {
            pMovedAnchor_ = pAnchor;
            mMovementStartPosition_ = pMovedAnchor_->mOffset;
        }
        else
        {
            const bounds2f lBorders = pMovedObject_->get_borders();

            pMovedObject_->clear_all_points();
            pMovedObject_->set_point(anchor_data(anchor_point::TOPLEFT, "", lBorders.top_left()));

            pMovedAnchor_ = &pMovedObject_->modify_point(anchor_point::TOPLEFT);

            mMovementStartPosition_ = lBorders.top_left();
        }
    }
}

void uiroot::stop_moving()
{
    pMovedObject_ = nullptr;
    pMovedAnchor_ = nullptr;
}

bool uiroot::is_moving(const uiobject& mObj) const
{
    return pMovedObject_.get() == &mObj;
}

void uiroot::start_sizing(utils::observer_ptr<uiobject> pObj, anchor_point mPoint)
{
    pMovedObject_   = nullptr;
    pSizedObject_   = pObj;
    mMouseMovement_ = vector2f::ZERO;

    if (pSizedObject_)
    {
        const bounds2f lBorders = pSizedObject_->get_borders();

        anchor_point mOppositePoint = anchor_point::CENTER;
        vector2f mOffset;

        switch (mPoint)
        {
            case anchor_point::TOPLEFT :
            case anchor_point::TOP :
                mOppositePoint = anchor_point::BOTTOMRIGHT;
                mOffset = lBorders.bottom_right();
                bResizeFromRight_  = false;
                bResizeFromBottom_ = false;
                break;
            case anchor_point::TOPRIGHT :
            case anchor_point::RIGHT :
                mOppositePoint = anchor_point::BOTTOMLEFT;
                mOffset = lBorders.bottom_left();
                bResizeFromRight_  = true;
                bResizeFromBottom_ = false;
                break;
            case anchor_point::BOTTOMRIGHT :
            case anchor_point::BOTTOM :
                mOppositePoint = anchor_point::TOPLEFT;
                mOffset = lBorders.top_left();
                bResizeFromRight_  = true;
                bResizeFromBottom_ = true;
                break;
            case anchor_point::BOTTOMLEFT :
            case anchor_point::LEFT :
                mOppositePoint = anchor_point::TOPRIGHT;
                mOffset = lBorders.top_right();
                bResizeFromRight_  = false;
                bResizeFromBottom_ = true;
                break;
            case anchor_point::CENTER :
                gui::out << gui::error << "gui::manager : "
                    << "Cannot resize \"" <<  pObj->get_name() << "\" from its center." << std::endl;
                pSizedObject_ = nullptr;
                return;
        }

        pSizedObject_->clear_all_points();
        pSizedObject_->set_point(anchor_data(mOppositePoint, "", anchor_point::TOPLEFT, mOffset));

        mResizeStart_ = pSizedObject_->get_apparent_dimensions();

        if (mPoint == anchor_point::LEFT || mPoint == anchor_point::RIGHT)
        {
            bResizeWidth_  = true;
            bResizeHeight_ = false;
        }
        else if (mPoint == anchor_point::TOP || mPoint == anchor_point::BOTTOM)
        {
            bResizeWidth_  = false;
            bResizeHeight_ = true;
        }
        else
        {
            bResizeWidth_  = true;
            bResizeHeight_ = true;
        }
    }
}

void uiroot::stop_sizing()
{
    pSizedObject_ = nullptr;
}

bool uiroot::is_sizing(const uiobject& mObj) const
{
    return pSizedObject_.get() == &mObj;
}

void release_focus_to_list(const frame& mReceiver, std::vector<utils::observer_ptr<frame>>& lList)
{
    if (lList.empty())
        return;

    // Find receiver in the list
    auto mIter = utils::find_if(lList,
        [&](const auto& pPtr) {
            return pPtr.get() == &mReceiver;
        }
    );

    if (mIter == lList.end())
        return;

    // Set it to null
    *mIter = nullptr;

    // Clean up null entries
    auto mEndIter = std::remove_if(lList.begin(), lList.end(),
        [](const auto& pPtr)
        {
            return pPtr == nullptr;
        }
    );

    lList.erase(mEndIter, lList.end());
}

void request_focus_to_list(utils::observer_ptr<frame> pReceiver,
    std::vector<utils::observer_ptr<frame>>& lList)
{
    auto* pRawPointer = pReceiver.get();
    if (!pRawPointer)
        return;

    // Check if this receiver was already in the focus stack and remove it
    release_focus_to_list(*pRawPointer, lList);

    // Add receiver at the top of the stack
    lList.push_back(std::move(pReceiver));
}

void uiroot::request_focus(utils::observer_ptr<frame> pReceiver)
{
    auto pOldFocus = get_focussed_frame();
    request_focus_to_list(std::move(pReceiver), lFocusStack_);
    auto pNewFocus = get_focussed_frame();

    if (pOldFocus != pNewFocus)
    {
        if (pOldFocus)
            pOldFocus->notify_focus(false);

        if (pNewFocus)
            pNewFocus->notify_focus(true);
    }

    get_manager().get_world_input_dispatcher().block_keyboard_events(get_focussed_frame() != nullptr);
}

void uiroot::release_focus(const frame& mReceiver)
{
    auto pOldFocus = get_focussed_frame();
    release_focus_to_list(mReceiver, lFocusStack_);
    auto pNewFocus = get_focussed_frame();

    if (pOldFocus != pNewFocus)
    {
        if (pOldFocus)
            pOldFocus->notify_focus(false);

        if (pNewFocus)
            pNewFocus->notify_focus(true);
    }

    get_manager().get_world_input_dispatcher().block_keyboard_events(get_focussed_frame() != nullptr);
}

void uiroot::clear_focus()
{
    auto pOldFocus = get_focussed_frame();
    lFocusStack_.clear();

    if (pOldFocus)
        pOldFocus->notify_focus(false);

    get_manager().get_world_input_dispatcher().block_keyboard_events(false);
}

bool uiroot::is_focused() const
{
    return get_focussed_frame() != nullptr;
}

utils::observer_ptr<const frame> uiroot::get_focussed_frame() const
{
    for (const auto& pPtr : utils::range::reverse(lFocusStack_))
    {
        if (pPtr)
            return pPtr;
    }

    return nullptr;
}

void uiroot::clear_hovered_frame_()
{
    pHoveredFrame_ = nullptr;
    get_manager().get_world_input_dispatcher().block_mouse_events(false);
}

void uiroot::set_hovered_frame_(utils::observer_ptr<frame> pFrame, const vector2f& mMousePos)
{
    if (pHoveredFrame_ && pFrame != pHoveredFrame_)
        pHoveredFrame_->notify_mouse_in_frame(false, mMousePos);

    if (pFrame)
    {
        pHoveredFrame_ = pFrame;
        pHoveredFrame_->notify_mouse_in_frame(true, mMousePos);
        get_manager().get_world_input_dispatcher().block_mouse_events(true);
    }
    else
        clear_hovered_frame_();
}

}
}
