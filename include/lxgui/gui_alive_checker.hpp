#ifndef LXGUI_GUI_ALIVE_CHECKER_HPP
#define LXGUI_GUI_ALIVE_CHECKER_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/utils_observer.hpp"

namespace lxgui::gui {

class region;

/**
 * \brief Utility class for safe checking of region validity
 * \details To use this class, construct an instance of alive_checker
 * with any object you wish to monitor. Then use the object.
 * Then use alive_checker::is_alive() to check if the object
 * is still alive.
 *
 * \code{cpp}
 * // Construct the checker
 * alive_checker checker(*this);
 *
 * // Call a member function that may destroy 'this'
 * maybe_destroy_this();
 *
 * // Check if 'this' is still alive
 * if (!checker.is_alive()) return;
 * \endcode
 */
class alive_checker {
public:
    /**
     * \brief Contructor.
     * \param object The object to monitor
     */
    explicit alive_checker(region& object) : object_(object.observer_from_this()) {}

    // Non-copiable, non-movable
    alive_checker(const alive_checker&) = delete;
    alive_checker(alive_checker&&)      = delete;
    alive_checker& operator=(const alive_checker&) = delete;
    alive_checker& operator=(alive_checker&&) = delete;

    /**
     * \brief Check if the wrapped region is still alive
     * \return 'true' if the region is alive, 'false' otherwise
     */
    bool is_alive() const {
        return !object_.expired();
    }

private:
    utils::observer_ptr<region> object_;
};

} // namespace lxgui::gui

#endif
