#include "lxgui/gui_keybinder.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/input_dispatcher.hpp"

#include <sol/state.hpp>

namespace lxgui {
namespace gui
{

std::string get_key_debug_name(input::key mKey)
{
    return std::string(input::get_key_codename(mKey));
}

std::string get_key_debug_name(input::key mKey, input::key mModifier)
{
    std::string sString;
    switch (mModifier)
    {
        case (input::key::K_LSHIFT) :
        case (input::key::K_RSHIFT) :
            sString = "Shift + ";
            break;

        case (input::key::K_LCONTROL) :
        case (input::key::K_RCONTROL) :
            sString = "Ctrl + ";
            break;

        case (input::key::K_LMENU) :
        case (input::key::K_RMENU) :
            sString = "Alt + ";
            break;

        default :
            sString = get_key_debug_name(mModifier) + " + ";
            break;
    }

    return sString + get_key_debug_name(mKey);
}

std::string get_key_debug_name(input::key mKey, input::key mModifier1, input::key mModifier2)
{
    std::string sString;
    switch (mModifier1)
    {
        case (input::key::K_LSHIFT) :
        case (input::key::K_RSHIFT) :
            sString = "Shift + ";
            break;

        case (input::key::K_LCONTROL) :
        case (input::key::K_RCONTROL) :
            sString = "Ctrl + ";
            break;

        case (input::key::K_LMENU) :
        case (input::key::K_RMENU) :
            sString = "Alt + ";
            break;

        default :
            sString = get_key_debug_name(mModifier1) + " + ";
            break;
    }

    switch (mModifier2)
    {
        case (input::key::K_LSHIFT) :
        case (input::key::K_RSHIFT) :
            sString += "Shift + ";
            break;

        case (input::key::K_LCONTROL) :
        case (input::key::K_RCONTROL) :
            sString += "Ctrl + ";
            break;

        case (input::key::K_LMENU) :
        case (input::key::K_RMENU) :
            sString += "Alt + ";
            break;

        default :
            sString += get_key_debug_name(mModifier2) + " + ";
            break;
    }

    return sString + get_key_debug_name(mKey);
}

void keybinder::set_key_binding(input::key mKey, sol::protected_function mHandler)
{
    lKeyBindingList_[mKey][input::key::K_UNASSIGNED][input::key::K_UNASSIGNED] = std::move(mHandler);
}

void keybinder::set_key_binding(input::key mKey, input::key mModifier,
    sol::protected_function mHandler)
{
    lKeyBindingList_[mKey][mModifier][input::key::K_UNASSIGNED] = std::move(mHandler);
}

void keybinder::set_key_binding(input::key mKey, input::key mModifier1, input::key mModifier2,
    sol::protected_function mHandler)
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
    input::key mKey, const input::dispatcher& mDispatcher) const
{
    auto iter1 = lKeyBindingList_.find(mKey);
    if (iter1 == lKeyBindingList_.end())
        return {nullptr, ""};

    for (const auto& iter2 : iter1->second)
    {
        if (iter2.first == input::key::K_UNASSIGNED || !mDispatcher.key_is_down(iter2.first))
            continue;

        // First try to get a match with the most complicated binding with two modifiers
        for (const auto& iter3 : iter2.second)
        {
            if (iter3.first == input::key::K_UNASSIGNED || !mDispatcher.key_is_down(iter3.first))
                continue;

            return {&iter3.second, get_key_debug_name(mKey, iter2.first, iter3.first)};
        }

        // If none was found, try with only one modifier
        auto iter3 = iter2.second.find(input::key::K_UNASSIGNED);
        if (iter3 != iter2.second.end())
        {
            return {&iter3->second, get_key_debug_name(mKey, iter2.first)};
        }
    }

    // If no modifier was matching, try with no modifier
    auto iter2 = iter1->second.find(input::key::K_UNASSIGNED);
    if (iter2 != iter1->second.end())
    {
        auto iter3 = iter2->second.find(input::key::K_UNASSIGNED);
        if (iter3 != iter2->second.end())
        {
            return {&iter3->second, get_key_debug_name(mKey)};
        }
    }

    // No match at all
    return {nullptr, ""};
}

bool keybinder::on_key_down(input::key mKey, const input::dispatcher& mDispatcher)
{
    const auto mHandler = find_handler_(mKey, mDispatcher);
    if (!mHandler.first)
        return false;

    try
    {
        (*mHandler.first)();
    }
    catch (const std::exception& e)
    {
        std::string sError = "Bound action: " + mHandler.second + ": " + std::string(e.what());
        throw std::runtime_error(std::move(sError));
    }

    return true;
}

}
}
