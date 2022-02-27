#ifndef LXGUI_GUI_GRADIENT_HPP
#define LXGUI_GUI_GRADIENT_HPP

#include "lxgui/gui_color.hpp"
#include "lxgui/gui_orientation.hpp"
#include "lxgui/lxgui.hpp"

namespace lxgui::gui {

/// Represents color gradients
struct gradient {
    orientation orient = orientation::horizontal;
    color       min_color, max_color;
};

} // namespace lxgui::gui

#endif
