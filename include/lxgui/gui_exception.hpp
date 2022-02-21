#ifndef LXGUI_GUI_EXCEPTION_HPP
#define LXGUI_GUI_EXCEPTION_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils_exception.hpp"

namespace lxgui::gui {

/**
 * \brief Exception to be thrown by GUI code.
 * \note These exceptions should always be handled.<br>
 *       The GUI is never a critical part of the program, so
 *       whatever happens here <b>must not</b> close the
 *       program.
 */
class exception : public utils::exception {
public:
    explicit exception(const std::string& message) : utils::exception(message) {}

    exception(const std::string& class_name, const std::string& message) :
        utils::exception(class_name, message) {}
};

} // namespace lxgui::gui

#endif
