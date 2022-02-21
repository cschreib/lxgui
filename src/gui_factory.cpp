#include "lxgui/gui_factory.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_framerenderer.hpp"
#include "lxgui/gui_layeredregion.hpp"
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

    utils::owner_ptr<region> new_object = iter->second(manager_);
    if (!new_object)
        return nullptr;

    if (!finalize_object_(reg, *new_object, attr))
        return nullptr;

    apply_inheritance_(*new_object, attr);

    return new_object;
}

utils::owner_ptr<frame>
factory::create_frame(registry& reg, frame_renderer* rdr, const region_core_attributes& attr) {
    if (!reg.check_region_name(attr.name))
        return nullptr;

    auto iter = custom_frame_list_.find(attr.object_type);
    if (iter == custom_frame_list_.end()) {
        gui::out << gui::warning << "gui::factory: Unknown frame class: \"" << attr.object_type
                 << "\"." << std::endl;
        return nullptr;
    }

    utils::owner_ptr<frame> new_frame = iter->second(manager_);
    if (!new_frame)
        return nullptr;

    if (!finalize_object_(reg, *new_frame, attr))
        return nullptr;

    if (rdr && !new_frame->is_virtual())
        rdr->notify_rendered_frame(new_frame, true);

    apply_inheritance_(*new_frame, attr);

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

    utils::owner_ptr<layered_region> new_region = iter->second(manager_);
    if (!new_region)
        return nullptr;

    if (!finalize_object_(reg, *new_region, attr))
        return nullptr;

    apply_inheritance_(*new_region, attr);

    return new_region;
}

sol::state& factory::get_lua() {
    return manager_.get_lua();
}

const sol::state& factory::get_lua() const {
    return manager_.get_lua();
}

bool factory::finalize_object_(registry& reg, region& object, const region_core_attributes& attr) {
    if (attr.is_virtual)
        object.set_virtual();

    if (attr.parent)
        object.set_name_and_parent_(attr.name, attr.parent);
    else
        object.set_name_(attr.name);

    if (!object.is_virtual() || attr.parent == nullptr) {
        if (!reg.add_region(observer_from(&object)))
            return false;
    }

    if (!object.is_virtual())
        object.create_glue();

    return true;
}

void factory::apply_inheritance_(region& object, const region_core_attributes& attr) {
    for (const auto& base : attr.inheritance) {
        if (!object.is_object_type(base->get_object_type())) {
            gui::out << gui::warning << "gui::factory: "
                     << "\"" << object.get_name() << "\" (" << object.get_object_type()
                     << ") cannot inherit from \"" << base->get_name() << "\" ("
                     << base->get_object_type() << "). Inheritance skipped." << std::endl;
            continue;
        }

        // Inherit from the other object
        object.copy_from(*base);
    }
}

} // namespace lxgui::gui
