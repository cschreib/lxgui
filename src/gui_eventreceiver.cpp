#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_eventemitter.hpp"

namespace lxgui {
namespace gui
{

event_receiver::event_receiver(event_emitter& mEmitter) :
    mEventEmitter_(mEmitter)
{
}

void event_receiver::register_event(const std::string& sEventName)
{
    mEventEmitter_.register_event_for(observer_from_this(), sEventName);
}

void event_receiver::unregister_event(const std::string& sEventName)
{
    mEventEmitter_.unregister_event_for(*this, sEventName);
}

}
}
