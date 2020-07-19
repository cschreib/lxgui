#ifndef LXGUI_GUI_STRATA_HPP
#define LXGUI_GUI_STRATA_HPP

#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/gui_sprite.hpp"

#include <lxgui/utils.hpp>
#include <lxgui/utils_refptr.hpp>

#include <vector>
#include <map>

namespace lxgui {
namespace gui
{
    class frame;

    enum class frame_strata
    {
        PARENT,
        BACKGROUND,
        LOW,
        MEDIUM,
        HIGH,
        DIALOG,
        FULLSCREEN,
        FULLSCREEN_DIALOG,
        TOOLTIP
    };

    /// Contains frame
    struct level
    {
        std::vector<frame*> lFrameList;
    };

    /// Contains level
    struct strata
    {
        frame_strata                 mStrata = frame_strata::PARENT;
        std::map<int, level>         lLevelList;
        mutable bool                 bRedraw = true;
        utils::refptr<render_target> pRenderTarget;
        sprite                       mSprite;
        mutable uint                 uiRedrawCount = 0u;
    };
}
}

#endif
