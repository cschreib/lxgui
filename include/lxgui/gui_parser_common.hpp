#ifndef LXGUI_GUI_PARSER_COMMON_HPP
#define LXGUI_GUI_PARSER_COMMON_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils_observer.hpp>
#include <lxgui/utils_layout_node.hpp>
#include <lxgui/gui_frame.hpp>
#include <lxgui/gui_manager.hpp>

namespace lxgui {
namespace gui
{
    /// Struct holding core information about a frame, parsed from XML.
    struct node_core_attributes
    {
        std::string sObjectType;
        std::string sName;
        bool        bVirtual = false;

        utils::observer_ptr<frame> pParent = nullptr;

        std::vector<utils::observer_ptr<const uiobject>> lInheritance;
    };

    /// Parse "core" attributes from an XML block, before creating a frame.
    /** \param mManager The GUI manager doing the parsing
    *   \param mNode    The layout node to parse from
    *   \param pParent  The current layout parent frame of this block (nullptr if none)
    *   \return Filled in core attributes structure.
    */
    node_core_attributes parse_core_attributes(manager& mManager, const utils::layout_node& mNode,
        utils::observer_ptr<frame> pParent);
}
}


#endif
