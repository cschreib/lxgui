#ifndef LXGUI_GUI_PARSER_COMMON_HPP
#define LXGUI_GUI_PARSER_COMMON_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/gui_layoutnode.hpp"

#include <lxgui/utils_observer.hpp>
#include <lxgui/gui_frame.hpp>
#include <lxgui/gui_manager.hpp>

namespace lxgui {
namespace gui
{
    /// Struct holding core information about a frame, parsed from a layout file.
    struct node_core_attributes
    {
        std::string sObjectType;
        std::string sName;
        bool        bVirtual = false;

        utils::observer_ptr<frame> pParent = nullptr;

        std::vector<utils::observer_ptr<const uiobject>> lInheritance;
    };

    /// Parse "core" attributes from a layout node, before creating a frame.
    /** \param mManager The GUI manager doing the parsing
    *   \param mNode    The layout node to parse from
    *   \param pParent  The current layout parent frame of this node (nullptr if none)
    *   \return Filled in core attributes structure.
    */
    node_core_attributes parse_core_attributes(manager& mManager, const layout_node& mNode,
        utils::observer_ptr<frame> pParent);


    /// Emit a warning if this node (or any of its attributes/children) was not read.
    /** \param mNode The node to check
    */
    void warn_for_not_accessed_node(const layout_node& mNode);
}
}


#endif
