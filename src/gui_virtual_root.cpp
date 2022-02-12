#include "lxgui/gui_virtual_root.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"

namespace lxgui::gui {

virtual_root::virtual_root(manager& m_manager, registry& m_non_virtual_registry) :
    frame_container(m_manager.get_factory(), m_object_registry_, nullptr),
    m_manager_(m_manager),
    m_object_registry_(m_non_virtual_registry) {}

virtual_root::~virtual_root() {
    // Must be done before we destroy the registry
    clear_frames_();
}

} // namespace lxgui::gui
