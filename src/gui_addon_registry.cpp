#include "lxgui/gui_addon_registry.hpp"

#include "lxgui/gui_event.hpp"
#include "lxgui/gui_event_emitter.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/utils_file_system.hpp"
#include "lxgui/utils_range.hpp"
#include "lxgui/utils_std.hpp"
#include "lxgui/utils_string.hpp"

#include <fstream>
#include <lxgui/extern_sol2_state.hpp>

namespace {
// This should be incremented for each non-backward compatible change
// to the GUI API, or when new elements are added to the API.
const char* lxgui_ui_version = "0001";
} // namespace

namespace lxgui::gui {

addon_registry::addon_registry(
    sol::state& lua, localizer& loc, event_emitter& emitter, root& r, virtual_root& vr) :
    lua_(lua), localizer_(loc), event_emitter_(emitter), root_(r), virtual_root_(vr) {}

void addon_registry::load_addon_toc_(
    const std::string& addon_name, const std::string& addon_directory) {
    auto& addons = addon_list_[addon_directory];
    if (addons.find(addon_name) != addons.end())
        return;

    addon a;
    a.enabled        = true;
    a.main_directory = utils::cut(addon_directory, "/").back();
    a.directory      = addon_directory + "/" + addon_name;

    std::string   toc_file = a.directory + "/" + addon_name + ".toc";
    std::ifstream file(toc_file);
    if (!file.is_open())
        return;

    std::string line;
    while (std::getline(file, line)) {
        utils::replace(line, "\r", "");
        if (line.empty())
            continue;

        std::string_view line_view = line;

        if (line_view.size() >= 2 && line_view[0] == '#' && line_view[1] == '#') {
            line_view = line_view.substr(2);
            line_view = utils::trim(line_view, ' ');
            auto args = utils::cut_first(line_view, ":");
            if (!args.first.empty() && !args.second.empty()) {
                std::string_view key   = utils::trim(args.first, ' ');
                std::string_view value = utils::trim(args.second, ' ');

                if (key == "Interface") {
                    a.ui_version = value;

                    if (a.ui_version == lxgui_ui_version)
                        a.enabled = true;
                    else {
                        gui::out << gui::warning << "gui::manager: "
                                 << "Wrong UI version for \"" << addon_name
                                 << "\" (got: " << a.ui_version
                                 << ", expected: " << lxgui_ui_version << "). AddOn disabled."
                                 << std::endl;
                        a.enabled = false;
                    }
                } else if (key == "Title")
                    a.name = value;
                else if (key == "Version")
                    a.version = value;
                else if (key == "Author")
                    a.author = value;
                else if (key == "SavedVariables") {
                    for (auto var : utils::cut(value, ",")) {
                        var = utils::trim(var, ' ');
                        if (!utils::has_no_content(var))
                            a.saved_variable_list.push_back(std::string{var});
                    }
                }
            }
        } else {
            line_view = utils::trim(line_view, ' ');
            if (!utils::has_no_content(line_view))
                a.file_list.push_back(a.directory + "/" + std::string{line_view});
        }
    }

    if (a.name.empty())
        gui::out << gui::error << "gui::manager: Missing addon name in " << toc_file << "."
                 << std::endl;
    else
        addons[addon_name] = a;
}

void addon_registry::load_addon_files_(const addon& a) {
    localizer_.load_translations(a.directory);

    current_addon_ = &a;
    for (const auto& file : a.file_list) {
        const std::string extension = utils::get_file_extension(file);
        if (extension == ".lua") {
            try {
                lua_.do_file(file);
            } catch (const sol::error& e) {
                std::string err = e.what();
                gui::out << gui::error << err << std::endl;
                event_emitter_.fire_event("LUA_ERROR", {err});
            }
        } else {
            this->parse_layout_file_(file, a);
        }
    }

    std::string saved_variables_file =
        "saves/interface/" + a.main_directory + "/" + a.name + ".lua";
    if (utils::file_exists(saved_variables_file)) {
        try {
            lua_.do_file(saved_variables_file);
        } catch (const sol::error& e) {
            std::string err = e.what();
            gui::out << gui::error << err << std::endl;
            event_emitter_.fire_event("LUA_ERROR", {err});
        }
    }

    event_emitter_.fire_event("ADDON_LOADED", {a.name});
}

void addon_registry::load_addon_directory(const std::string& directory) {
    for (const auto& sub_dir : utils::get_directory_list(directory))
        this->load_addon_toc_(sub_dir, directory);

    std::vector<addon*> core_addon_stack;
    std::vector<addon*> addon_stack;
    bool                core = false;

    auto& addons = addon_list_[directory];

    std::ifstream file(directory + "/addons.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            utils::replace(line, "\r", "");
            if (line.empty())
                continue;

            std::string_view line_view = line;

            if (line_view[0] == '#') {
                line_view = line_view.substr(1);
                line_view = utils::trim(line_view, ' ');
                core      = line_view == "Core";
            } else {
                auto args = utils::cut_first(line_view, ":");
                if (!args.first.empty() && !args.second.empty()) {
                    std::string_view key   = utils::trim(args.first, ' ');
                    std::string_view value = utils::trim(args.second, ' ');
                    auto             iter  = addons.find(std::string{key});
                    if (iter != addons.end()) {
                        if (core)
                            core_addon_stack.push_back(&iter->second);
                        else
                            addon_stack.push_back(&iter->second);

                        iter->second.enabled = value == "1";
                    }
                }
            }
        }
        file.close();
    }

