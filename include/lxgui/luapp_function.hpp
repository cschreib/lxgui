#ifndef LXGUI_LUAPP_FUNCTION_HPP
#define LXGUI_LUAPP_FUNCTION_HPP

#include <lxgui/utils_variant.hpp>

#include <map>
#include <memory>

namespace lxgui {
namespace lua
{
class argument;
class function;

/// Lua data
class data
{
public :

    /// Default constructor
    data() = default;

    /// Main constructor
    /** \param sName    The name associated to this data
    *   \param mLuaType The expected type in Lua
    *   \param pParent  A pointer to the argument that'll be using it
    *   \note You shouldn't have to call it. Consider using function instead.
    */
    data(const std::string& sName, type mLuaType, argument* pParent);

    /// gets data from Lua
    /** \param mLua   The Lua state to use
    *   \param iIndex The index at which to get the value
    *   \note Only called on a valid data (expected type is found).
    */
    void set(state& mLua, int iIndex);

    /// Returns this argument's name.
    /** \return This argument's name
    */
    const std::string& get_name() const;

    /// Returns this argument's value.
    /** \return This argument's value
    */
    const utils::variant& get_value() const;

    /// Returns this argument's Lua type.
    /** \return This argument's Lua type
    */
    lua::type get_type() const;

private :

    std::string    sName_;
    utils::variant mValue_;
    type           mLuaType_ = type::NIL;
    argument*      pParent_ = nullptr;
};

/// argument of a Lua glue
/** Used internally by function.<br>
*   If, for some reason, you need to add an argument
*   without using function's interface, please let me
*   know on the forums and I'll try to see what can be
*   done.
*/
class argument
{
friend class function;
public :

    /// Constructor.
    /** \param sName    The name of this argument (used to print errors in the log)
    *   \param mLuaType The expected type in Lua
    *   \param pParent  A pointer to the function that'll be using it
    */
    argument(const std::string& sName, type mLuaType, function* pParent);

    argument(const argument&) = delete;
    argument(argument&&) = delete;
    argument& operator = (const argument&) = delete;
    argument& operator = (argument&&) = delete;

    /// Adds an alternative to this argument.
    /** \param sName    The name of this alternative argument (used to print errors in the log)
    *   \param mLuaType The expected type in Lua
    */
    void add(const std::string& sName, type mLuaType);

    /// Returns the associated data.
    /** \return The associated data
    */
    data* get_data() const;

    /// Returns the value and converts it to a float.
    /** \return The value and converts it to a float
    */
    float get_number() const;

    /// Returns the value and converts it to an int.
    /** \return The value and converts it to an int
    */
    int get_int() const;

    /// Returns the value and converts it to a bool.
    /** \return The value and converts it to a bool
    */
    bool get_bool() const;

    /// Returns the value and converts it to a string.
    /** \return The value and converts it to a string
    */
    std::string get_string() const;

    /// Returns the value as userdata.
    /** \return The value as userdata
    */
    template<class T>
    T* get() const
    {
        return mLua_.get<T>(utils::get<int>(pData_->get_value()));
    }

    /// Returns the value and converts it to an int.
    /** \return The value and converts it to an int
    */
    int get_index() const;

    /// Returns the actual type of this value.
    /** \return The actual type of this value
    */
    type get_type() const;

    /// Checks if this argument has been provided (in case it is optional).
    /** \return 'true' if the user has provided this argument
    */
    bool is_provided() const;

    /// Sets this argument's data
    /** \param pdata A pointer to the good data
    */
    void set_data(data* pdata);

private :

    /// Checks if this argument has the expected type(s).
    /** \param mLua        The Lua state to use
    *   \param iIndex      The index to check
    *   \param bPrintError Set to 'false' if you don't want that function
    *                      to print errors in the log
    *   \return 'true' if everything went fine
    */
    bool test(state& mLua, int iIndex, bool bPrintError = true);

