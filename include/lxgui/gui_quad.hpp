#ifndef LXGUI_GUI_QUAD_HPP
#define LXGUI_GUI_QUAD_HPP

#include "lxgui/gui_material.hpp"
#include "lxgui/gui_vertex.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

#include <array>
#include <memory>

namespace lxgui::gui {

/// Specifies the rendering mode of a quad
enum class blend_mode { normal, add, mul };

/// Simple structure holding four vertices and a material
struct quad {
    std::array<vertex, 4>     v;
    std::shared_ptr<material> mat;
    blend_mode                blend = blend_mode::normal;
};

} // namespace lxgui::gui

#endif
