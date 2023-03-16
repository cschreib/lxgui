#include "lxgui/gui_virtual_root.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"

namespace lxgui::gui {

virtual_root::virtual_root(manager& mgr, registry& non_virtual_registry) :
    frame_container(mgr.get_factory(), object_registry_, nullptr),
    manager_(mgr),
    object_registry_(non_virtual_registry) {}

virtual_root::~virtual_root() {
    // Must be done before we destroy the registry
    clear_frames_();
}

} // namespace lxgui::gui
