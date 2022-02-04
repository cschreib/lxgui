#ifndef LXGUI_GUI_PARSER_COMMON_HPP
#define LXGUI_GUI_PARSER_COMMON_HPP

#include "lxgui/gui_region_attributes.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_observer.hpp"

namespace lxgui::gui {

class frame;
class registry;
class virtual_registry;
class layout_node;

/// Parse "core" attributes from a layout node, before creating a frame.
/** \param mRegistry        The UI object registry, for parent lookup
 *   \param mVirtualRegistry The virtual UI object registry, for inheritance lookup
 *   \param mNode            The layout node to parse from
 *   \param pParent          The current layout parent frame of this node (nullptr if none)
 *   \return Filled-in core attributes structure.
 */
region_core_attributes parse_core_attributes(
    registry&                  mRegistry,
    virtual_registry&          mVirtualRegistry,
    const layout_node&         mNode,
    utils::observer_ptr<frame> pParent);

/// Emit a warning if this node (or any of its attributes/children) was not read.
/** \param mNode The node to check
 */
void warn_for_not_accessed_node(const layout_node& mNode);

} // namespace lxgui::gui

#endif
