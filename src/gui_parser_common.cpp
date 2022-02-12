#include "lxgui/gui_parser_common.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_root.hpp"
#include "lxgui/gui_virtual_registry.hpp"
#include "lxgui/gui_virtual_root.hpp"

namespace lxgui::gui {

region_core_attributes parse_core_attributes(
    registry&                  m_registry,
    virtual_registry&          m_virtual_registry,
    const layout_node&         m_node,
    utils::observer_ptr<frame> p_parent) {
    region_core_attributes m_attr;
    m_attr.s_object_type = m_node.get_name();
    m_attr.s_name       = m_node.get_attribute_value<std::string>("name");

    if (p_parent) {
        m_attr.p_parent = std::move(p_parent);

        if (m_node.has_attribute("virtual")) {
            gui::out << gui::warning << m_node.get_location() << " : "
                     << "Cannot use the \"virtual\" attribute on \"" << m_attr.s_name
                     << "\", "
                        "because it is a nested region. Attribute ignored."
                     << std::endl;
        }
        if (m_node.has_attribute("parent")) {
            gui::out << gui::warning << m_node.get_location() << " : "
                     << "Cannot use the \"parent\" attribute on \"" << m_attr.s_name
                     << "\", "
                        "because it is a nested region. Attribute ignored."
                     << std::endl;
        }
    } else {
        m_attr.b_virtual = m_node.get_attribute_value_or<bool>("virtual", false);

        if (const layout_attribute* p_attr = m_node.try_get_attribute("parent")) {
            std::string s_parent    = p_attr->get_value<std::string>();
            auto        p_parent_obj = m_registry.get_region_by_name(s_parent);
            if (!s_parent.empty() && !p_parent_obj) {
                gui::out << gui::warning << m_node.get_location() << " : "
                         << "Cannot find \"" << m_attr.s_name << "\"'s parent : \"" << s_parent
                         << "\". "
                            "No parent given to this region."
                         << std::endl;
            }

            m_attr.p_parent = down_cast<frame>(p_parent_obj);
            if (p_parent_obj != nullptr && m_attr.p_parent == nullptr) {
                gui::out << gui::warning << m_node.get_location() << " : "
                         << "Cannot set  \"" << m_attr.s_name << "\"'s parent : \"" << s_parent
                         << "\". "
                            "This is not a frame."
                         << std::endl;
            }
        }
    }

    if (const layout_attribute* p_attr = m_node.try_get_attribute("inherits")) {
        m_attr.l_inheritance =
            m_virtual_registry.get_virtual_region_list(p_attr->get_value<std::string>());
    }

    return m_attr;
}

void warn_for_not_accessed_node(const layout_node& m_node) {
    if (m_node.is_access_check_bypassed())
        return;

    if (!m_node.was_accessed()) {
        gui::out << gui::warning << m_node.get_location() << " : "
                 << "node '" << m_node.get_name()
                 << "' was not read by parser; "
                    "check its name is spelled correctly and that it is at the right location."
                 << std::endl;
        return;
    }

    for (const auto& m_attr : m_node.get_attributes()) {
        if (m_attr.is_access_check_bypassed())
            continue;

        if (!m_attr.was_accessed()) {
            gui::out << gui::warning << m_node.get_location() << " : "
                     << "attribute '" << m_node.get_name()
                     << "' was not read by parser; "
                        "check its name is spelled correctly and that it is at the right location."
                     << std::endl;
        }
    }

    for (const auto& m_child : m_node.get_children())
        warn_for_not_accessed_node(m_child);
}

} // namespace lxgui::gui
