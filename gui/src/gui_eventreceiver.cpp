#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_eventmanager.hpp"

namespace lxgui {
namespace gui
{
event_receiver::event_receiver(event_manager* pManager) :
    pEventManager_(pManager)
{
}

event_receiver::~event_receiver()
{
    if (pEventManager_)
        pEventManager_->unregister_receiver(this);
}

void event_receiver::register_event(const std::string& sEventName)
{
    if (pEventManager_)
        pEventManager_->register_event(this, sEventName);
}

void event_receiver::unregister_event(const std::string& sEventName)
{
    if (pEventManager_)
        pEventManager_->unregister_event(this, sEventName);
}

void event_receiver::set_event_manager(event_manager* pManager)
{
    if (pEventManager_)
        pEventManager_->unregister_receiver(this);

    pEventManager_ = pManager;
}
}
}
