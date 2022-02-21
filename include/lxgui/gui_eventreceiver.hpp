#ifndef LXGUI_GUI_EVENTRECEIVER_HPP
#define LXGUI_GUI_EVENTRECEIVER_HPP

#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/lxgui.hpp"

#include <string>
#include <vector>

namespace lxgui::gui {

class event;
class event_emitter;

/**
 * \brief Utility object to store and manage connections to event signals.
 * This class enables registering callbacks to multiple events, and
 * automatically manages the lifetime of the callbacks.
 */
class event_receiver {
public:
    /**
     * \brief Constructor.
     * \param emitter The event emitter to listen to
     */
    explicit event_receiver(event_emitter& emitter);

    // Non-copiable, non-movable
    event_receiver(const event_receiver&) = delete;
    event_receiver(event_receiver&&)      = delete;
    event_receiver& operator=(const event_receiver&) = delete;
    event_receiver& operator=(event_receiver&&) = delete;

    /**
     * \brief Enables reaction to an event.
     * \param event_name The name of the event this class should react to
     * \param callback The callback function to register to this event
     */
    void register_event(const std::string& event_name, event_handler_function callback);

    /**
     * \brief Disables reaction to an event.
     * \param event_name The name of the event this class shouldn't react to anymore
     */
    void unregister_event(const std::string& event_name);

private:
    struct event_connection {
        std::string              name;
        utils::scoped_connection connection;
    };

    event_emitter&                event_emitter_;
    std::vector<event_connection> registered_events_;
};

} // namespace lxgui::gui

#endif
