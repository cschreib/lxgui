#include "lxgui/gui_keybinder.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/input.hpp"

#include <sol/state.hpp>

namespace lxgui {
namespace gui
{

keybinder::keybinder(input::manager& mInputManager, event_manager& mEventManager) :
    event_receiver(mEventManager), mInputManager_(mInputManager)
{
}

void keybinder::set_key_binding(input::key mKey, sol::protected_function mHandler)
{
    lKeyBindingList_[mKey][input::key::K_UNASSIGNED][input::key::K_UNASSIGNED] = std::move(mHandler);
}

void keybinder::set_key_binding(input::key mKey, input::key mModifier, sol::protected_function mHandler)
{
    lKeyBindingList_[mKey][mModifier][input::key::K_UNASSIGNED] = std::move(mHandler);
}

void keybinder::set_key_binding(input::key mKey, input::key mModifier1, input::key mModifier2, sol::protected_function mHandler)
{
    lKeyBindingList_[mKey][mModifier1][mModifier2] = std::move(mHandler);
}

void keybinder::remove_key_binding(input::key mKey, input::key mModifier1, input::key mModifier2)
{
    auto iter1 = lKeyBindingList_.find(mKey);
    if (iter1 != lKeyBindingList_.end())
    {
        auto iter2 = iter1->second.find(mModifier1);
        if (iter2 != iter1->second.end())
        {
            auto iter3 = iter2->second.find(mModifier2);
            if (iter3 != iter2->second.end())
            {
                iter2->second.erase(iter3);

                if (iter2->second.empty())
                    iter1->second.erase(iter2);

                if (iter1->second.empty())
                    lKeyBindingList_.erase(iter1);
            }
        }
    }
}

std::pair<const sol::protected_function*, std::string> keybinder::find_handler_(
    input::key mKey) const
{
    auto iter1 = lKeyBindingList_.find(mKey);
    if (iter1 == lKeyBindingList_.end())
        return {nullptr, ""};

    for (const auto& iter2 : iter1->second)
    {
        if (iter2.first == input::key::K_UNASSIGNED ||
            !mInputManager_.key_is_down(iter2.first))
            continue;

        // First try to get a match with the most complicated binding with two modifiers
        for (const auto& iter3 : iter2.second)
        {
            if (iter3.first == input::key::K_UNASSIGNED ||
                !mInputManager_.key_is_down(iter3.first))
                continue;

            return {&iter3.second, mInputManager_.get_key_name(mKey, iter2.first, iter3.first)};
        }

        // If none was found, try with only one modifier
        auto iter3 = iter2.second.find(input::key::K_UNASSIGNED);
        if (iter3 != iter2.second.end())
        {
            return {&iter3->second, mInputManager_.get_key_name(mKey, iter2.first)};
        }
    }

    // If no modifier was matching, try with no modifier
    auto iter2 = iter1->second.find(input::key::K_UNASSIGNED);
    if (iter2 != iter1->second.end())
    {
        auto iter3 = iter2->second.find(input::key::K_UNASSIGNED);
        if (iter3 != iter2->second.end())
        {
            return {&iter3->second, mInputManager_.get_key_name(mKey)};
        }
    }

    // No match at all
    return {nullptr, ""};
}

void keybinder::on_event(const event& mEvent)
{
    if (mEvent.get_name() != "KEY_PRESSED")
        return;

    const input::key mKey = mEvent.get<input::key>(0);
    const auto mHandler = find_handler_(mKey);
    if (!mHandler.first)
        return;

    try
    {
        (*mHandler.first)();
    }
    catch (const sol::error& e)
    {
        gui::out << gui::error << "Bound action : " << mHandler.second
            << " : " << e.what() << std::endl;
    }
}

}
}
