#include "lxgui/gui_manager.hpp"

#include "lxgui/gui_addon_registry.hpp"
#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_factory.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_keybinder.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_root.hpp"
#include "lxgui/gui_virtual_root.hpp"
#include "lxgui/input_dispatcher.hpp"
#include "lxgui/input_source.hpp"
#include "lxgui/input_window.hpp"
#include "lxgui/input_world_dispatcher.hpp"
#include "lxgui/utils_std.hpp"

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

namespace lxgui::gui {

manager::manager(
    utils::control_block&          block,
    std::unique_ptr<input::source> src,
    std::unique_ptr<renderer>      rdr) :
    utils::enable_observer_from_this<manager>(block),
    input_source_(std::move(src)),
    renderer_(std::move(rdr)),
    window_(std::make_unique<input::window>(*input_source_)),
    input_dispatcher_(std::make_unique<input::dispatcher>(*input_source_)),
    world_input_dispatcher_(std::make_unique<input::world_dispatcher>()),
    event_emitter_(std::make_unique<gui::event_emitter>()),
    factory_(std::make_unique<factory>(*this)),
    localizer_(std::make_unique<localizer>()) {
    set_interface_scaling_factor(1.0f);

    // Register base types
    factory_->register_region_type<region>();
    factory_->register_region_type<frame>();
    factory_->register_region_type<layered_region>();

    window_->on_window_resized.connect([&](const vector2ui& dimensions) {
        // Update the scaling factor; on mobile platforms, rotating the screen will
        // trigger a change of window size and resolution, which the scaling factor "hint"
        // will pick up.
        set_interface_scaling_factor(base_scaling_factor_);

        renderer_->notify_window_resized(dimensions);
    });
}

manager::~manager() {
    close_ui_now();
}

void manager::set_interface_scaling_factor(float scaling_factor) {
    float full_scaling_factor = scaling_factor * window_->get_interface_scaling_factor_hint();

    if (full_scaling_factor == scaling_factor_)
        return;

    base_scaling_factor_ = scaling_factor;
    scaling_factor_      = full_scaling_factor;

    input_dispatcher_->set_interface_scaling_factor(scaling_factor_);

    if (root_) {
        root_->notify_scaling_factor_updated();
        root_->notify_hovered_frame_dirty();
    }
}

float manager::get_interface_scaling_factor() const {
    return scaling_factor_;
}

void manager::enable_caching(bool enable_caching) {
    enable_caching_ = enable_caching;
    if (root_)
        root_->enable_caching(enable_caching_);
}

void manager::toggle_caching() {
    enable_caching_ = !enable_caching_;
    if (root_)
        root_->enable_caching(enable_caching_);
}

bool manager::is_caching_enabled() const {
    if (root_)
        return root_->is_caching_enabled();
    else
        return enable_caching_;
}

void manager::add_addon_directory(const std::string& directory) {
    if (utils::find(gui_directory_list_, directory) == gui_directory_list_.end())
        gui_directory_list_.push_back(directory);
}

void manager::clear_addon_directory_list() {
    gui_directory_list_.clear();
}

sol::state& manager::get_lua() {
    return *lua_;
}

const sol::state& manager::get_lua() const {
    return *lua_;
}

void manager::read_files_() {
    if (is_loaded_ || addon_registry_)
        return;

    addon_registry_ = std::make_unique<addon_registry>(
        get_lua(), get_localizer(), get_event_emitter(), get_root(), get_virtual_root());

    for (const auto& directory : gui_directory_list_)
        addon_registry_->load_addon_directory(directory);
}

void manager::load_ui() {
    if (is_loaded_)
        return;

    root_         = utils::make_owned<root>(*this);
    virtual_root_ = utils::make_owned<virtual_root>(*this, get_root().get_registry());

    create_lua_();
    read_files_();

    is_loaded_     = true;
    close_ui_flag_ = false;
}

void manager::close_ui() {
    if (is_updating_)
        close_ui_flag_ = true;
    else
        close_ui_now();
}

void manager::close_ui_now() {
    if (!is_loaded_)
        return;

    if (addon_registry_)
        addon_registry_->save_variables();

    virtual_root_   = nullptr;
    root_           = nullptr;
    addon_registry_ = nullptr;
    lua_            = nullptr;

    localizer_->clear_translations();

    is_loaded_          = false;
    is_first_iteration_ = true;
}

void manager::reload_ui() {
    if (is_updating_)
        reload_ui_flag_ = true;
    else
        reload_ui_now();
}

void manager::reload_ui_now() {
    gui::out << "Closing UI..." << std::endl;
    close_ui_now();
    gui::out << "Done. Loading UI..." << std::endl;
    load_ui();
    gui::out << "Done." << std::endl;

    reload_ui_flag_ = false;
}

void manager::render_ui() const {
    renderer_->begin();

    root_->render();

    renderer_->end();
}

bool manager::is_loaded() const {
    return is_loaded_;
}

void manager::update_ui(float delta) {
    is_updating_ = true;

    DEBUG_LOG(" Update regions...");
    root_->update(delta);

    if (is_first_iteration_) {
        DEBUG_LOG(" Entering world...");
        get_event_emitter().fire_event("ENTERING_WORLD");
        is_first_iteration_ = false;

        root_->notify_hovered_frame_dirty();
    }

    is_updating_ = false;

    if (reload_ui_flag_)
        reload_ui_now();
    if (close_ui_flag_)
        close_ui_now();
}

std::string manager::print_ui() const {
    std::stringstream s;

    s << "\n\n######################## Regions "
         "########################\n\n########################\n"
      << std::endl;
    for (const auto& frame : root_->get_root_frames()) {
        s << frame.serialize("") << "\n########################\n" << std::endl;
    }

    s << "\n\n#################### Virtual Regions "
         "####################\n\n########################\n"
      << std::endl;
    for (const auto& frame : virtual_root_->get_root_frames()) {
        s << frame.serialize("") << "\n########################\n" << std::endl;
    }

    return s.str();
}

} // namespace lxgui::gui
