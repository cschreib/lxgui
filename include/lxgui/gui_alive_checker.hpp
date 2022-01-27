#ifndef LXGUI_GUI_ALIVE_CHECKER_HPP
#define LXGUI_GUI_ALIVE_CHECKER_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include <lxgui/utils_observer.hpp>

namespace lxgui
{
namespace gui
{
    class region;

    /// Utility class for safe checking of region validity
    /** To use this class, construct an instance of alive_checker
    *   with any object you wish to monitor. Then use the object.
    *   Then use alive_checker::is_alive() to check if the object
    *   is still alive.
    *   \note This class will not be able to notice that an object
    *         is not alive if the object was already destroyed before
    *         the alive_checker instance is created.
    */
    class alive_checker
    {
    public :

        /// Contructor.
        explicit alive_checker(region& mObject) : pObject_(mObject.observer_from_this()) {}

        // Non-copiable, non-movable
        alive_checker(const alive_checker&) = delete;
        alive_checker(alive_checker&&) = delete;
        alive_checker& operator=(const alive_checker&) = delete;
        alive_checker& operator=(alive_checker&&) = delete;

        /// Check if the wrapped region is still alive
        /** \return 'true' if the region is alive, 'false' otherwise
        */
        bool is_alive() const { return !pObject_.expired(); }

    private :

        utils::observer_ptr<region> pObject_;
    };
}
}

#endif
