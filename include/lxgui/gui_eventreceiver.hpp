#ifndef LXGUI_GUI_EVENTRECEIVER_HPP
#define LXGUI_GUI_EVENTRECEIVER_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include <lxgui/utils_observer.hpp>

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
    class event_receiver : public utils::enable_observer_from_this<event_receiver>
    {
    public :

        /// Constructor.
        /** \param mEmitter The event emitter to listen to
        */
        explicit event_receiver(utils::control_block& mBlock, event_emitter& mEmitter);

        /// Non-copiable
        event_receiver(const event_receiver&) = delete;

        /// Non-movable
        event_receiver(event_receiver&&) = delete;

        /// Non-copiable
        event_receiver& operator=(const event_receiver&) = delete;

        /// Non-movable
        event_receiver& operator=(event_receiver&&) = delete;

        /// Called whenever an Event occurs.
        /** \param mEvent The Event which has occured
        *   \note Only registered events will cause this
        *         function to be called.
        */
        virtual void on_event(const event& mEvent) = 0;

        /// Enables reaction to an Event.
        /** \param sEventName The name of the Event this class should
        *                     react to
        */
        virtual void register_event(const std::string& sEventName);

        /// Disables reaction to an Event.
        /** \param sEventName The name of the Event this class shouldn't
        *                     react to anymore
        */
        virtual void unregister_event(const std::string& sEventName);

    private :

        event_emitter& mEventEmitter_;
    };
}
}

#endif
