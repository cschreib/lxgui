#include "lxgui/utils_exception.hpp"

namespace lxgui { namespace utils {

exception::exception(const std::string& sMessage) : sMessage_(sMessage) {}

exception::exception(const std::string& sClassName, const std::string& sMessage) :
    sMessage_(sClassName + " : " + sMessage) {}

const std::string& exception::get_description() const {
    return sMessage_;
}

const char* exception::what() const noexcept {
    return sMessage_.c_str();
}

}} // namespace lxgui::utils
