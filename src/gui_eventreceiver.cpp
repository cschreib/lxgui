#include "lxgui/gui_eventreceiver.hpp"

#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/utils_std.hpp"

namespace lxgui::gui {

event_receiver::event_receiver(event_emitter& emitter) : event_emitter_(emitter) {}

void event_receiver::register_event(
    const std::string& event_name, event_handler_function callback) {
    utils::connection connection = event_emitter_.register_event(event_name, std::move(callback));
    registered_events_.push_back({event_name, std::move(connection)});
}

void event_receiver::unregister_event(const std::string& event_name) {
    auto iter = utils::find_if(
        registered_events_, [&](const auto& event) { return event.name == event_name; });

    if (iter == registered_events_.end()) {
        gui::out << gui::warning << "event_emitter: "
                 << "Event \"" << event_name << "\" is not registered to this event_receiver."
                 << std::endl;

        return;
    }

    registered_events_.erase(iter);
}

} // namespace lxgui::gui
