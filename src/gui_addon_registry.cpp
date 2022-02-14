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
    const std::string& s_add_on_name, const std::string& s_add_on_directory) {
    auto& add_ons = add_on_list_[s_add_on_directory];
    if (add_ons.find(s_add_on_name) != add_ons.end())
        return;

    addon m_add_on;
    m_add_on.b_enabled        = true;
    m_add_on.s_main_directory = utils::cut(s_add_on_directory, "/").back();
    m_add_on.s_directory      = s_add_on_directory + "/" + s_add_on_name;

    std::string   s_toc_file = m_add_on.s_directory + "/" + s_add_on_name + ".toc";
    std::ifstream m_file(s_toc_file);
    if (!m_file.is_open())
        return;

    std::string s_line;
    while (std::getline(m_file, s_line)) {
        utils::replace(s_line, "\r", "");
        if (s_line.empty())
            continue;

        std::string_view s_line_view = s_line;

        if (s_line_view.size() >= 2 && s_line_view[0] == '#' && s_line_view[1] == '#') {
            s_line_view = s_line_view.substr(2);
            s_line_view = utils::trim(s_line_view, ' ');
            auto args   = utils::cut_first(s_line_view, ":");
            if (!args.first.empty() && !args.second.empty()) {
                std::string_view s_key   = utils::trim(args.first, ' ');
                std::string_view s_value = utils::trim(args.second, ' ');

                if (s_key == "Interface") {
                    m_add_on.s_ui_version = s_value;

                    if (m_add_on.s_ui_version == lxgui_ui_version)
                        m_add_on.b_enabled = true;
                    else {
                        gui::out << gui::warning << "gui::manager : "
                                 << "Wrong UI version for \"" << s_add_on_name
                                 << "\" (got : " << m_add_on.s_ui_version
                                 << ", expected : " << lxgui_ui_version << "). AddOn disabled."
                                 << std::endl;
                        m_add_on.b_enabled = false;
                    }
                } else if (s_key == "Title")
                    m_add_on.s_name = s_value;
                else if (s_key == "Version")
                    m_add_on.s_version = s_value;
                else if (s_key == "Author")
                    m_add_on.s_author = s_value;
                else if (s_key == "SavedVariables") {
                    for (auto s_var : utils::cut(s_value, ",")) {
                        s_var = utils::trim(s_var, ' ');
                        if (!utils::has_no_content(s_var))
                            m_add_on.saved_variable_list.push_back(std::string{s_var});
                    }
                }
            }
        } else {
            s_line_view = utils::trim(s_line_view, ' ');
            if (!utils::has_no_content(s_line_view))
                m_add_on.file_list.push_back(m_add_on.s_directory + "/" + std::string{s_line_view});
        }
    }

    if (m_add_on.s_name.empty())
        gui::out << gui::error << "gui::manager : Missing addon name in " << s_toc_file << "."
                 << std::endl;
    else
        add_ons[s_add_on_name] = m_add_on;
}

void addon_registry::load_addon_files_(const addon& m_add_on) {
    m_localizer_.load_translations(m_add_on.s_directory);

    p_current_add_on_ = &m_add_on;
    for (const auto& s_file : m_add_on.file_list) {
        const std::string s_extension = utils::get_file_extension(s_file);
        if (s_extension == ".lua") {
            try {
                m_lua_.do_file(s_file);
            } catch (const sol::error& e) {
                std::string s_error = e.what();

                gui::out << gui::error << s_error << std::endl;

                m_event_emitter_.fire_event("LUA_ERROR", {s_error});
            }
        } else {
            this->parse_layout_file_(s_file, m_add_on);
        }
    }

    std::string s_saved_variables_file =
        "saves/interface/" + m_add_on.s_main_directory + "/" + m_add_on.s_name + ".lua";
    if (utils::file_exists(s_saved_variables_file)) {
        try {
            m_lua_.do_file(s_saved_variables_file);
        } catch (const sol::error& e) {
            std::string s_error = e.what();

            gui::out << gui::error << s_error << std::endl;

            m_event_emitter_.fire_event("LUA_ERROR", {s_error});
        }
    }

    m_event_emitter_.fire_event("ADDON_LOADED", {m_add_on.s_name});
}

void addon_registry::load_addon_directory(const std::string& s_directory) {
    for (const auto& s_sub_dir : utils::get_directory_list(s_directory))
        this->load_addon_toc_(s_sub_dir, s_directory);

    std::vector<addon*> core_add_on_stack;
    std::vector<addon*> add_on_stack;
    bool                b_core = false;

    auto& add_ons = add_on_list_[s_directory];

    std::ifstream m_file(s_directory + "/addons.txt");
    if (m_file.is_open()) {
        std::string s_line;
        while (std::getline(m_file, s_line)) {
            utils::replace(s_line, "\r", "");
            if (s_line.empty())
                continue;

            std::string_view s_line_view = s_line;

            if (s_line_view[0] == '#') {
                s_line_view = s_line_view.substr(1);
                s_line_view = utils::trim(s_line_view, ' ');
                b_core      = s_line_view == "Core";
            } else {
                auto args = utils::cut_first(s_line_view, ":");
                if (!args.first.empty() && !args.second.empty()) {
                    std::string_view s_key   = utils::trim(args.first, ' ');
                    std::string_view s_value = utils::trim(args.second, ' ');
                    auto             iter    = add_ons.find(std::string{s_key});
                    if (iter != add_ons.end()) {
                        if (b_core)
                            core_add_on_stack.push_back(&iter->second);
                        else
                            add_on_stack.push_back(&iter->second);

                        iter->second.b_enabled = s_value == "1";
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

std::string serialize(const std::string& s_tab, const sol::object& m_value) noexcept {
    if (m_value.is<double>()) {
        return utils::to_string(m_value.as<double>());
    } else if (m_value.is<int>()) {
        return utils::to_string(m_value.as<int>());
    } else if (m_value.is<std::string>()) {
        return "\"" + utils::to_string(m_value.as<std::string>()) + "\"";
    } else if (m_value.is<sol::table>()) {
        std::string s_result;
        s_result += "{";

        std::string s_content;
        sol::table  m_table = m_value.as<sol::table>();
        for (const auto& m_key_value : m_table) {
            s_content += s_tab + "    [" + serialize("", m_key_value.first) +
                         "] = " + serialize(s_tab + "    ", m_key_value.second) + ",\n";
        }

        if (!s_content.empty())
            s_result += "\n" + s_content + s_tab;

        s_result += "}";
        return s_result;
    }

    return "nil";
}

std::string serialize_global(sol::state& m_lua, const std::string& s_variable) noexcept {
    sol::object m_value = m_lua.globals()[s_variable];
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
        if (!utils::make_directory("saves/interface/" + m_add_on.s_main_directory)) {
            gui::out << gui::error
                     << "gui::addon_registry : "
                        "unable to create directory 'saves/interface/"
                     << m_add_on.s_main_directory << "'" << std::endl;
            return;
        }

        std::ofstream m_file(
            "saves/interface/" + m_add_on.s_main_directory + "/" + m_add_on.s_name + ".lua");
        for (const auto& s_variable : m_add_on.saved_variable_list) {
            std::string s_serialized = serialize_global(m_lua_, s_variable);
            if (!s_serialized.empty())
                m_file << s_serialized << "\n";
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
