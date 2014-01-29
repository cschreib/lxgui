#ifndef UTILS_EXCEPTION_HPP
#define UTILS_EXCEPTION_HPP

#include "lxgui/utils.hpp"
#include <string>

namespace utils
{
/// Exception class.
class exception
{
public :

    /// Default exception.
    /** \note Reports : "Undefined exception."
    */
    exception();

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

    /// Destructor.
    virtual ~exception();

    /// Returns the message of the exception.
    /** \return The message of the exception
    */
    const std::string& get_description() const;

protected :

    std::string sMessage_;
};
}

#endif
