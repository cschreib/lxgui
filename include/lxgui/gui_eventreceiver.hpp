#ifndef LXGUI_GUI_EVENTRECEIVER_HPP
#define LXGUI_GUI_EVENTRECEIVER_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/gui_eventemitter.hpp>

#include <vector>
#include <string>

namespace lxgui {
namespace gui
{
    class event;
    class event_emitter;

    /// Abstract interface for event handling
    /** Any class needing to listen to events from an event_emitter
    *   must inherit from this class and implement @ref on_event().
    */
    class event_receiver
    {
    public :

        /// Constructor.
        /** \param mBlock   The owner pointer control block
        *   \param mEmitter The event emitter to listen to
        */
        explicit event_receiver(event_emitter& mEmitter);

        // Non-copiable, non-movable
        event_receiver(const event_receiver&) = delete;
        event_receiver(event_receiver&&) = delete;
        event_receiver& operator=(const event_receiver&) = delete;
        event_receiver& operator=(event_receiver&&) = delete;

        /// Enables reaction to an event.
        /** \param sEventName The name of the event this class should
        *                     react to
        */
        void register_event(const std::string& sEventName, event_handler_function mCallback);

        /// Disables reaction to an event.
        /** \param sEventName The name of the event this class shouldn't
        *                     react to anymore
        */
        void unregister_event(const std::string& sEventName);

    private :

        struct event_connection
        {
            std::string              sName;
            utils::scoped_connection mConnection;
        };

        event_emitter&                mEventEmitter_;
        std::vector<event_connection> lRegisteredEvents_;
    };
}
}

#endif
