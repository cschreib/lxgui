#ifndef LUAPP_VAR_HPP
#define LUAPP_VAR_HPP

#include "lxgui/luapp_exception.hpp"
#include <typeinfo>
#include <memory>

namespace lua
{
typedef std::type_info var_type;

/// Base type : untyped variable
/** The purpose of this class is to have an untyped
*   return value / argument. It allows simple manipulation
*   of events, and a lot of other things.<br><br>
*   It uses a little bit more memory than a simple object :<br>
*   sizeof(void*) + sizeof(T)<br>... and has the overhead of
*   using the v-table (because of inheritance). At last, var
*   does a dynamic_cast (which is not the fastest thing in the
*   world) each time you call get().<br>
*   So this class is definatly slower than base types : <b>use it
*   wisely</b>.
*   \note This class is highly inspired from boost::any.
*/
class var
{
public :

    /// Default constructor.
    var() = default;

    /// Value assignment constructor.
    /** \param mValue The value to assign
    */
    template <class T>
    var(const T& mValue) : pValue_(new value<T>(mValue)) {}

    /// Copy constructor.
    /** \param mVar The var to copy
    */
    var(const var& mVar);

    /// Move constructor.
    /** \param mVar The var to move from
    */
    var(var&& mVar) = default;

    var& operator = (const var& mVar);

    var& operator = (var&& mVar) = default;

    bool operator == (const var& mVar) const;

    bool operator != (const var& mVar) const;

    /// Swaps this value with another.
    /** \param mVar the value to swap with this one
    */
    void swap(var& mVar);

    /// Returns the contained value.
    /** \return The contained value
    *   \note If the provided type doesn't match the
    *         contained value's one, this function throws
    *         an exception.
    */
    template<class T>
    const T& get() const
    {
        const value<T>* pValue = dynamic_cast<const value<T>*>(pValue_.get());

        if (pValue)
            return pValue->mT_;

        throw lua::exception("var",
            "Conversion from "+std::string(pValue ? "type \""+std::string(pValue_->get_type().name())
            +"\"" : "empty lua::var")+" to \""+typeid(T).name()+"\" failed."
        );
    }

    /// Checks if this variable is empty.
    /** \return 'true' if this variable is empty
    *   \note Only the default constructor of var returns
    *         an empty variable.
    */
    bool is_empty() const;

    /// Returns the type of the contained value.
    /** \return The type of the contained value
    *   \note Returns typeid(void) if the variable is empty.
    */
    const var_type& get_type() const;

    /// Checks the contained value's type.
    /** \return 'true' if the contained value's type is the one
    *           you provided
    */
    template<class T>
    bool is_of_type() const
    {
        if (pValue_)
        {
            return pValue_->get_type() == typeid(T);
        }
        else
        {
            return typeid(void) == typeid(T);
        }
    }

    /// Converts this variable to a string.
    /** \return This variable converted to a string
    */
    std::string to_string() const;

    static const var_type& VALUE_NONE;
    static const var_type& VALUE_INT;
    static const var_type& VALUE_UINT;
    static const var_type& VALUE_FLOAT;
    static const var_type& VALUE_DOUBLE;
    static const var_type& VALUE_BOOL;
    static const var_type& VALUE_STRING;
    static const var_type& VALUE_POINTER;

private :

    /** \cond NOT_REMOVE_FROM_DOC
    */

    class value_base
    {
    public :

        virtual ~value_base() {}
        virtual value_base* clone() const = 0;
        virtual const var_type& get_type() const = 0;
    };

    template <class T>
    class value : public value_base
    {
    public :

        friend var;

        value(const T& mT) : mT_(mT) {}

        value_base* clone() const
        {
            return new value(mT_);
        }

        const var_type& get_type() const
        {
            return typeid(T);
        }

    private :

        T mT_;
    };

    /** \endcond
    */

    std::unique_ptr<value_base> pValue_;
};
}

#endif
