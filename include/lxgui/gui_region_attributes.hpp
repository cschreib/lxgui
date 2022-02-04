#ifndef LXGUI_GUI_REGION_ATTRIBUTES_HPP
#define LXGUI_GUI_REGION_ATTRIBUTES_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils_observer.hpp"

#include <string>
#include <vector>

namespace lxgui::gui {

class frame;
class region;

/// Struct holding all the core information about a region necessary for its creation.
struct region_core_attributes {
    std::string sObjectType;
    std::string sName;
    bool        bVirtual = false;

    utils::observer_ptr<frame> pParent = nullptr;

    std::vector<utils::observer_ptr<const region>> lInheritance;
};

} // namespace lxgui::gui

#endif
