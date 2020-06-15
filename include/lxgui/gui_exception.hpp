#ifndef GUI_EXCEPTION_HPP
#define GUI_EXCEPTION_HPP

#include <lxgui/utils_exception.hpp>

namespace lxgui {
namespace gui
{
    /// Exception to be thrown by GUI code.
    /** \note These exceptions should always be handled.<br>
    *         The GUI is never a critical part of the program, so
    *         whatever happens here <b>must not</b> close the
    *         program.
    */
    class exception : public utils::exception
    {
    public :

        explicit exception(const std::string& sMessage) : utils::exception(sMessage)
        {
        }

        exception(const std::string& sClassName, const std::string& sMessage) : utils::exception(sClassName, sMessage)
        {
        }
    };
}
}

#endif
