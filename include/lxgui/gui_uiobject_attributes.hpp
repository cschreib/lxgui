#ifndef LXGUI_GUI_UIOBJECT_ATTRIBUTES_HPP
#define LXGUI_GUI_UIOBJECT_ATTRIBUTES_HPP

#include <lxgui/lxgui.hpp>

#include <lxgui/utils_observer.hpp>
#include <string>
#include <vector>

namespace lxgui {
namespace gui
{
    class frame;
    class uiobject;

    /// Struct holding all the core information about a uiobject necessary for its creation.
    struct uiobject_core_attributes
    {
        std::string sObjectType;
        std::string sName;
        bool        bVirtual = false;

        utils::observer_ptr<frame> pParent = nullptr;

        std::vector<utils::observer_ptr<const uiobject>> lInheritance;
    };
}
}

#endif
