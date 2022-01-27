#include "lxgui/gui_virtual_registry.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_uiobject.hpp"

#include <lxgui/utils_string.hpp>

namespace lxgui {
namespace gui
{

virtual_registry::virtual_registry(const registry& mObjectRegistry) :
    pObjectRegistry_(&mObjectRegistry)
{
}

std::vector<utils::observer_ptr<const uiobject>> virtual_registry::get_virtual_uiobject_list(
    std::string_view sNames) const
{
    std::vector<utils::observer_ptr<const uiobject>> lInheritance;
    for (auto sParent : utils::cut(sNames, ","))
    {
        sParent = utils::trim(sParent, ' ');

        utils::observer_ptr<const uiobject> pObj = get_uiobject_by_name(sParent);

        if (!pObj)
        {
            bool bExistsNonVirtual = pObjectRegistry_->get_uiobject_by_name(sParent) != nullptr;

            gui::out << gui::warning << "gui::manager : "
                << "Cannot find inherited object \"" << sParent << "\""
                << std::string(bExistsNonVirtual ? " (object is not virtual)" : "")
                << ". Inheritance skipped." << std::endl;

            continue;
        }

        lInheritance.push_back(std::move(pObj));
    }

    return lInheritance;
}

}
}
