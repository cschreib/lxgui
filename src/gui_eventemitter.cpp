#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_event.hpp"

namespace lxgui {
namespace gui
{

utils::connection event_emitter::register_event(const std::string& sEventName,
    event_handler_function mFunction)
{
    return lRegisteredEventList_[sEventName].connect(std::move(mFunction));
}

void event_emitter::fire_event(const event& mEvent)
{
    lRegisteredEventList_[mEvent.get_name()](mEvent);
}

}
}
