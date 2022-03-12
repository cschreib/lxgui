#ifndef LXGUI_GUI_STRATA_HPP
#define LXGUI_GUI_STRATA_HPP

#include "lxgui/gui_quad.hpp"
#include "lxgui/gui_render_target.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/utils_observer.hpp"

#include <map>
#include <memory>
#include <vector>

namespace lxgui::gui {

class frame;

enum class frame_strata {
    parent,
    background,
    low,
    medium,
    high,
    dialog,
    fullscreen,
    fullscreen_dialog,
    tooltip
};

/// Contains frames sorted by level
struct strata {
    frame_strata                        id;
    std::pair<std::size_t, std::size_t> range;
    bool                                redraw_flag = true;
    std::shared_ptr<render_target>      target;
    quad                                target_quad;
};

} // namespace lxgui::gui

#endif
