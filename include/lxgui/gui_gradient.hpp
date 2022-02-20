#ifndef LXGUI_GUI_GRADIENT_HPP
#define LXGUI_GUI_GRADIENT_HPP

#include "lxgui/gui_color.hpp"
#include "lxgui/lxgui.hpp"

namespace lxgui::gui {

/// Represents color gradients
struct gradient {
    enum class orientation { horizontal, vertical };

    orientation orient = orientation::horizontal;
    color       min_color, max_color;
};

} // namespace lxgui::gui

#endif
