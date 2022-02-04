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
    PARENT     = -1,
    BACKGROUND = 0,
    LOW,
    MEDIUM,
    HIGH,
    DIALOG,
    FULLSCREEN,
    FULLSCREEN_DIALOG,
    TOOLTIP
};

struct strata;

/// Contains gui::frame
struct level {
    std::vector<utils::observer_ptr<frame>> lFrameList;
    strata*                                 pStrata = nullptr;
};

/// Contains gui::level
struct strata {
    std::map<int, level>           lLevelList;
    bool                           bRedraw = true;
    std::shared_ptr<render_target> pRenderTarget;
    quad                           mQuad;
};

} // namespace lxgui::gui

#endif
