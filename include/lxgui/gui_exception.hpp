#ifndef LXGUI_GUI_EXCEPTION_HPP
#define LXGUI_GUI_EXCEPTION_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils_exception.hpp"

namespace lxgui::gui {

/// Exception to be thrown by GUI code.
/** \note These exceptions should always be handled.<br>
 *         The GUI is never a critical part of the program, so
 *         whatever happens here <b>must not</b> close the
 *         program.
 */
class exception : public utils::exception {
public:
    explicit exception(const std::string& s_message) : utils::exception(s_message) {}

    exception(const std::string& s_class_name, const std::string& s_message) :
        utils::exception(s_class_name, s_message) {}
};

} // namespace lxgui::gui

#endif
