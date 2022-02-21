#ifndef LXGUI_UTILS_EXCEPTION_HPP
#define LXGUI_UTILS_EXCEPTION_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

#include <string>

namespace lxgui::utils {

/// Exception class.
class exception : public std::exception {
public:
    /// Default exception.
    /** \note Reports: "Undefined exception."
     */
    exception() = default;

    /// Copy constructor
    exception(const exception& other) = default;

    /// Simple message exception.
    /** \param message The message to throw
     *   \note Reports: "<message>"
     */
    explicit exception(const std::string& message);

    /// Class name + message exception.
    /** \param class_name The name of the class which throws the exception
     *   \param message   The message to throw
     *   \note Reports: "<class_name>: <message>"
     */
    exception(const std::string& class_name, const std::string& message);

    /// Returns the message of the exception.
    /** \return The message of the exception
     */
    const std::string& get_description() const;

    /// Override std::exception::what()
    /** \return The message of the exception
     */
    const char* what() const noexcept override;

protected:
    std::string message_ = "Undefined exception.";
};

} // namespace lxgui::utils

#endif
