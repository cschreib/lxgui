#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/utils_std.hpp"

namespace lxgui {
namespace gui
{

event_receiver::event_receiver(event_emitter& mEmitter) :
    mEventEmitter_(mEmitter)
{
}

void event_receiver::register_event(const std::string& sEventName, event_handler_function mCallback)
{
    utils::connection mConnection = mEventEmitter_.register_event(sEventName, std::move(mCallback));
    lRegisteredEvents_.push_back({sEventName, std::move(mConnection)});
}

void event_receiver::unregister_event(const std::string& sEventName)
{
    auto mIter = utils::find_if(lRegisteredEvents_, [&](const auto& mEvent)
    {
        return mEvent.sName == sEventName;
    });

    if (mIter == lRegisteredEvents_.end())
    {
        gui::out << gui::warning << "event_emitter : "
            << "Event \"" << sEventName << "\" is not registered to this event_receiver."
            << std::endl;

        return;

    }

    lRegisteredEvents_.erase(mIter);
}

}
}
