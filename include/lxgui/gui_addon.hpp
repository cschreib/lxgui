#ifndef LXGUI_GUI_ADDON_HPP
#define LXGUI_GUI_ADDON_HPP

#include "lxgui/lxgui.hpp"

#include <string>
#include <vector>

namespace lxgui::gui {

/// A piece of the user interface
struct addon {
    std::string name;
    std::string version;
    std::string ui_version;
    std::string author;

    bool enabled = true;

    std::string main_directory;
    std::string directory;

    std::vector<std::string> file_list;
    std::vector<std::string> saved_variable_list;
};

} // namespace lxgui::gui

#endif
