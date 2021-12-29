#include "lxgui/gui_manager.hpp"

#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_focusframe.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_uiroot.hpp"
#include "lxgui/gui_virtual_uiroot.hpp"
#include "lxgui/gui_factory.hpp"
#include "lxgui/gui_addon_registry.hpp"
#include "lxgui/input.hpp"

#include <lxgui/utils_string.hpp>
#include <lxgui/utils_filesystem.hpp>
#include <lxgui/utils_std.hpp>
#include <lxgui/utils_range.hpp>

#include <fstream>
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

manager::manager(std::unique_ptr<input::source> pInputSource,
    std::unique_ptr<renderer> pRenderer) :
    event_manager(),
    event_receiver(static_cast<event_manager&>(*this)),
    pInputManager_(new input::manager(std::move(pInputSource))),
    pRenderer_(std::move(pRenderer))
{
    set_interface_scaling_factor(1.0f);

    pLocalizer_ = std::make_unique<localizer>();

    // NB: cannot call register_event() here, as observable_from_this()
    // is not yet fully initialised! This is done in load_ui() instead.
}

manager::~manager()
{
    close_ui_now();
}

void manager::set_interface_scaling_factor(float fScalingFactor)
{
    float fFullScalingFactor = fScalingFactor*pInputManager_->get_interface_scaling_factor_hint();

    if (fFullScalingFactor == fScalingFactor_) return;

    fBaseScalingFactor_ = fScalingFactor;
    fScalingFactor_ = fFullScalingFactor;

    pInputManager_->set_interface_scaling_factor(fScalingFactor_);
    pRoot_->notify_scaling_factor_updated();

    notify_object_moved();
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
        get_lua(), get_localizer(), get_event_manager(), get_root(), get_virtual_root());

    for (const auto& sDirectory : lGUIDirectoryList_)
        pAddOnRegistry_->load_addon_directory(sDirectory);
}

