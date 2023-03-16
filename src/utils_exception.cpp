#include "lxgui/utils_exception.hpp"

namespace lxgui::utils {

exception::exception(const std::string& message) : message_(message) {}

exception::exception(const std::string& class_name, const std::string& message) :
    message_(class_name + ": " + message) {}

const std::string& exception::get_description() const {
    return message_;
}

const char* exception::what() const noexcept {
    return message_.c_str();
}

} // namespace lxgui::utils
