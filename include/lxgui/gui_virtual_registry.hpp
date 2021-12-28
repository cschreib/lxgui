#ifndef LXGUI_GUI_VIRTUAL_REGISTRY_HPP
#define LXGUI_GUI_VIRTUAL_REGISTRY_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_registry.hpp"

#include <lxgui/utils_observer.hpp>

#include <vector>

namespace lxgui {
namespace gui
{
    class uiobject;

    /// Manages the user interface
    class virtual_registry : public registry
    {
    public :

        explicit virtual_registry(const registry& mObjectRegistry);

        virtual_registry(const virtual_registry& mMgr) = default;
        virtual_registry(virtual_registry&& mMgr) = default;
        virtual_registry& operator = (const virtual_registry& mMgr) = default;
        virtual_registry& operator = (virtual_registry&& mMgr) = default;

        /// Return a list of virtual uiobjects matching the provided comma-separated list.
        /** \param sNames Comma-separated list of object names
        *   \return A vector of objects matching the list. Objects not found will be excluded.
        */
        std::vector<utils::observer_ptr<const uiobject>> get_virtual_uiobject_list(
            const std::string& sNames) const;

    private :

        const registry* pObjectRegistry_ = nullptr;
    };
}
}


#endif
