#ifndef LXGUI_GUI_PARSER_COMMON_HPP
#define LXGUI_GUI_PARSER_COMMON_HPP

#include "lxgui/gui_region_core_attributes.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_observer.hpp"

namespace lxgui::gui {

class frame;
class registry;
class virtual_registry;
class layout_node;

/**
 * \brief Parse "core" attributes from a layout node, before creating a frame.
 * \param reg The UI object registry, for parent lookup
 * \param vreg The virtual UI object registry, for inheritance lookup
 * \param node The layout node to parse from
 * \param parent The current layout parent frame of this node (nullptr if none)
 * \return Filled-in core attributes structure.
 */
region_core_attributes parse_core_attributes(
    registry&                  reg,
    virtual_registry&          vreg,
    const layout_node&         node,
    utils::observer_ptr<frame> parent);

/**
 * \brief Emit a warning if this node (or any of its attributes/children) was not read.
 * \param node The node to check
 */
void warn_for_not_accessed_node(const layout_node& node);

} // namespace lxgui::gui

#endif
