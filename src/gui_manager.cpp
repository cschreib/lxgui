#include "lxgui/gui_manager.hpp"

#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_focusframe.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_uiroot.hpp"
#include "lxgui/gui_virtual_uiroot.hpp"
#include "lxgui/gui_factory.hpp"
#include "lxgui/gui_addon_registry.hpp"
#include "lxgui/gui_keybinder.hpp"
#include "lxgui/input.hpp"
#include "lxgui/input_window.hpp"

#include <lxgui/utils_std.hpp>

#include <sstream>

/** \mainpage lxgui documentation
*
* This page allows you to browse the documentation for the C++ API of lxgui.
*
* For the Lua and layout file API, please go to the
* <a href="../lua/index.html">Lua documentation</a>.
*/

// #define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

namespace lxgui {
namespace gui
{

manager::manager(utils::control_block& mBlock, std::unique_ptr<input::source> pInputSource,
    std::unique_ptr<renderer> pRenderer) :
    event_emitter(),
    event_receiver(mBlock, static_cast<event_emitter&>(*this)),
    pInputSource_(std::move(pInputSource)),
    pRenderer_(std::move(pRenderer)),
    pWindow_(std::make_unique<input::window>(*pInputSource_)),
    pInputManager_(utils::make_owned<input::manager>(*pInputSource_)),
    pWorldInputManager_(utils::make_owned<input::manager>(*pInputSource_)),
    pLocalizer_(std::make_unique<localizer>())
{
    set_interface_scaling_factor(1.0f);

    register_event("MOUSE_MOVED");
    register_event("WINDOW_RESIZED");
}

manager::~manager()
{
    close_ui_now();
}

void manager::set_interface_scaling_factor(float fScalingFactor)
{
    float fFullScalingFactor = fScalingFactor*pWindow_->get_interface_scaling_factor_hint();

    if (fFullScalingFactor == fScalingFactor_) return;

    fBaseScalingFactor_ = fScalingFactor;
    fScalingFactor_ = fFullScalingFactor;

    pInputManager_->set_interface_scaling_factor(fScalingFactor_);
    pWorldInputManager_->set_interface_scaling_factor(fScalingFactor_);
    pRoot_->notify_scaling_factor_updated();

    notify_hovered_frame_dirty();
}

float manager::get_interface_scaling_factor() const
{
    return fScalingFactor_;
}

void manager::enable_caching(bool bEnableCaching)
{
    bEnableCaching_ = bEnableCaching;
    if (pRoot_)
        pRoot_->enable_caching(bEnableCaching_);
}

void manager::toggle_caching()
{
    bEnableCaching_ = !bEnableCaching_;
    if (pRoot_)
        pRoot_->enable_caching(bEnableCaching_);
}

bool manager::is_caching_enabled() const
{
    if (pRoot_)
        return pRoot_->is_caching_enabled();
    else
        return bEnableCaching_;
}

void manager::add_addon_directory(const std::string& sDirectory)
{
    if (utils::find(lGUIDirectoryList_, sDirectory) == lGUIDirectoryList_.end())
        lGUIDirectoryList_.push_back(sDirectory);
}

void manager::clear_addon_directory_list()
{
    lGUIDirectoryList_.clear();
}

sol::state& manager::get_lua()
{
    return *pLua_;
}

const sol::state& manager::get_lua() const
{
    return *pLua_;
}

void manager::read_files_()
{
    if (bLoaded_ || pAddOnRegistry_)
        return;

    pAddOnRegistry_ = std::make_unique<addon_registry>(
        get_lua(), get_localizer(), get_event_emitter(), get_root(), get_virtual_root());

    for (const auto& sDirectory : lGUIDirectoryList_)
        pAddOnRegistry_->load_addon_directory(sDirectory);
}

void manager::load_ui()
{
    if (bLoaded_)
        return;

    pFactory_ = std::make_unique<factory>(*this);
    pRoot_ = utils::make_owned<uiroot>(*this);
    pVirtualRoot_ = utils::make_owned<virtual_uiroot>(*this, get_root().get_registry());

    pInputManager_->register_event_emitter(
        utils::observer_ptr<event_emitter>(observer_from_this(), static_cast<event_emitter*>(this)));
    pWorldInputManager_->register_event_emitter(
        utils::observer_ptr<event_emitter>(observer_from_this(), static_cast<event_emitter*>(this)));

    create_lua_();

    pKeybinder_ = utils::make_owned<keybinder>(get_input_manager(), get_event_emitter());

    read_files_();

    bLoaded_ = true;
    bCloseUI_ = false;
}

void manager::close_ui()
{
    if (bUpdating_)
        bCloseUI_ = true;
    else
        close_ui_now();
}

void manager::close_ui_now()
{
    if (!bLoaded_)
        return;

    if (pAddOnRegistry_)
        pAddOnRegistry_->save_variables();

    pVirtualRoot_ = nullptr;
    pRoot_ = nullptr;
    pFactory_ = nullptr;
    pAddOnRegistry_ = nullptr;
    pKeybinder_ = nullptr;
    pLua_ = nullptr;

    pHoveredFrame_ = nullptr;
    pMovedObject_ = nullptr;
    pSizedObject_ = nullptr;
    pMovedAnchor_ = nullptr;
    mMouseMovement_ = vector2f::ZERO;
    mMovementStartPosition_ = vector2f::ZERO;
    mConstraint_ = constraint::NONE;
    mResizeStart_ = vector2f::ZERO;
    bResizeWidth_ = false;
    bResizeHeight_ = false;
    bResizeFromRight_ = false;
    bResizeFromBottom_ = false;

    pLocalizer_->clear_translations();

    pInputManager_->unregister_event_emitter(*this);
    pWorldInputManager_->unregister_event_emitter(*this);

    bLoaded_ = false;
}

void manager::reload_ui()
{
    if (bUpdating_)
        bReloadUI_ = true;
    else
        reload_ui_now();
}

void manager::reload_ui_now()
{
    gui::out << "Closing UI..." << std::endl;
    close_ui_now();
    gui::out << "Done. Loading UI..." << std::endl;
    load_ui();
    gui::out << "Done." << std::endl;

    bReloadUI_ = false;
}

void manager::render_ui() const
{
    pRenderer_->begin();

    pRoot_->render();

    pRenderer_->end();
}

bool manager::is_loaded() const
{
    return bLoaded_;
}

void manager::update_ui(float fDelta)
{
    bUpdating_ = true;

    DEBUG_LOG(" Update widgets...");
    pRoot_->update(fDelta);

    if (bFirstIteration_)
    {
        DEBUG_LOG(" Entering world...");
        fire_event(event("ENTERING_WORLD"));
        bFirstIteration_ = false;

        update_hovered_frame_();
    }

    frame_ended();

    bUpdating_ = false;

    if (bReloadUI_)
        reload_ui_now();
    if (bCloseUI_)
        close_ui_now();
}

void manager::clear_hovered_frame_()
{
    pHoveredFrame_ = nullptr;
    pWorldInputManager_->block_mouse_events(false);
}

void manager::set_hovered_frame_(utils::observer_ptr<frame> pFrame, const vector2f& mMousePos)
{
    if (pHoveredFrame_ && pFrame != pHoveredFrame_)
        pHoveredFrame_->notify_mouse_in_frame(false, mMousePos);

    if (pFrame)
    {
        pHoveredFrame_ = pFrame;
        pHoveredFrame_->notify_mouse_in_frame(true, mMousePos);
        if (pHoveredFrame_->is_world_input_allowed())
            pWorldInputManager_->block_mouse_events(false);
        else
            pWorldInputManager_->block_mouse_events(true);
    }
    else
        clear_hovered_frame_();
}

void manager::start_moving(utils::observer_ptr<uiobject> pObj, anchor* pAnchor,
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

void manager::stop_moving(const uiobject& mObj)
{
    if (pMovedObject_.get() == &mObj)
    {
        pMovedObject_ = nullptr;
        pMovedAnchor_ = nullptr;
    }
}

bool manager::is_moving(const uiobject& mObj) const
{
    return pMovedObject_.get() == &mObj;
}

void manager::start_sizing(utils::observer_ptr<uiobject> pObj, anchor_point mPoint)
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

void manager::stop_sizing(const uiobject& mObj)
{
    if (pSizedObject_.get() == &mObj)
        pSizedObject_ = nullptr;
}

bool manager::is_sizing(const uiobject& mObj) const
{
    return pSizedObject_.get() == &mObj;
}

void manager::update_hovered_frame_()
{
    DEBUG_LOG(" Update hovered frame...");
    const auto mMousePos = pInputManager_->get_mouse_position();

    utils::observer_ptr<frame> pHoveredFrame = pRoot_->find_hovered_frame(mMousePos);
    set_hovered_frame_(std::move(pHoveredFrame), mMousePos);
}

const utils::observer_ptr<frame>& manager::get_hovered_frame() const
{
    return pHoveredFrame_;
}

void manager::notify_hovered_frame_dirty()
{
    update_hovered_frame_();
}

void manager::on_event(const event& mEvent)
{
    if (mEvent.get_name() == "WINDOW_RESIZED")
    {
        // Update the scaling factor
        set_interface_scaling_factor(fBaseScalingFactor_);

        notify_hovered_frame_dirty();

        pRenderer_->notify_window_resized(vector2ui(
            mEvent.get<std::uint32_t>(0), mEvent.get<std::uint32_t>(1)));
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
    }
}

std::string manager::print_ui() const
{
    std::stringstream s;

    s << "\n\n######################## UIObjects ########################\n\n########################\n" << std::endl;
    for (const auto& mFrame : pRoot_->get_root_frames())
    {
        s << mFrame.serialize("") << "\n########################\n" << std::endl;
    }

    s << "\n\n#################### Virtual UIObjects ####################\n\n########################\n" << std::endl;
    for (const auto& mFrame : pVirtualRoot_->get_root_frames())
    {
        s << mFrame.serialize("") << "\n########################\n" << std::endl;
    }

    return s.str();
}

}
}
