#include "lxgui/gui_registry.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_uiobject.hpp"

#include <lxgui/utils_string.hpp>

namespace lxgui {
namespace gui
{

bool registry::check_uiobject_name(std::string_view sName) const
{
    if (utils::has_no_content(sName))
    {
        gui::out << gui::error << "gui::registry : "
            << "Cannot create a uiobject with a blank name." << std::endl;
        return false;
    }

    if (utils::is_number(sName[0]))
    {
        gui::out << gui::error << "gui::registry : "
            << "A widget's name cannot start by a number : \"" << sName
            << "\" is forbidden." << std::endl;
        return false;
    }

    std::size_t uiPos = sName.find("$");
    if (uiPos != sName.npos && uiPos != 0)
    {
        gui::out << gui::error << "gui::registry : "
            << "A widget's name cannot contain the character '$' except at the begining : \""
            << sName << "\" is forbidden." << std::endl;
        return false;
    }

    for (auto c : sName)
    {
        if (!std::isalnum(c) && c != '_' && c != '$')
        {
            gui::out << gui::error << "gui::registry : "
                << "A widget's name can only contain alphanumeric symbols, or underscores : \""
                << sName << "\" is forbidden." << std::endl;
            return false;
        }
    }

    return true;
}

bool registry::add_uiobject(utils::observer_ptr<uiobject> pObj)
{
    if (!pObj)
    {
        gui::out << gui::error << "gui::registry : Adding a null widget." << std::endl;
        return false;
    }

    auto iterNamedObj = lNamedObjectList_.find(pObj->get_name());
    if (iterNamedObj != lNamedObjectList_.end())
    {
        gui::out << gui::warning << "gui::registry : "
            << "A widget with the name \"" << pObj->get_name()
            << "\" already exists." << std::endl;
        return false;
    }

    lNamedObjectList_[pObj->get_name()] = std::move(pObj);

    return true;
}

void registry::remove_uiobject(const uiobject& pObj)
{
    lNamedObjectList_.erase(pObj.get_name());
}

utils::observer_ptr<const uiobject> registry::get_uiobject_by_name(std::string_view sName) const
{
    auto iter = lNamedObjectList_.find(std::string{sName});
    if (iter != lNamedObjectList_.end())
        return iter->second;
    else
        return nullptr;
}

}
}
