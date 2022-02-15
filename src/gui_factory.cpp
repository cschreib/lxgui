#include "lxgui/gui_factory.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_framerenderer.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_registry.hpp"

namespace lxgui::gui {

factory::factory(manager& m_manager) : m_manager_(m_manager) {}

utils::owner_ptr<region>
factory::create_region(registry& m_registry, const region_core_attributes& m_attr) {
    if (!m_registry.check_region_name(m_attr.name))
        return nullptr;

    auto m_iter = custom_object_list_.find(m_attr.object_type);
    if (m_iter == custom_object_list_.end()) {
        gui::out << gui::warning << "gui::factory : Unknown object class : \"" << m_attr.object_type
                 << "\"." << std::endl;
        return nullptr;
    }

    utils::owner_ptr<region> p_new_object = m_iter->second(m_manager_);
    if (!p_new_object)
        return nullptr;

    if (!finalize_object_(m_registry, *p_new_object, m_attr))
        return nullptr;

    apply_inheritance_(*p_new_object, m_attr);

    return p_new_object;
}

utils::owner_ptr<frame> factory::create_frame(
    registry& m_registry, frame_renderer* p_renderer, const region_core_attributes& m_attr) {
    if (!m_registry.check_region_name(m_attr.name))
        return nullptr;

    auto m_iter = custom_frame_list_.find(m_attr.object_type);
    if (m_iter == custom_frame_list_.end()) {
        gui::out << gui::warning << "gui::factory : Unknown frame class : \"" << m_attr.object_type
                 << "\"." << std::endl;
        return nullptr;
    }

    utils::owner_ptr<frame> p_new_frame = m_iter->second(m_manager_);
    if (!p_new_frame)
        return nullptr;

    if (!finalize_object_(m_registry, *p_new_frame, m_attr))
        return nullptr;

    if (p_renderer && !p_new_frame->is_virtual())
        p_renderer->notify_rendered_frame(p_new_frame, true);

    apply_inheritance_(*p_new_frame, m_attr);

    return p_new_frame;
}

utils::owner_ptr<layered_region>
factory::create_layered_region(registry& m_registry, const region_core_attributes& m_attr) {
    if (!m_registry.check_region_name(m_attr.name))
        return nullptr;

    auto m_iter = custom_region_list_.find(m_attr.object_type);
    if (m_iter == custom_region_list_.end()) {
        gui::out << gui::warning << "gui::factory : Unknown layered_region class : \""
                 << m_attr.object_type << "\"." << std::endl;
        return nullptr;
    }

    utils::owner_ptr<layered_region> p_new_region = m_iter->second(m_manager_);
    if (!p_new_region)
        return nullptr;

    if (!finalize_object_(m_registry, *p_new_region, m_attr))
        return nullptr;

    apply_inheritance_(*p_new_region, m_attr);

    return p_new_region;
}

sol::state& factory::get_lua() {
    return m_manager_.get_lua();
}

const sol::state& factory::get_lua() const {
    return m_manager_.get_lua();
}

bool factory::finalize_object_(
    registry& m_registry, region& m_object, const region_core_attributes& m_attr) {
    if (m_attr.b_virtual)
        m_object.set_virtual();

    if (m_attr.p_parent)
        m_object.set_name_and_parent_(m_attr.name, m_attr.p_parent);
    else
        m_object.set_name_(m_attr.name);

    if (!m_object.is_virtual() || m_attr.p_parent == nullptr) {
        if (!m_registry.add_region(observer_from(&m_object)))
            return false;
    }

    if (!m_object.is_virtual())
        m_object.create_glue();

    return true;
}

void factory::apply_inheritance_(region& m_object, const region_core_attributes& m_attr) {
    for (const auto& p_base : m_attr.inheritance) {
        if (!m_object.is_object_type(p_base->get_object_type())) {
            gui::out << gui::warning << "gui::factory : "
                     << "\"" << m_object.get_name() << "\" (" << m_object.get_object_type()
                     << ") cannot inherit from \"" << p_base->get_name() << "\" ("
                     << p_base->get_object_type() << "). Inheritance skipped." << std::endl;
            continue;
        }

        // Inherit from the other object
        m_object.copy_from(*p_base);
    }
}

} // namespace lxgui::gui
