#ifndef LXGUI_GUI_EVENTEMITTER_HPP
#define LXGUI_GUI_EVENTEMITTER_HPP

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

    /// Generates events and keep tracks of registered event receivers
    class event_emitter
    {
    public :
        /// Default constructor
        event_emitter() = default;

        /// Non-copiable
        event_emitter(const event_emitter&) = delete;

        /// Non-movable
        event_emitter(event_emitter&&) = delete;

        /// Non-copiable
        event_emitter& operator=(const event_emitter&) = delete;

        /// Non-movable
        event_emitter& operator=(event_emitter&&) = delete;

        /// Registers an event_receiver as listening to an event.
        /** \param pReceiver The event_receiver to consider
        *   \param sEvent    The name of the event to listen to
        *   \note The event_receiver should call @ref unregister_receiver before
        *         being destroyed, for best performance, however this is not mandatory.
        *         A reference to a deleted event_receiver will automatically be
        *         garbage-collected in @ref frame_ended.
        *   \see fire_event
        */
        void register_event_for(utils::observer_ptr<event_receiver> pReceiver, const std::string& sEvent);

        /// Registers an event_receiver as no longer listening to an event.
        /** \param mReceiver The event_receiver to consider
        *   \param sEvent    The name of the event to stop listening to
        */
        void unregister_event_for(event_receiver& mReceiver, const std::string& sEvent);

        /// Makes an event_receiver stop listening to any event.
        /** \param mReceiver The event_receiver to disable
        */
        void unregister_receiver(event_receiver& mReceiver);

        /// Emmit a new event.
        /** \param mEvent The event which has occurred
        *   \note All event_receivers listening to this event will be notified of the event
        *         by calling @ref lxgui::gui::event_receiver::on_event().
        */
        void fire_event(const event& mEvent);

        /// Tells this manager to clear the fired events list.
        /** \note This is necessary for events that are flagged as "once per frame", and which
        *         will not be triggered more than once until @ref frame_ended()"is called.
        *         This function must not be called from within an event callback, as it will
        *         perform garbage-collection of expired event receivers.
        */
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