void manager::load_ui()
{
    if (bLoaded_)
        return;

    pFactory_ = std::make_unique<factory>(*this);
    pRoot_ = utils::make_owned<uiroot>(*this);
    pVirtualRoot_ = utils::make_owned<virtual_uiroot>(*this, pRoot_->get_registry());

    pInputManager_->register_event_manager(
        utils::observer_ptr<event_manager>(observer_from_this(), static_cast<event_manager*>(this)));

    register_event("KEY_PRESSED");
    register_event("MOUSE_MOVED");
    register_event("WINDOW_RESIZED");
    pRoot_->register_event("WINDOW_RESIZED");

    create_lua_();

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

    pLua_ = nullptr;

    pHoveredFrame_ = nullptr;
    bUpdateHoveredFrame_ = false;
    pFocusedFrame_ = nullptr;
    pMovedObject_ = nullptr;
    pSizedObject_ = nullptr;
    pMovedAnchor_ = nullptr;
    bObjectMoved_ = false;
    mMouseMovement_ = vector2f::ZERO;
    mMovementStartPosition_ = vector2f::ZERO;
    mConstraint_ = constraint::NONE;
    mResizeStart_ = vector2f::ZERO;
    bResizeWidth_ = false;
    bResizeHeight_ = false;
    bResizeFromRight_ = false;
    bResizeFromBottom_ = false;

    lKeyBindingList_.clear();

    pLocalizer_->clear_translations();

    unregister_event("KEY_PRESSED");
    unregister_event("MOUSE_MOVED");
    unregister_event("WINDOW_RESIZED");

    pInputManager_->unregister_event_manager(*this);

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

void manager::begin(std::shared_ptr<render_target> pTarget) const
{
    pRenderer_->begin(pTarget);

    vector2f mView;
    if (pTarget)
        mView = vector2f(pTarget->get_canvas_dimensions())/fScalingFactor_;
    else
        mView = pRoot_->get_target_dimensions();

    pRenderer_->set_view(matrix4f::view(mView));
}

void manager::end() const
{
    pRenderer_->end();
}

void manager::render_ui() const
{
    begin();

    pRoot_->render();

    end();
}

bool manager::is_loaded() const
{
    return bLoaded_;
}

void manager::update_ui(float fDelta)
{
    bUpdating_ = true;

    DEBUG_LOG(" Input...");
    pInputManager_->update(fDelta);

    DEBUG_LOG(" Update widgets...");
    pRoot_->update(fDelta);

    if (bObjectMoved_ || pInputManager_->get_mouse_delta() != vector2f::ZERO)
        bUpdateHoveredFrame_ = true;

    update_hovered_frame_();

    bObjectMoved_ = false;

    if (bFirstIteration_)
    {
        DEBUG_LOG(" Entering world...");
        fire_event(event("ENTERING_WORLD"));
        bFirstIteration_ = false;
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
    pInputManager_->allow_input("WORLD");
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
            pInputManager_->allow_input("WORLD");
        else
            pInputManager_->block_input("WORLD");
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

const vector2f& manager::get_movement() const
{
    return mMouseMovement_;
}

void manager::notify_object_moved()
{
    bObjectMoved_ = true;
}

void manager::enable_input(bool bEnable)
{
    if (bInputEnabled_ != bEnable)
        toggle_input();
}

void manager::toggle_input()
{
    bInputEnabled_ = !bInputEnabled_;

    if (bInputEnabled_)
    {
        bUpdateHoveredFrame_ = true;

        if (pFocusedFrame_)
            pInputManager_->set_keyboard_focus(true, pFocusedFrame_);
    }
    else
    {
        set_hovered_frame_(nullptr);

        if (pFocusedFrame_)
            pInputManager_->set_keyboard_focus(false);
    }
}

bool manager::is_input_enabled() const
{
    return bInputEnabled_;
}

void manager::update_hovered_frame_()
{
    if (!bUpdateHoveredFrame_ || !bInputEnabled_)
        return;

    DEBUG_LOG(" Update hovered frame...");
    const auto mMousePos = pInputManager_->get_mouse_position();

    utils::observer_ptr<frame> pHoveredFrame = pRoot_->find_hovered_frame(mMousePos);
    set_hovered_frame_(std::move(pHoveredFrame), mMousePos);

    bUpdateHoveredFrame_ = false;
}

const utils::observer_ptr<frame>& manager::get_hovered_frame()
{
    update_hovered_frame_();
    return pHoveredFrame_;
}

void manager::notify_hovered_frame_dirty()
{
    bUpdateHoveredFrame_ = true;
}

void manager::request_focus(utils::observer_ptr<focus_frame> pFocusFrame)
{
    if (pFocusFrame == pFocusedFrame_)
        return;

    if (pFocusFrame)
    {
        pFocusedFrame_ = std::move(pFocusFrame);
        pInputManager_->set_keyboard_focus(true, pFocusedFrame_);
    }
    else
    {
        pFocusedFrame_ = nullptr;
        pInputManager_->set_keyboard_focus(false);
    }
}

void manager::set_key_binding(input::key mKey, const std::string& sLuaString)
{
    lKeyBindingList_[mKey][input::key::K_UNASSIGNED][input::key::K_UNASSIGNED] = sLuaString;
}

void manager::set_key_binding(input::key mKey, input::key mModifier, const std::string& sLuaString)
{
    lKeyBindingList_[mKey][mModifier][input::key::K_UNASSIGNED] = sLuaString;
}

void manager::set_key_binding(input::key mKey, input::key mModifier1, input::key mModifier2, const std::string& sLuaString)
{
    lKeyBindingList_[mKey][mModifier1][mModifier2] = sLuaString;
}

void manager::remove_key_binding(input::key mKey, input::key mModifier1, input::key mModifier2)
{
    auto iter1 = lKeyBindingList_.find(mKey);
    if (iter1 != lKeyBindingList_.end())
    {
        auto iter2 = iter1->second.find(mModifier1);
        if (iter2 != iter1->second.end())
        {
            auto iter3 = iter2->second.find(mModifier2);
            if (iter3 != iter2->second.end())
            {
                iter2->second.erase(iter3);

                if (iter2->second.empty())
                    iter1->second.erase(iter2);

                if (iter1->second.empty())
                    lKeyBindingList_.erase(iter1);
            }
        }
    }
}

void manager::on_event(const event& mEvent)
{
    if (mEvent.get_name() == "KEY_PRESSED")
    {
        const input::key mKey = mEvent.get<input::key>(0);

        std::string sScript;
        std::string sKeyName;

        auto iter1 = lKeyBindingList_.find(mKey);
        if (iter1 != lKeyBindingList_.end())
        {
            for (const auto& iter2 : iter1->second)
            {
                if (iter2.first == input::key::K_UNASSIGNED ||
                    !pInputManager_->key_is_down(iter2.first))
                    continue;

                // First try to get a match with the most complicated binding with two modifiers
                for (const auto& iter3 : iter2.second)
                {
                    if (iter3.first == input::key::K_UNASSIGNED ||
                        !pInputManager_->key_is_down(iter3.first))
                        continue;

                    sScript = iter3.second;
                    sKeyName = pInputManager_->get_key_name(mKey, iter2.first, iter3.first);
                    break;
                }

                if (!sScript.empty())
                    break;

                // If none was found, try with only one modifier
                auto iter3 = iter2.second.find(input::key::K_UNASSIGNED);
                if (iter3 != iter2.second.end())
                {
                    sScript = iter3->second;
                    sKeyName = pInputManager_->get_key_name(mKey, iter2.first);
                }
            }

            if (sScript.empty())
            {
                // If no modifier was matching, try with no modifier
                auto iter2 = iter1->second.find(input::key::K_UNASSIGNED);
                if (iter2 != iter1->second.end())
                {
                    auto iter3 = iter2->second.find(input::key::K_UNASSIGNED);
                    if (iter3 != iter2->second.end())
                    {
                        sScript = iter3->second;
                        sKeyName = pInputManager_->get_key_name(mKey);
                    }
                }
            }
        }

        if (!sScript.empty())
        {
            try
            {
                pLua_->do_string(sScript);
            }
            catch (const sol::error& e)
            {
                gui::out << gui::error << "Bound action : " << sKeyName
                    << " : " << e.what() << std::endl;
            }
        }
    }
    else if (mEvent.get_name() == "WINDOW_RESIZED")
    {
        // Update the scaling factor
        set_interface_scaling_factor(fBaseScalingFactor_);

        notify_object_moved();

        pRenderer_->notify_window_resized(vector2ui(
            mEvent.get<std::uint32_t>(0), mEvent.get<std::uint32_t>(1)));
    }
    else if (mEvent.get_name() == "MOUSE_MOVED")
    {
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
