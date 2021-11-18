#ifndef LXGUI_GUI_ALIVE_CHECKER_HPP
#define LXGUI_GUI_ALIVE_CHECKER_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include <lxgui/utils_observer.hpp>

namespace lxgui
{
namespace gui
{
    class uiobject;

    /// Utility class for safe checking of widget validity
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
        explicit alive_checker(uiobject& mObject);

        /// Non-copiable
        alive_checker(const alive_checker&) = delete;

        /// Non-movable
        alive_checker(alive_checker&&) = delete;

        /// Non-copiable
        alive_checker& operator=(const alive_checker&) = delete;

        /// Non-movable
        alive_checker& operator=(alive_checker&&) = delete;

        /// Check if the wrapper widget is still alive
        /** \return 'true' if the widget is alive, 'false' otherwise
        */
        bool is_alive() const;

    private :

        utils::observer_ptr<uiobject> pObject_;
    };
}
}

#endif
