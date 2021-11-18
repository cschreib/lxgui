#ifndef LXGUI_GUI_EVENTMANAGER_HPP
#define LXGUI_GUI_EVENTMANAGER_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include <lxgui/utils_observer.hpp>

#include <string>
#include <vector>
#include <list>

namespace lxgui {
namespace gui
{
    class event_receiver;
    class event;

    /// Manages events and their responses
    class event_manager
    {
    public :
        /// Default constructor
        event_manager() = default;

        /// Non-copiable
        event_manager(const event_manager&) = delete;

        /// Non-movable
        event_manager(event_manager&&) = delete;

        /// Non-copiable
        event_manager& operator=(const event_manager&) = delete;

        /// Non-movable
        event_manager& operator=(event_manager&&) = delete;

        /// Enables an event_receiver's reaction to an event.
        /** \param pReceiver The event_receiver to consider
        *   \param sEvent    The name of the event it should react to
        */
        void register_event_for(utils::observer_ptr<event_receiver> pReceiver, const std::string& sEvent);

        /// Disables an event_receiver's reaction to an event.
        /** \param mReceiver The event_receiver to consider
        *   \param sEvent    The name of the event it shouldn't react to anymore
        */
        void unregister_event_for(event_receiver& mReceiver, const std::string& sEvent);

        /// Disables all events connected to the provided event_receiver.
        /** \param mReceiver The event_receiver to disable
        */
        void unregister_receiver(event_receiver& mReceiver);

        /// Tells this manager an Event as occured.
        /** \param mEvent The Event which has occured
        *   \note All event_receivers registred to react to this Event
        *         will be told the Event has occured by calling
        *         event_receiver::on_event().
        */
        void fire_event(const event& mEvent);

        /// Tells this manager to clear the fired Events list.
        void frame_ended();

    private :

        struct registered_event
        {
            std::string sName;
            bool        bFired = false;

            std::list<utils::observer_ptr<event_receiver>> lReceiverList;
        };

        std::list<registered_event> lRegisteredEventList_;
    };
}
}

#endif
