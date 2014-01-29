#include "lxgui/utils_exception.hpp"

namespace utils
{
exception::exception() : sMessage_("Undefined exception.")
{
}

exception::exception(const std::string& sMessage) : sMessage_(sMessage)
{
}

exception::exception(const std::string& sClassName, const std::string& sMessage) :
    sMessage_(sClassName+" : "+sMessage)
{
}

exception::~exception()
{
}

const std::string& exception::get_description() const
{
    return sMessage_;
}
}