    bool              bSet_ = false;
    data*             pData_ = nullptr;
    std::vector<data> lData_;
    function*         pParent_ = nullptr;
    state&            mLua_;
};

/// Holds all possible arguments of a Lua function's argument set.
struct argument_list
{
    std::map<uint, std::unique_ptr<argument>> lArg_;
    std::map<uint, std::unique_ptr<argument>> lOptional_;
    uint                                      uiRank_ = 0u;

    argument_list() = default;
    argument_list(const argument_list&) = delete;
    argument_list(argument_list&&) = default;
    argument_list& operator=(const argument_list&) = delete;
    argument_list& operator=(argument_list&&) = default;
};

/// A helper to write Lua glues
/** A glue is a C++ function that is executed
*   in Lua. Due to the way Lua communicates with
*   C++, creating such functions can become boring
*   if you check every argument's type, or if you
*   allow optional arguments, or even two possible
*   types for a single argument.<br>
*   This is done quite easilly with this class.
*/
class function
{
public :

    /// Default constructor.
    /** \param sName       The name of your function (used to print errors in the log)
    *   \param pLua        The Lua state to use
    *   \param uiReturnNbr The maximum number of returned values
    *   \note Call this at the beginning of your glue
    */
    function(const std::string& sName, lua_State* pLua, uint uiReturnNbr = 0u);

    function(const function&) = delete;
    function(function&&) = delete;
    function& operator = (function&) = delete;
    function& operator = (function&&) = delete;

    /// Adds an argument to that function.
    /** \param uiIndex    The index of this argument
    *   \param sName      The name of this argument (used to print errors in the log)
    *   \param mLuaType   The expected type in Lua
    *   \param bOptional 'true' if this argument is not essential and can be ommited
    *   \note Optional arguments work just like in C++ : if you say argument n is optional,
    *         then all the following arguments will have to be optional too.<br>
    *         You can add several different arguments with the same index, but they must
    *         have different Lua types. The function will then choose the right one acording
    *         to the actual Lua type that the user has provided.
    */
    void add(uint uiIndex, const std::string& sName, type mLuaType, bool bOptional = false);

    /// Tells the function there is another parameter set that will be provided.
    void new_param_set();

    /// Returns the indice of the paramater set that the user has provided.
    /** \return The indice of the paramater set that the user has provided
    *   \note Should be called after Check() has been called.
    */
    uint get_param_set_rank();

    /// Checks if all the arguments types and retreive them.
    /** \param bPrintError If set to 'false', this function will fail
    *          silently
    *   \return 'true' if everything went fine
    *   \note You should always check the return value of this function.
    */
    bool check(bool bPrintError = true);

    /// Returns the argument at the provided index.
    /** \param uiIndex The index of the argument
    *   \return The argument at the provided index
    */
    argument* get(uint uiIndex);

    /// Checks if an argument has been provided (in case it is optional).
    /** \param uiIndex The index of the argument
    *   \return 'true' if the user has provided this argument
    *   \note You should always check the return value of this function
    *         whenever you want to use an optional argument.
    */
    bool is_provided(uint uiIndex) const;

    /// Returns the number of provided arguments.
    /** \return The number of provided arguments
    *   \note Should be called after Check() has been called.
    */
    uint get_argument_count() const;

    /// Adds a return value to the function.
    /** \param sValue The value
    *   \note - If you want to return a complex lua object (a table,
    *         a function, ...), <b>you must put it on the
    *         stack yourself</b>, and then call notify_pushed();<br>
    *         - Values are immediatly pushed onto the lua stack, so
    *         the order in which you return your values is important.<br>
    *         - If, for some reason, your can't push one of your return
    *         values, push nil instead.<br>
    *         - Note that this class will automatically fill the stack with
    *         nil until the proper number of return values is reached when
    *         on_return() is called.
    */
    void push(const std::string& sValue);

