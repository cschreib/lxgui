#ifndef LXGUI_GUI_FRAME_CORE_ATTRIBUTES_HPP
#define LXGUI_GUI_FRAME_CORE_ATTRIBUTES_HPP

#include "lxgui/gui_region_core_attributes.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_observer.hpp"

#include <string>
#include <vector>

namespace lxgui::gui {

class frame_renderer;

/// Struct holding all the core information about a frame necessary for its creation.
struct frame_core_attributes : region_core_attributes {
    frame_core_attributes()                             = default;
    frame_core_attributes(const frame_core_attributes&) = default;
    frame_core_attributes(frame_core_attributes&&)      = default;
    frame_core_attributes& operator=(const frame_core_attributes&) = default;
    frame_core_attributes& operator=(frame_core_attributes&&) = default;

    explicit frame_core_attributes(const region_core_attributes& rattr) :
        region_core_attributes(rattr) {}

    utils::observer_ptr<frame_renderer> rdr = nullptr;
};

} // namespace lxgui::gui

#endif
