#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_eventmanager.hpp"

namespace lxgui {
namespace gui
{

event_receiver::event_receiver(event_manager& mManager) :
    mEventManager_(mManager)
{
}

void event_receiver::register_event(const std::string& sEventName)
{
    mEventManager_.register_event_for(observer_from_this(), sEventName);
}

void event_receiver::unregister_event(const std::string& sEventName)
{
    mEventManager_.unregister_event_for(*this, sEventName);
}

}
}
