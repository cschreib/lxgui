#ifndef LXGUI_GUI_VIRTUAL_UIROOT_HPP
#define LXGUI_GUI_VIRTUAL_UIROOT_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_frame_container.hpp"
#include "lxgui/gui_virtual_registry.hpp"

#include <lxgui/utils_observer.hpp>

#include <list>
#include <memory>

namespace lxgui {

namespace gui
{
    class uiobject;
    class frame;
    class manager;
    class renderer;

    /// Root of the virtual UI object hierarchy.
    /** This class contains and owns all virtual "root" frames (frames with no parents)
    *   and is responsible for their lifetime.
    */
    class virtual_uiroot: public frame_container
    {
    public :

        /// Constructor.
        /** \param mManager            The GUI manager
        *   \param mNonVirtualRegistry The registry for non-virtual objects (for error messages)
        */
        explicit virtual_uiroot(manager& mManager, registry& mNonVirtualRegistry);

        /// Destructor.
        ~virtual_uiroot();

        virtual_uiroot(const virtual_uiroot&) = delete;
        virtual_uiroot(virtual_uiroot&&) = delete;
        virtual_uiroot& operator = (const virtual_uiroot&) = delete;
        virtual_uiroot& operator = (virtual_uiroot&&) = delete;

        /// Returns the manager instance associated with this uiroot.
        /** \return The manager instance associated with this uiroot
        */
        manager& get_manager() { return mManager_; }

        /// Returns the manager instance associated with this uiroot.
        /** \return The manager instance associated with this uiroot
        */
        const manager& get_manager() const { return mManager_; }

        /// Returns the UI object registry, which keeps track of all objects in the UI.
        /** \return The registry object
        */
        virtual_registry& get_registry() { return mObjectRegistry_; }

        /// Returns the UI object registry, which keeps track of all objects in the UI.
        /** \return The registry object
        */
        const virtual_registry& get_registry() const { return mObjectRegistry_; }

    private :

        manager& mManager_;
        virtual_registry mObjectRegistry_;
    };
}
}


#endif
