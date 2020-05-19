#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_string.hpp>

//#define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

namespace lxgui {
namespace gui
{
void event_manager::register_event(event_receiver* pReceiver, const std::string& sEventName)
{
    auto mIterEvent = lRegisteredEventList_.find(sEventName);
    if (mIterEvent != lRegisteredEventList_.end())
    {
        auto mIter = std::find(mIterEvent->lReceiverList.begin(), mIterEvent->lReceiverList.end(), pReceiver);
        if (mIter != mIterEvent->lReceiverList.end())
        {
            gui::out << gui::warning << "event_manager : "
                << "Event \"" << sEventName << "\" is already registered to this event_receiver "
                << "(event_receiver : " << pReceiver << ")." << std::endl;
            return;
        }

        mIterEvent->lReceiverList.push_back(pReceiver);
    }
    else
    {
        registered_event mEvent;
        mEvent.sName = sEventName;
        mEvent.lReceiverList.push_back(pReceiver);
        lRegisteredEventList_.insert(std::move(mEvent));
    }
}

void event_manager::unregister_event(event_receiver* pReceiver, const std::string& sEventName)
{
    auto mIterEvent = lRegisteredEventList_.find(sEventName);
    if (mIterEvent == lRegisteredEventList_.end())
    {
        gui::out << gui::warning << "event_manager : "
            << "Event \"" << sEventName << "\" is not registered to this event_receiver "
            << "(event_receiver : " << pReceiver << ")." << std::endl;

        return;
    }

    auto mIter = std::find(mIterEvent->lReceiverList.begin(), mIterEvent->lReceiverList.end(), pReceiver);
    if (mIter == mIterEvent->lReceiverList.end())
    {
        gui::out << gui::warning << "event_manager : "
            << "Event \"" << sEventName << "\" is not registered to this event_receiver "
            << "(event_receiver : " << pReceiver << ")." << std::endl;

        return;
    }

    mIterEvent->lReceiverList.erase(mIter);
}

void event_manager::unregister_receiver(event_receiver* pReceiver)
{
    for (auto& mEvent : lRegisteredEventList_)
    {
        auto mIter = std::find(mEvent.lReceiverList.begin(), mEvent.lReceiverList.end(), pReceiver);
        if (mIter != mEvent.lReceiverList.end())
            mEvent.lReceiverList.erase(mIter);
    }
}

void event_manager::fire_event(const event& mEvent)
{
    DEBUG_LOG(mEvent.get_name());
    auto mIter = lRegisteredEventList_.find(mEvent.get_name());
    if (mIter == lRegisteredEventList_.end())
        return;

    // This event is registered to one or more event_receivers.
    // Check if this event should only be fired once per frame.
    if (mIter->bFired)
        return;

    DEBUG_LOG(mEvent.get_name()+"!");

    // Now, tell all the event_receivers that this Event has occured.
    for (auto* lReceiver : mIter->lReceiverList)
    {
        DEBUG_LOG(std::string(" ") + utils::to_string(iterReceiver->second));
        lReceiver->on_event(mEvent);
    }

    if (mEvent.is_once_per_frame())
        mIter->bFired = true;
}

void event_manager::frame_ended()
{
    for (auto& mEvent : lRegisteredEventList_)
        mEvent.bFired = false;
}
}
}
