#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_std.hpp>
#include <lxgui/utils_string.hpp>

// #define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

namespace lxgui {
namespace gui
{
void event_manager::register_event(event_receiver* pReceiver, const std::string& sEventName)
{
    DEBUG_LOG("register "+sEventName+" to "+utils::to_string(pReceiver));
    auto mIterEvent = utils::find_if(lRegisteredEventList_, [&](auto& mObj) {
        return mObj.sName == sEventName;
    });

    if (mIterEvent != lRegisteredEventList_.end())
    {
        auto mIter = utils::find(mIterEvent->lReceiverList, pReceiver);
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
        auto& mEvent = lRegisteredEventList_.emplace_back();
        mEvent.sName = sEventName;
        mEvent.lReceiverList.push_back(pReceiver);
    }
}

void event_manager::unregister_event(event_receiver* pReceiver, const std::string& sEventName)
{
    auto mIterEvent = utils::find_if(lRegisteredEventList_, [&](auto& mObj) {
        return mObj.sName == sEventName;
    });

    if (mIterEvent == lRegisteredEventList_.end())
    {
        gui::out << gui::warning << "event_manager : "
            << "Event \"" << sEventName << "\" is not registered to this event_receiver "
            << "(event_receiver : " << pReceiver << ")." << std::endl;

        return;
    }

    auto mIter = utils::find(mIterEvent->lReceiverList, pReceiver);
    if (mIter == mIterEvent->lReceiverList.end())
    {
        gui::out << gui::warning << "event_manager : "
            << "Event \"" << sEventName << "\" is not registered to this event_receiver "
            << "(event_receiver : " << pReceiver << ")." << std::endl;

        return;
    }

    DEBUG_LOG("unregister " + utils::to_string(pReceiver) + " for " + sEventName);

    *mIter = nullptr;
}

void event_manager::unregister_receiver(event_receiver* pReceiver)
{
    DEBUG_LOG("unregister " + utils::to_string(pReceiver));
    for (auto& mEvent : lRegisteredEventList_)
    {
        auto mIter = utils::find(mEvent.lReceiverList, pReceiver);
        if (mIter != mEvent.lReceiverList.end())
            *mIter = nullptr;
    }
}

void event_manager::fire_event(const event& mEvent)
{
    DEBUG_LOG(mEvent.get_name());
    auto mIter = utils::find_if(lRegisteredEventList_, [&](auto& mObj) {
        return mObj.sName == mEvent.get_name();
    });

    if (mIter == lRegisteredEventList_.end())
        return;

    // This event is registered to one or more event_receivers.
    // Check if this event should only be fired once per frame.
    if (mIter->bFired)
        return;

    DEBUG_LOG(mEvent.get_name()+" started");

    // Now, tell all the event_receivers that this Event has occured.
    for (auto* pReceiver : mIter->lReceiverList)
    {
        DEBUG_LOG(std::string(" ") + utils::to_string(pReceiver));
        if (pReceiver)
            pReceiver->on_event(mEvent);
    }

    DEBUG_LOG(mEvent.get_name()+" finished");

    // Notify the event has been fired this frame.
    if (mEvent.is_once_per_frame())
        mIter->bFired = true;
}

void event_manager::frame_ended()
{
    for (auto& mEvent : lRegisteredEventList_)
    {
        // Clear "fired" state
        mEvent.bFired = false;

        // Remove receivers that have been disconnected.
        auto mIterRem = std::remove_if(mEvent.lReceiverList.begin(), mEvent.lReceiverList.end(),
            [](event_receiver* pReceiver) {
                return pReceiver == nullptr;
            }
        );

        mEvent.lReceiverList.erase(mIterRem, mEvent.lReceiverList.end());
    }
}
}
}
