#ifndef UTILS_EXCEPTION_HPP
#define UTILS_EXCEPTION_HPP

#include "lxgui/utils.hpp"
#include <string>

namespace utils
{
/// Exception class.
class exception : public std::exception
{
public :

    /// Default exception.
    /** \note Reports : "Undefined exception."
    */
    exception() = default;

    /// Copy constructor
    exception(const exception& mOther) = default;

    /// Simple message exception.
    /** \param sMessage The message to throw
    *   \note Reports : "<sMessage>"
    */
    explicit exception(const std::string& sMessage);

    /// Class name + message exception.
    /** \param sClassName The name of the class which throws the exception
    *   \param sMessage   The message to throw
    *   \note Reports : "<sClassName> : <sMessage>"
    */
    exception(const std::string& sClassName, const std::string& sMessage);

    /// Returns the message of the exception.
    /** \return The message of the exception
    */
    const std::string& get_description() const;

    /// Override std::exception::what()
    /** \return The message of the exception
    */
    const char* what() const noexcept override;

protected :

    std::string sMessage_ = "Undefined exception.";
};
}

#endif
