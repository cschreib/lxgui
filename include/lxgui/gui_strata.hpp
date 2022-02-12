#ifndef LXGUI_GUI_STRATA_HPP
#define LXGUI_GUI_STRATA_HPP

#include "lxgui/gui_quad.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/utils_observer.hpp"

#include <map>
#include <memory>
#include <vector>

namespace lxgui::gui {

class frame;

enum class frame_strata {
    parent     = -1,
    background = 0,
    low,
    medium,
    high,
    dialog,
    fullscreen,
    fullscreen_dialog,
    tooltip
};

struct strata;

/// Contains gui::frame
struct level {
    std::vector<utils::observer_ptr<frame>> l_frame_list;
    strata*                                 p_strata = nullptr;
};

/// Contains gui::level
struct strata {
    std::map<int, level>           l_level_list;
    bool                           b_redraw = true;
    std::shared_ptr<render_target> p_render_target;
    quad                           m_quad;
};

} // namespace lxgui::gui

#endif
