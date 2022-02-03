#ifndef LXGUI_GUI_CODE_POINT_RANGE_HPP
#define LXGUI_GUI_CODE_POINT_RANGE_HPP

#include "lxgui/lxgui.hpp"

namespace lxgui {
namespace gui
{
    /// Represents a contiguous range of unicode code points
    struct code_point_range
    {
        char32_t uiFirst = 0u;
        char32_t uiLast = 0u;
    };
}
}

#endif
