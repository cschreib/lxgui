#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_string.hpp>

//#define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

namespace gui
{
typedef std::multimap<std::string, event_receiver*>::iterator iterator;

void event_manager::register_event(event_receiver* pReceiver, const std::string& sEventName)
{
    if (lReceiverList_.find(sEventName) != lReceiverList_.end())
    {
        // This event is already registered to one or more event_receivers.
        // Check the provided event_receiver isn't one of them :
        auto mRange = lReceiverList_.equal_range(sEventName);
        for (iterator iterReceiver = mRange.first; iterReceiver != mRange.second; ++iterReceiver)
        {
            if (iterReceiver->second == pReceiver)
            {
                gui::out << gui::warning << "event_manager : "
                    << "Event \"" << sEventName << "\" is already registered to this event_receiver "
                    << "(event_receiver : " << pReceiver << ")." << std::endl;
                return;
            }
        }
    }

    lReceiverList_.insert(std::make_pair(sEventName, pReceiver));
}

void event_manager::unregister_event(event_receiver* pReceiver, const std::string& sEventName)
{
    // This event is registered to one or more event_receivers.
    // Check the provided event_receiver is one of them :
    auto mRange = lReceiverList_.equal_range(sEventName);
    for (iterator iterReceiver = mRange.first; iterReceiver != mRange.second; ++iterReceiver)
    {
        if (iterReceiver->second == pReceiver)
        {
            lReceiverList_.erase(iterReceiver);
            return;
        }
    }

    gui::out << gui::warning << "event_manager : "
        << "Event \"" << sEventName << "\" is not registered to this event_receiver "
        << "(event_receiver : " << pReceiver << ")." << std::endl;
}

void event_manager::unregister_receiver(event_receiver* pReceiver)
{
    iterator iterReceiver = lReceiverList_.begin();
    while (iterReceiver != lReceiverList_.end())
    {
        if (iterReceiver->second == pReceiver)
            iterReceiver = lReceiverList_.erase(iterReceiver);
        else
            ++iterReceiver;
    }
}

void event_manager::fire_event(const event& mEvent)
{
    DEBUG_LOG(mEvent.get_name());
    if (lReceiverList_.find(mEvent.get_name()) != lReceiverList_.end())
    {
        DEBUG_LOG(mEvent.get_name()+"!");
        // This event is registered to one or more event_receivers.
        // Check if this event should only be fired once per frame.
        if (utils::find(lFiredEventList_, mEvent.get_name()) == lFiredEventList_.end())
        {
            std::pair<iterator, iterator> mRange = lReceiverList_.equal_range(mEvent.get_name());

            // Now, tell all these event_receivers that this Event has occured.
            for (iterator iterReceiver = mRange.first; iterReceiver != mRange.second; ++iterReceiver)
            {
                DEBUG_LOG(std::string(" ") + utils::to_string(iterReceiver->second));
                iterReceiver->second->on_event(mEvent);
            }


            if (mEvent.is_once_per_frame())
                lFiredEventList_.push_back(mEvent.get_name());
        }
    }
}

void event_manager::frame_ended()
{
    lFiredEventList_.clear();
}
}
