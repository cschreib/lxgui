#include "lxgui/gui_addon_registry.hpp"

#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/utils_filesystem.hpp"
#include "lxgui/utils_range.hpp"
#include "lxgui/utils_std.hpp"
#include "lxgui/utils_string.hpp"

#include <fstream>
#include <sol/state.hpp>

namespace {
// This should be incremented for each non-backward compatible change
// to the GUI API, or when new elements are added to the API.
const char* lxgui_ui_version = "0001";
} // namespace

namespace lxgui::gui {

addon_registry::addon_registry(
    sol::state&    m_lua,
    localizer&     m_localizer,
    event_emitter& m_event_emitter,
    root&          m_root,
    virtual_root&  m_virtual_root) :
    m_lua_(m_lua),
    m_localizer_(m_localizer),
    m_event_emitter_(m_event_emitter),
    m_root_(m_root),
    m_virtual_root_(m_virtual_root) {}

void addon_registry::load_addon_toc_(
    const std::string& add_on_name, const std::string& add_on_directory) {
    auto& add_ons = add_on_list_[add_on_directory];
    if (add_ons.find(add_on_name) != add_ons.end())
        return;

    addon m_add_on;
    m_add_on.b_enabled      = true;
    m_add_on.main_directory = utils::cut(add_on_directory, "/").back();
    m_add_on.directory      = add_on_directory + "/" + add_on_name;

    std::string   toc_file = m_add_on.directory + "/" + add_on_name + ".toc";
    std::ifstream m_file(toc_file);
    if (!m_file.is_open())
        return;

    std::string line;
    while (std::getline(m_file, line)) {
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
                    m_add_on.ui_version = value;

                    if (m_add_on.ui_version == lxgui_ui_version)
                        m_add_on.b_enabled = true;
                    else {
                        gui::out << gui::warning << "gui::manager : "
                                 << "Wrong UI version for \"" << add_on_name
                                 << "\" (got : " << m_add_on.ui_version
                                 << ", expected : " << lxgui_ui_version << "). AddOn disabled."
                                 << std::endl;
                        m_add_on.b_enabled = false;
                    }
                } else if (key == "Title")
                    m_add_on.name = value;
                else if (key == "Version")
                    m_add_on.version = value;
                else if (key == "Author")
                    m_add_on.author = value;
                else if (key == "SavedVariables") {
                    for (auto var : utils::cut(value, ",")) {
                        var = utils::trim(var, ' ');
                        if (!utils::has_no_content(var))
                            m_add_on.saved_variable_list.push_back(std::string{var});
                    }
                }
            }
        } else {
            line_view = utils::trim(line_view, ' ');
            if (!utils::has_no_content(line_view))
                m_add_on.file_list.push_back(m_add_on.directory + "/" + std::string{line_view});
        }
    }

    if (m_add_on.name.empty())
        gui::out << gui::error << "gui::manager : Missing addon name in " << toc_file << "."
                 << std::endl;
    else
        add_ons[add_on_name] = m_add_on;
}

void addon_registry::load_addon_files_(const addon& m_add_on) {
    m_localizer_.load_translations(m_add_on.directory);

    p_current_add_on_ = &m_add_on;
    for (const auto& file : m_add_on.file_list) {
        const std::string extension = utils::get_file_extension(file);
        if (extension == ".lua") {
            try {
                m_lua_.do_file(file);
            } catch (const sol::error& e) {
                std::string error = e.what();

                gui::out << gui::error << error << std::endl;

                m_event_emitter_.fire_event("LUA_ERROR", {error});
            }
        } else {
            this->parse_layout_file_(file, m_add_on);
        }
    }

    std::string saved_variables_file =
        "saves/interface/" + m_add_on.main_directory + "/" + m_add_on.name + ".lua";
    if (utils::file_exists(saved_variables_file)) {
        try {
            m_lua_.do_file(saved_variables_file);
        } catch (const sol::error& e) {
            std::string error = e.what();

            gui::out << gui::error << error << std::endl;

            m_event_emitter_.fire_event("LUA_ERROR", {error});
        }
    }

    m_event_emitter_.fire_event("ADDON_LOADED", {m_add_on.name});
}

