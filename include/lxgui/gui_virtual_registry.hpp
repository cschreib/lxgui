#ifndef LXGUI_GUI_VIRTUAL_REGISTRY_HPP
#define LXGUI_GUI_VIRTUAL_REGISTRY_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_registry.hpp"

#include <lxgui/utils_observer.hpp>

#include <vector>
#include <string_view>

namespace lxgui {
namespace gui
{
    class region;

    /// Keeps track of virtual UI objects and records their names for lookup.
    class virtual_registry : public registry
    {
    public :

        explicit virtual_registry(const registry& mObjectRegistry);

        virtual_registry(const virtual_registry& mMgr) = default;
        virtual_registry(virtual_registry&& mMgr) = default;
        virtual_registry& operator = (const virtual_registry& mMgr) = default;
        virtual_registry& operator = (virtual_registry&& mMgr) = default;

        /// Return a list of virtual regions matching the provided comma-separated list.
        /** \param sNames Comma-separated list of object names
        *   \return A vector of objects matching the list. Objects not found will be excluded.
        */
        std::vector<utils::observer_ptr<const region>> get_virtual_region_list(
            std::string_view sNames) const;

    private :

        const registry* pObjectRegistry_ = nullptr;
    };
}
}


#endif
