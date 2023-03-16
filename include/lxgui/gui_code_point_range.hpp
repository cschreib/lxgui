#ifndef LXGUI_GUI_CODE_POINT_RANGE_HPP
#define LXGUI_GUI_CODE_POINT_RANGE_HPP

#include "lxgui/lxgui.hpp"

namespace lxgui::gui {

/// Represents a contiguous range of unicode code points
struct code_point_range {
    char32_t first = 0u;
    char32_t last  = 0u;
};

} // namespace lxgui::gui

#endif
