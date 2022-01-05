#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_eventemitter.hpp"

namespace lxgui {
namespace gui
{

event_receiver::event_receiver(utils::control_block& mBlock, event_emitter& mEmitter) :
    utils::enable_observer_from_this<event_receiver>(mBlock),
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
