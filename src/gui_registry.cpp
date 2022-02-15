#include "lxgui/gui_registry.hpp"

#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui::gui {

bool registry::check_region_name(std::string_view name) const {
    if (utils::has_no_content(name)) {
        gui::out << gui::error << "gui::registry : "
                 << "Cannot create a region with a blank name." << std::endl;
        return false;
    }

    if (utils::is_number(name[0])) {
        gui::out << gui::error << "gui::registry : "
                 << "A region's name cannot start by a number : \"" << name << "\" is forbidden."
                 << std::endl;
        return false;
    }

    std::size_t ui_pos = name.find("$");
    if (ui_pos != name.npos && ui_pos != 0) {
        gui::out << gui::error << "gui::registry : "
                 << "A region's name cannot contain the character '$' except at the begining : \""
                 << name << "\" is forbidden." << std::endl;
        return false;
    }

    for (auto c : name) {
        if ((std::isalnum(c) == 0) && c != '_' && c != '$') {
            gui::out << gui::error << "gui::registry : "
                     << "A region's name can only contain alphanumeric symbols, or underscores : \""
                     << name << "\" is forbidden." << std::endl;
            return false;
        }
    }

    return true;
}

bool registry::add_region(utils::observer_ptr<region> p_obj) {
    if (!p_obj) {
        gui::out << gui::error << "gui::registry : Adding a null region." << std::endl;
        return false;
    }

    auto iter_named_obj = named_object_list_.find(p_obj->get_name());
    if (iter_named_obj != named_object_list_.end()) {
        gui::out << gui::warning << "gui::registry : "
                 << "A region with the name \"" << p_obj->get_name() << "\" already exists."
                 << std::endl;
        return false;
    }

    named_object_list_[p_obj->get_name()] = std::move(p_obj);

    return true;
}

void registry::remove_region(const region& p_obj) {
    named_object_list_.erase(p_obj.get_name());
}

utils::observer_ptr<const region> registry::get_region_by_name(std::string_view name) const {
    auto iter = named_object_list_.find(std::string{name});
    if (iter != named_object_list_.end())
        return iter->second;
    else
        return nullptr;
}

} // namespace lxgui::gui
