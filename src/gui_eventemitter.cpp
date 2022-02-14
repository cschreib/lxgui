#include "lxgui/gui_eventemitter.hpp"

#include "lxgui/gui_event.hpp"

namespace lxgui::gui {

utils::connection
event_emitter::register_event(const std::string& s_event_name, event_handler_function m_callback) {
    return registered_event_list_[s_event_name].connect(std::move(m_callback));
}

void event_emitter::fire_event(const std::string& s_event_name, event_data m_data) {
    registered_event_list_[s_event_name](std::move(m_data));
}

} // namespace lxgui::gui
