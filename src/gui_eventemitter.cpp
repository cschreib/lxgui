#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_event.hpp"

namespace lxgui {
namespace gui
{

utils::connection event_emitter::register_event(const std::string& sEventName,
    event_handler_function mCallback)
{
    return lRegisteredEventList_[sEventName].connect(std::move(mCallback));
}

void event_emitter::fire_event(const std::string& sEventName, event_data mData)
{
    lRegisteredEventList_[sEventName](std::move(mData));
}

}
}
