#include "lxgui/gui_eventreceiver.hpp"

#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/utils_std.hpp"

namespace lxgui::gui {

event_receiver::event_receiver(event_emitter& m_emitter) : m_event_emitter_(m_emitter) {}

void event_receiver::register_event(
    const std::string& s_event_name, event_handler_function m_callback) {
    utils::connection m_connection =
        m_event_emitter_.register_event(s_event_name, std::move(m_callback));
    registered_events_.push_back({s_event_name, std::move(m_connection)});
}

void event_receiver::unregister_event(const std::string& s_event_name) {
    auto m_iter = utils::find_if(
        registered_events_, [&](const auto& m_event) { return m_event.s_name == s_event_name; });

    if (m_iter == registered_events_.end()) {
        gui::out << gui::warning << "event_emitter : "
                 << "Event \"" << s_event_name << "\" is not registered to this event_receiver."
                 << std::endl;

        return;
    }

    registered_events_.erase(m_iter);
}

} // namespace lxgui::gui
