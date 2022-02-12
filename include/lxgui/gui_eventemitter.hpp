#ifndef LXGUI_GUI_EVENTEMITTER_HPP
#define LXGUI_GUI_EVENTEMITTER_HPP

#include "lxgui/gui_event.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_signal.hpp"

#include <string>
#include <unordered_map>

namespace lxgui::gui {

class event;

/// Signature of event handler.
using event_handler_signature = void(const event_data&);

/// Signal type for scripts (used internally).
using event_signal = utils::signal<event_handler_signature>;

/// C++ function type for UI script handlers.
using event_handler_function = event_signal::function_type;

/// Generates events and keep tracks of registered callbacks
class event_emitter {
public:
    /// Default constructor
    event_emitter() = default;

    // Non-copiable, non-movable
    event_emitter(const event_emitter&) = delete;
    event_emitter(event_emitter&&)      = delete;
    event_emitter& operator=(const event_emitter&) = delete;
    event_emitter& operator=(event_emitter&&) = delete;

    /// Registers a callback to an event.
    /** \param sEventName The name of the event to listen to
     *   \param mCallback  The function to execute when the event is triggered
     *   \return A object representing the connection between this emitter and the callback.
     *   \note To avoid dangling references, the caller should store the returned connection
     *         object, and use it to terminate the connection when the owner of the callback is
     *         destroyed. This can be done automatically if using the @ref event_receiver helper
     *         class.
     *   \see fire_event
     */
    utils::connection
    register_event(const std::string& s_event_name, event_handler_function m_callback);

    /// Emmit a new event.
    /** \param sEventName The ID of the event which has occurred
     *   \param mData      The payload of the event
     */
    void fire_event(const std::string& s_event_name, event_data m_data = event_data{});

private:
    std::unordered_map<std::string, event_signal> l_registered_event_list_;
};

} // namespace lxgui::gui

#endif
