#include "lxgui/gui_factory.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_frame_renderer.hpp"
#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_registry.hpp"

namespace lxgui::gui {

factory::factory(manager& mgr) : manager_(mgr) {}

utils::owner_ptr<region> factory::create_region(registry& reg, const region_core_attributes& attr) {
    if (!reg.check_region_name(attr.name))
        return nullptr;

    auto iter = custom_object_list_.find(attr.object_type);
    if (iter == custom_object_list_.end()) {
        gui::out << gui::warning << "gui::factory: Unknown object class: \"" << attr.object_type
                 << "\"." << std::endl;
        return nullptr;
    }

    utils::owner_ptr<region> new_object = iter->second(manager_, attr);
    if (!new_object)
        return nullptr;

    if (!finalize_object_(reg, *new_object, attr))
        return nullptr;

    return new_object;
}

utils::owner_ptr<frame> factory::create_frame(registry& reg, const frame_core_attributes& attr) {
    if (!reg.check_region_name(attr.name))
        return nullptr;

    auto iter = custom_frame_list_.find(attr.object_type);
    if (iter == custom_frame_list_.end()) {
        gui::out << gui::warning << "gui::factory: Unknown frame class: \"" << attr.object_type
                 << "\"." << std::endl;
        return nullptr;
    }

    utils::owner_ptr<frame> new_frame = iter->second(manager_, attr);
    if (!new_frame)
        return nullptr;

    if (!finalize_object_(reg, *new_frame, attr))
        return nullptr;

    return new_frame;
}

utils::owner_ptr<layered_region>
factory::create_layered_region(registry& reg, const region_core_attributes& attr) {
    if (!reg.check_region_name(attr.name))
        return nullptr;

    auto iter = custom_region_list_.find(attr.object_type);
    if (iter == custom_region_list_.end()) {
        gui::out << gui::warning << "gui::factory: Unknown layered_region class: \""
                 << attr.object_type << "\"." << std::endl;
        return nullptr;
    }

    utils::owner_ptr<layered_region> new_region = iter->second(manager_, attr);
    if (!new_region)
        return nullptr;

    if (!finalize_object_(reg, *new_region, attr))
        return nullptr;

    return new_region;
}

void factory::register_on_lua(sol::state& lua) {
    for (const auto& reg : custom_lua_regs_)
        reg.second(lua);
}

bool factory::finalize_object_(registry& reg, region& object, const region_core_attributes& attr) {
    if (!object.is_virtual() || attr.parent == nullptr) {
        if (!reg.add_region(observer_from(&object)))
            return false;
    }

    // NB: This needs to be done after the region is fully constructed, since this
    // should override defaults set in any constructor.
    apply_inheritance_(object, attr);

    return true;
}

void factory::apply_inheritance_(region& object, const region_core_attributes& attr) {
    for (const auto& base : attr.inheritance) {
        if (!object.is_region_type(*base)) {
            gui::out << gui::warning << "gui::factory: "
                     << "\"" << object.get_name() << "\" (" << object.get_region_type()
                     << ") cannot inherit from \"" << base->get_name() << "\" ("
                     << base->get_region_type() << "). Inheritance skipped." << std::endl;
            continue;
        }

        // Inherit from the other object
        object.copy_from(*base);
    }
}

} // namespace lxgui::gui
