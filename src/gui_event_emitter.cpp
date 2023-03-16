#include "lxgui/gui_event_emitter.hpp"

namespace lxgui::gui {

utils::connection
event_emitter::register_event(const std::string& event_name, event_handler_function callback) {
    return registered_event_list_[event_name].connect(std::move(callback));
}

void event_emitter::fire_event(const std::string& event_name, event_data data) {
    registered_event_list_[event_name](std::move(data));
}

} // namespace lxgui::gui
