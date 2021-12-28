#ifndef LXGUI_GUI_PARSER_COMMON_HPP
#define LXGUI_GUI_PARSER_COMMON_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/gui_uiobject_attributes.hpp"

#include <lxgui/utils_observer.hpp>

namespace lxgui {
namespace gui
{
    class frame;
    class manager;
    class layout_node;

    /// Parse "core" attributes from a layout node, before creating a frame.
    /** \param mManager The GUI manager doing the parsing
    *   \param mNode    The layout node to parse from
    *   \param pParent  The current layout parent frame of this node (nullptr if none)
    *   \return Filled in core attributes structure.
    */
    uiobject_core_attributes parse_core_attributes(manager& mManager, const layout_node& mNode,
        utils::observer_ptr<frame> pParent);

    /// Emit a warning if this node (or any of its attributes/children) was not read.
    /** \param mNode The node to check
    */
    void warn_for_not_accessed_node(const layout_node& mNode);
}
}


#endif
