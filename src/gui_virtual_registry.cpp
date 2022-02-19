#include "lxgui/gui_virtual_registry.hpp"

#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui::gui {

virtual_registry::virtual_registry(const registry& object_registry) :
    p_object_registry_(&object_registry) {}

std::vector<utils::observer_ptr<const region>>
virtual_registry::get_virtual_region_list(std::string_view names) const {
    std::vector<utils::observer_ptr<const region>> inheritance;
    for (auto parent : utils::cut(names, ",")) {
        parent = utils::trim(parent, ' ');

        utils::observer_ptr<const region> p_obj = get_region_by_name(parent);

        if (!p_obj) {
            bool exists_non_virtual = p_object_registry_->get_region_by_name(parent) != nullptr;

            gui::out << gui::warning << "gui::manager : "
                     << "Cannot find inherited object \"" << parent << "\""
                     << std::string(exists_non_virtual ? " (object is not virtual)" : "")
                     << ". Inheritance skipped." << std::endl;

            continue;
        }

        inheritance.push_back(std::move(p_obj));
    }

    return inheritance;
}

} // namespace lxgui::gui
