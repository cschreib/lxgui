#ifndef LXGUI_GUI_REGISTRY_HPP
#define LXGUI_GUI_REGISTRY_HPP

#include <lxgui/lxgui.hpp>

#include <lxgui/utils_observer.hpp>

#include <string>
#include <string_view>
#include <unordered_map>

namespace lxgui {
namespace gui
{
    class uiobject;

    /// Manages the user interface
    class registry
    {
    public :

        registry() = default;
        virtual ~registry() = default;
        registry(const registry& mMgr) = default;
        registry(registry&& mMgr) = default;
        registry& operator = (const registry& mMgr) = default;
        registry& operator = (registry&& mMgr) = default;

        /// Checks the provided string is suitable for naming a widget.
        /** \param sName The string to test
        *   \return 'true' if the provided string can be the name of a widget
        */
        bool check_uiobject_name(std::string_view sName) const;

        /// Adds an uiobject to be handled by this registry.
        /** \param pObj The object to add
        *   \return 'false' if the name of the widget was already taken
        */
        bool add_uiobject(utils::observer_ptr<uiobject> pObj);

        /// Removes an uiobject from this registry.
        /** \param mObj The object to remove
        */
        void remove_uiobject(const uiobject& mObj);

        /// Returns the uiobject associated with the given name.
        /** \param sName    The name of the widget you're after
        *   \return The uiobject associated with the given name, or nullptr if not found
        */
        utils::observer_ptr<const uiobject> get_uiobject_by_name(std::string_view sName) const;

        /// Returns the uiobject associated with the given name.
        /** \param sName    The name of the widget you're after
        *   \return The uiobject associated with the given name, or nullptr if not found
        */
        utils::observer_ptr<uiobject> get_uiobject_by_name(std::string_view sName)
        {
            return utils::const_pointer_cast<uiobject>(
                const_cast<const registry*>(this)->get_uiobject_by_name(sName));
        }

    private :

        template<typename T>
        using string_map = std::unordered_map<std::string,T>;

        string_map<utils::observer_ptr<uiobject>> lNamedObjectList_;
    };
}
}


#endif
