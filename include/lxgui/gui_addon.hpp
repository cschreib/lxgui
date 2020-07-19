#ifndef LXGUI_GUI_ADDON_HPP
#define LXGUI_GUI_ADDON_HPP

#include <string>
#include <vector>

namespace lxgui {
namespace gui
{
    /// A piece of the user interface
    struct addon
    {
        std::string sName;
        std::string sVersion;
        std::string sUIVersion;
        std::string sAuthor;

        bool bEnabled = true;

        std::string sMainDirectory;
        std::string sDirectory;

        std::vector<std::string> lFileList;
        std::vector<std::string> lSavedVariableList;
    };
}
}

#endif
