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
    registry&                  reg,
    virtual_registry&          vreg,
    const layout_node&         node,
    utils::observer_ptr<frame> parent) {
    region_core_attributes attr;
    attr.object_type = node.get_name();
    attr.name        = node.get_attribute_value<std::string>("name");

    if (parent) {
        attr.parent = std::move(parent);

        if (node.has_attribute("virtual")) {
            gui::out << gui::warning << node.get_location() << " : "
                     << "Cannot use the \"virtual\" attribute on \"" << attr.name
                     << "\", "
                        "because it is a nested region. Attribute ignored."
                     << std::endl;
        }
        if (node.has_attribute("parent")) {
            gui::out << gui::warning << node.get_location() << " : "
                     << "Cannot use the \"parent\" attribute on \"" << attr.name
                     << "\", "
                        "because it is a nested region. Attribute ignored."
                     << std::endl;
        }
    } else {
        attr.is_virtual = node.get_attribute_value_or<bool>("virtual", false);

        if (const layout_attribute* parent_attr = node.try_get_attribute("parent")) {
            std::string parent_name = parent_attr->get_value<std::string>();
            auto        parent_obj  = reg.get_region_by_name(parent_name);
            if (!parent_name.empty() && !parent_obj) {
                gui::out << gui::warning << node.get_location() << " : "
                         << "Cannot find \"" << attr.name << "\"'s parent : \"" << parent_name
                         << "\". "
                            "No parent given to this region."
                         << std::endl;
            }

            attr.parent = down_cast<frame>(parent_obj);
            if (parent_obj != nullptr && attr.parent == nullptr) {
                gui::out << gui::warning << node.get_location() << " : "
                         << "Cannot set  \"" << attr.name << "\"'s parent : \"" << parent_name
                         << "\". "
                            "This is not a frame."
                         << std::endl;
            }
        }
    }

    if (const layout_attribute* inh_attr = node.try_get_attribute("inherits")) {
        attr.inheritance = vreg.get_virtual_region_list(inh_attr->get_value<std::string>());
    }

    return attr;
}

void warn_for_not_accessed_node(const layout_node& node) {
    if (node.is_access_check_bypassed())
        return;

    if (!node.was_accessed()) {
        gui::out << gui::warning << node.get_location() << " : "
                 << "node '" << node.get_name()
                 << "' was not read by parser; "
                    "check its name is spelled correctly and that it is at the right location."
                 << std::endl;
        return;
    }

    for (const auto& attr : node.get_attributes()) {
        if (attr.is_access_check_bypassed())
            continue;

        if (!attr.was_accessed()) {
            gui::out << gui::warning << node.get_location() << " : "
                     << "attribute '" << node.get_name()
                     << "' was not read by parser; "
                        "check its name is spelled correctly and that it is at the right location."
                     << std::endl;
        }
    }

    for (const auto& child : node.get_children())
        warn_for_not_accessed_node(child);
}

} // namespace lxgui::gui
