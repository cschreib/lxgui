#ifndef LXGUI_GUI_VIRTUAL_ROOT_HPP
#define LXGUI_GUI_VIRTUAL_ROOT_HPP

#include "lxgui/gui_frame_container.hpp"
#include "lxgui/gui_virtual_registry.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_observer.hpp"

#include <list>
#include <memory>

namespace lxgui::gui {

class registry;
class manager;
class renderer;

/**
 * \brief Root of the virtual UI object hierarchy.
 * \details This class contains and owns all virtual "root" frames (frames with no parents)
 * and is responsible for their lifetime.
 */
class virtual_root : public frame_container {
public:
    /**
     * \brief Constructor.
     * \param mgr The GUI manager
     * \param non_virtual_registry The registry for non-virtual objects (for error messages)
     */
    explicit virtual_root(manager& mgr, registry& non_virtual_registry);

    /// Destructor.
    ~virtual_root() override;

    virtual_root(const virtual_root&) = delete;
    virtual_root(virtual_root&&)      = delete;
    virtual_root& operator=(const virtual_root&) = delete;
    virtual_root& operator=(virtual_root&&) = delete;

    /**
     * \brief Returns the manager instance associated with this root.
     * \return The manager instance associated with this root
     */
    manager& get_manager() {
        return manager_;
    }

    /**
     * \brief Returns the manager instance associated with this root.
     * \return The manager instance associated with this root
     */
    const manager& get_manager() const {
        return manager_;
    }

    /**
     * \brief Returns the UI object registry, which keeps track of all objects in the UI.
     * \return The registry object
     */
    virtual_registry& get_registry() {
        return object_registry_;
    }

    /**
     * \brief Returns the UI object registry, which keeps track of all objects in the UI.
     * \return The registry object
     */
    const virtual_registry& get_registry() const {
        return object_registry_;
    }

private:
    manager&         manager_;
    virtual_registry object_registry_;
};

} // namespace lxgui::gui

#endif
