#ifndef LXGUI_GUI_EVENTRECEIVER_HPP
#define LXGUI_GUI_EVENTRECEIVER_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>

#include <string>

namespace lxgui {
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
        explicit event_receiver(event_manager* pManager = nullptr);

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

        void set_event_manager(event_manager* pManager);

    private :

        event_manager* pEventManager_ = nullptr;
    };
}
}

#endif
