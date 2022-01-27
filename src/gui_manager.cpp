#include "lxgui/gui_manager.hpp"

#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/gui_frame.hpp"
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
#include "lxgui/input_source.hpp"
#include "lxgui/input_dispatcher.hpp"
#include "lxgui/input_world_dispatcher.hpp"
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
    utils::enable_observer_from_this<manager>(mBlock),
    pInputSource_(std::move(pInputSource)),
    pRenderer_(std::move(pRenderer)),
    pWindow_(std::make_unique<input::window>(*pInputSource_)),
    pInputDispatcher_(std::make_unique<input::dispatcher>(*pInputSource_)),
    pWorldInputDispatcher_(std::make_unique<input::world_dispatcher>()),
    pEventEmitter_(std::make_unique<gui::event_emitter>()),
    pLocalizer_(std::make_unique<localizer>())
{
    set_interface_scaling_factor(1.0f);

    pWindow_->on_window_resized.connect([&](const vector2ui& mDimensions)
    {
        // Update the scaling factor; on mobile platforms, rotating the screen will
        // trigger a change of window size and resolution, which the scaling factor "hint"
        // will pick up.
        set_interface_scaling_factor(fBaseScalingFactor_);

        pRenderer_->notify_window_resized(mDimensions);
    });
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

    pInputDispatcher_->set_interface_scaling_factor(fScalingFactor_);

    pRoot_->notify_scaling_factor_updated();
    pRoot_->notify_hovered_frame_dirty();
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

    pLocalizer_->clear_translations();

    bLoaded_ = false;
    bFirstIteration_ = true;
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

    DEBUG_LOG(" Update regions...");
    pRoot_->update(fDelta);

    if (bFirstIteration_)
    {
        DEBUG_LOG(" Entering world...");
        get_event_emitter().fire_event("ENTERING_WORLD");
        bFirstIteration_ = false;

        pRoot_->notify_hovered_frame_dirty();
    }

    bUpdating_ = false;

    if (bReloadUI_)
        reload_ui_now();
    if (bCloseUI_)
        close_ui_now();
}

std::string manager::print_ui() const
{
    std::stringstream s;

    s << "\n\n######################## Regions ########################\n\n########################\n" << std::endl;
    for (const auto& mFrame : pRoot_->get_root_frames())
    {
        s << mFrame.serialize("") << "\n########################\n" << std::endl;
    }

    s << "\n\n#################### Virtual Regions ####################\n\n########################\n" << std::endl;
    for (const auto& mFrame : pVirtualRoot_->get_root_frames())
    {
        s << mFrame.serialize("") << "\n########################\n" << std::endl;
    }

    return s.str();
}

}
}
