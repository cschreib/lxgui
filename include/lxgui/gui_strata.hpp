#ifndef LXGUI_GUI_STRATA_HPP
#define LXGUI_GUI_STRATA_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/gui_sprite.hpp"

#include <lxgui/utils.hpp>

#include <vector>
#include <map>
#include <memory>

namespace lxgui {
namespace gui
{
    class frame;

    enum class frame_strata
    {
        PARENT = -1,
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
    struct level
    {
        std::vector<frame*> lFrameList;
        strata*             pStrata = nullptr;
    };

    /// Contains gui::level
    struct strata
    {
        std::map<int, level>           lLevelList;
        mutable bool                   bRedraw = true;
        std::shared_ptr<render_target> pRenderTarget;
        sprite                         mSprite;
        mutable uint                   uiRedrawCount = 0u;
    };
}
}

#endif
