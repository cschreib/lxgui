#ifndef GUI_EVENTRECEIVER_HPP
#define GUI_EVENTRECEIVER_HPP

#include <lxgui/utils.hpp>
#include <string>

namespace gui
{
    class event;
    class event_manager;

    /// Abstract interface for event handling
    /** All classes which should react to some events
    *   should inherit from this class.<br>
    *   They will automatically react to Events thanks
    *   to the event_manager.
    */
    class event_receiver
    {
    public :

        /// Constructor.
        explicit event_receiver(event_manager* mManager = nullptr);

        /// Destructor.
        virtual ~event_receiver();

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

    protected :

        event_manager* pEventManager_;
    };
}

#endif