    for (auto* a : core_addon_stack) {
        if (a->enabled)
            this->load_addon_files_(*a);
    }

    for (auto* a : addon_stack) {
        if (a->enabled)
            this->load_addon_files_(*a);
    }

    current_addon_ = nullptr;
}

std::string serialize(const std::string& tab, const sol::object& value) noexcept {
    if (value.is<double>()) {
        return utils::to_string(value.as<double>());
    } else if (value.is<int>()) {
        return utils::to_string(value.as<int>());
    } else if (value.is<std::string>()) {
        return "\"" + utils::to_string(value.as<std::string>()) + "\"";
    } else if (value.is<sol::table>()) {
        std::string result;
        result += "{";

        std::string content;
        sol::table  table = value.as<sol::table>();
        for (const auto& key_value : table) {
            content += tab + "    [" + serialize("", key_value.first) +
                       "] = " + serialize(tab + "    ", key_value.second) + ",\n";
        }

        if (!content.empty())
            result += "\n" + content + tab;

        result += "}";
        return result;
    }

    return "nil";
}

std::string serialize_global(sol::state& lua, const std::string& variable) noexcept {
    sol::object value = lua.globals()[variable];
    return serialize("", value);
}

void addon_registry::save_variables() const {
    for (const auto& directory : addon_list_) {
        for (const auto& a : utils::range::value(directory.second))
            save_variables_(a);
    }
}

void addon_registry::save_variables_(const addon& a) const noexcept {
    if (!a.saved_variable_list.empty()) {
        if (!utils::make_directory("saves/interface/" + a.main_directory)) {
            gui::out << gui::error
                     << "gui::addon_registry: unable to create directory 'saves/interface/"
                     << a.main_directory << "'" << std::endl;
            return;
        }

        std::ofstream file("saves/interface/" + a.main_directory + "/" + a.name + ".lua");
        for (const auto& variable : a.saved_variable_list) {
            std::string serialized = serialize_global(lua_, variable);
            if (!serialized.empty())
                file << serialized << "\n";
        }
    }
}

const addon* addon_registry::get_current_addon() {
    return current_addon_;
}

void addon_registry::set_current_addon(const addon* a) {
    current_addon_ = a;
}

} // namespace lxgui::gui
