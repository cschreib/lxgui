#ifndef LXGUI_GUI_VERTEX_HPP
#define LXGUI_GUI_VERTEX_HPP

#include "lxgui/gui_color.hpp"
#include "lxgui/gui_vector2.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

namespace lxgui { namespace gui {

struct vertex {
    vector2f pos;
    vector2f uvs;
    color    col;
};

}} // namespace lxgui::gui

#endif
