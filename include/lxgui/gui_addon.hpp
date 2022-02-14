#ifndef LXGUI_GUI_ADDON_HPP
#define LXGUI_GUI_ADDON_HPP

#include "lxgui/lxgui.hpp"

#include <string>
#include <vector>

namespace lxgui::gui {

/// A piece of the user interface
struct addon {
    std::string s_name;
    std::string s_version;
    std::string s_ui_version;
    std::string s_author;

    bool b_enabled = true;

    std::string s_main_directory;
    std::string s_directory;

    std::vector<std::string> file_list;
    std::vector<std::string> saved_variable_list;
};

} // namespace lxgui::gui

#endif