void addon_registry::load_addon_directory(const std::string& directory) {
    for (const auto& sub_dir : utils::get_directory_list(directory))
        this->load_addon_toc_(sub_dir, directory);

    std::vector<addon*> core_add_on_stack;
    std::vector<addon*> add_on_stack;
    bool                b_core = false;

    auto& add_ons = add_on_list_[directory];

    std::ifstream m_file(directory + "/addons.txt");
    if (m_file.is_open()) {
        std::string line;
        while (std::getline(m_file, line)) {
            utils::replace(line, "\r", "");
            if (line.empty())
                continue;

            std::string_view line_view = line;

            if (line_view[0] == '#') {
                line_view = line_view.substr(1);
                line_view = utils::trim(line_view, ' ');
                b_core    = line_view == "Core";
            } else {
                auto args = utils::cut_first(line_view, ":");
                if (!args.first.empty() && !args.second.empty()) {
                    std::string_view key   = utils::trim(args.first, ' ');
                    std::string_view value = utils::trim(args.second, ' ');
                    auto             iter  = add_ons.find(std::string{key});
                    if (iter != add_ons.end()) {
                        if (b_core)
                            core_add_on_stack.push_back(&iter->second);
                        else
                            add_on_stack.push_back(&iter->second);

                        iter->second.b_enabled = value == "1";
                    }
                }
            }
        }
        m_file.close();
    }

    for (auto* p_add_on : core_add_on_stack) {
        if (p_add_on->b_enabled)
            this->load_addon_files_(*p_add_on);
    }

    for (auto* p_add_on : add_on_stack) {
        if (p_add_on->b_enabled)
            this->load_addon_files_(*p_add_on);
    }

    p_current_add_on_ = nullptr;
}

std::string serialize(const std::string& tab, const sol::object& m_value) noexcept {
    if (m_value.is<double>()) {
        return utils::to_string(m_value.as<double>());
    } else if (m_value.is<int>()) {
        return utils::to_string(m_value.as<int>());
    } else if (m_value.is<std::string>()) {
        return "\"" + utils::to_string(m_value.as<std::string>()) + "\"";
    } else if (m_value.is<sol::table>()) {
        std::string result;
        result += "{";

        std::string content;
        sol::table  m_table = m_value.as<sol::table>();
        for (const auto& m_key_value : m_table) {
            content += tab + "    [" + serialize("", m_key_value.first) +
                       "] = " + serialize(tab + "    ", m_key_value.second) + ",\n";
        }

        if (!content.empty())
            result += "\n" + content + tab;

        result += "}";
        return result;
    }

    return "nil";
}

std::string serialize_global(sol::state& m_lua, const std::string& variable) noexcept {
    sol::object m_value = m_lua.globals()[variable];
    return serialize("", m_value);
}

void addon_registry::save_variables() const {
    for (const auto& m_directory : add_on_list_) {
        for (const auto& m_add_on : utils::range::value(m_directory.second))
            save_variables_(m_add_on);
    }
}

void addon_registry::save_variables_(const addon& m_add_on) const noexcept {
    if (!m_add_on.saved_variable_list.empty()) {
        if (!utils::make_directory("saves/interface/" + m_add_on.main_directory)) {
            gui::out << gui::error
                     << "gui::addon_registry : "
                        "unable to create directory 'saves/interface/"
                     << m_add_on.main_directory << "'" << std::endl;
            return;
        }

        std::ofstream m_file(
            "saves/interface/" + m_add_on.main_directory + "/" + m_add_on.name + ".lua");
        for (const auto& variable : m_add_on.saved_variable_list) {
            std::string serialized = serialize_global(m_lua_, variable);
            if (!serialized.empty())
                m_file << serialized << "\n";
        }
    }
}

const addon* addon_registry::get_current_addon() {
    return p_current_add_on_;
}

void addon_registry::set_current_addon(const addon* p_add_on) {
    p_current_add_on_ = p_add_on;
}

} // namespace lxgui::gui