    /// Adds a return value to the function.
    /** \param dValue The value
    *   \note - If you want to return a complex lua object (a table,
    *         a function, ...), <b>you must put it on the
    *         stack yourself</b>, and then call notify_pushed();<br>
    *         - Values are immediatly pushed onto the lua stack, so
    *         the order in which you return your values is important.<br>
    *         - If, for some reason, your can't push one of your return
    *         values, push nil instead.<br>
    *         - Note that this class will automatically fill the stack with
    *         nil until the proper number of return values is reached when
    *         on_return() is called.
    */
    void push(double dValue);

    /// Adds a return value to the function.
    /** \param fValue The value
    *   \note - If you want to return a complex lua object (a table,
    *         a function, ...), <b>you must put it on the
    *         stack yourself</b>, and then call notify_pushed();<br>
    *         - Values are immediatly pushed onto the lua stack, so
    *         the order in which you return your values is important.<br>
    *         - If, for some reason, your can't push one of your return
    *         values, push nil instead.<br>
    *         - Note that this class will automatically fill the stack with
    *         nil until the proper number of return values is reached when
    *         on_return() is called.
    */
    void push(float fValue);

    /// Adds a return value to the function.
    /** \param iValue The value
    *   \note - If you want to return a complex lua object (a table,
    *         a function, ...), <b>you must put it on the
    *         stack yourself</b>, and then call notify_pushed();<br>
    *         - Values are immediatly pushed onto the lua stack, so
    *         the order in which you return your values is important.<br>
    *         - If, for some reason, your can't push one of your return
    *         values, push nil instead.<br>
    *         - Note that this class will automatically fill the stack with
    *         nil until the proper number of return values is reached when
    *         on_return() is called.
    */
    void push(int iValue);

    /// Adds a return value to the function.
    /** \param uiValue The value
    *   \note - If you want to return a complex lua object (a table,
    *         a function, ...), <b>you must put it on the
    *         stack yourself</b>, and then call notify_pushed();<br>
    *         - Values are immediatly pushed onto the lua stack, so
    *         the order in which you return your values is important.<br>
    *         - If, for some reason, your can't push one of your return
    *         values, push nil instead.<br>
    *         - Note that this class will automatically fill the stack with
    *         nil until the proper number of return values is reached when
    *         on_return() is called.
    */
    void push(uint uiValue);

    /// Adds a return value to the function.
    /** \param bValue The value
    *   \note - If you want to return a complex lua object (a table,
    *         a function, ...), <b>you must put it on the
    *         stack yourself</b>, and then call notify_pushed();<br>
    *         - Values are immediatly pushed onto the lua stack, so
    *         the order in which you return your values is important.<br>
    *         - If, for some reason, your can't push one of your return
    *         values, push nil instead.<br>
    *         - Note that this class will automatically fill the stack with
    *         nil until the proper number of return values is reached when
    *         on_return() is called.
    */
    void push(bool bValue);

    /// Adds nil to the return values.
    /** \param uiNbr The number of nil to push
    *   \note Calling push(utils::variant()) does exactly the same thing. But this
    *         function is clearer.
    */
    void push_nil(uint uiNbr = 1);

    /// Tells this function you pushed a return value.
    /** \note See push for more infos.
    */
    void notify_pushed();

    /// ends the function.
    /** \return The number of returned values
    *   \note Use the return of this function as return value of your glue.<br>
    *         Note that this class will automatically fill the stack with
    *         nil until the proper number of return values is reached.
    */
    int on_return();

    /// Returns the name of this function.
    /** \return The name of this function
    */
    const std::string& get_name() const;

    /// Returns the state used by this function.
    /** \return The state used by this function
    */
    state& get_state();

private :

    std::string                sName_;
    state                      mLua_;
    uint                       uiArgumentCount_ = 0u;
    uint                       uiReturnNbr_ = 0u;
    uint                       uiReturnCount_ = 0u;
    std::vector<argument_list> lArgListStack_;
    argument_list*             pArgList_ = nullptr;
};
}
}

#endif
