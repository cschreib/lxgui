#ifndef LXGUI_GUI_VIRTUAL_REGISTRY_HPP
#define LXGUI_GUI_VIRTUAL_REGISTRY_HPP

#include "lxgui/gui_registry.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_observer.hpp"

#include <string_view>
#include <vector>

namespace lxgui::gui {

class region;

/// Keeps track of virtual UI objects and records their names for lookup.
class virtual_registry : public registry {
public:
    explicit virtual_registry(const registry& object_registry);

    virtual_registry(const virtual_registry& mgr) = default;
    virtual_registry(virtual_registry&& mgr)      = default;
    virtual_registry& operator=(const virtual_registry& mgr) = default;
    virtual_registry& operator=(virtual_registry&& mgr) = default;

    /// Return a list of virtual regions matching the provided comma-separated list.
    /** \param names Comma-separated list of object names
     * \return A vector of objects matching the list. Objects not found will be excluded.
     */
    std::vector<utils::observer_ptr<const region>>
    get_virtual_region_list(std::string_view names) const;

private:
    const registry* object_registry_ = nullptr;
};

} // namespace lxgui::gui

#endif
