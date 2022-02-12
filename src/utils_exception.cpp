#include "lxgui/utils_exception.hpp"

namespace lxgui::utils {

exception::exception(const std::string& s_message) : s_message_(s_message) {}

exception::exception(const std::string& s_class_name, const std::string& s_message) :
    s_message_(s_class_name + " : " + s_message) {}

const std::string& exception::get_description() const {
    return s_message_;
}

const char* exception::what() const noexcept {
    return s_message_.c_str();
}

} // namespace lxgui::utils
