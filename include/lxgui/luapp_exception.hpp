#ifndef LXGUI_LUAPP_EXCEPTION_HPP
#define LXGUI_LUAPP_EXCEPTION_HPP

#include <lxgui/utils_exception.hpp>

namespace lxgui {
namespace lua
{
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
