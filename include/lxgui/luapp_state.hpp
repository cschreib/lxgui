#ifndef LUAPP_STATE_HPP
#define LUAPP_STATE_HPP

#include "lxgui/lunar.hpp"
#include <lxgui/utils.hpp>
#include <vector>
#include <map>

namespace lxgui {
namespace lua
{
class var;

enum class type
{
    NONE,
    NIL,
    NUMBER,
    BOOLEAN,
    STRING,
    FUNCTION,
    TABLE,
    THREAD,
    LIGHTUSERDATA,
    USERDATA
};

typedef int (*c_function) (lua_State *L);
typedef void (*print_function) (const std::string& s);

/// Lua wrapper
/** Wraps the Lua api into a single class.
*/
class state
{
public :

    /// Constructor
    /** \note Opens a new Lua state and registers all base functions.
    */
    state();

    state(const state&) = delete;
    state(state&&) = delete;
    state& operator = (const state&) = delete;
    state& operator = (state&&) = delete;

    /// Destructor
    /** \note Definately closes the associated Lua state (all data will be lost).
    */
    ~state();

    /// Return the associated Lua state.
    /** \return The associated Lua state
    */
    lua_State* get_state();

    /// Concatenates a Lua table into a string.
    /** \param sTable The name of the table in Lua
    */
    std::string table_to_string(const std::string& sTable);

    /// Copy the content of a table from one Lua state to another.
    /** \param mLua      The other Lua state into wich the table will be copied
    *   \param sSrcName  The name of the table in the source state
    *   \param sDestName The name of the table in the destination state
    *   \note If sDestName is ommited, the table will have the same name in both Lua states.
    */
    void copy_table(state& mLua, const std::string& sSrcName, const std::string& sDestName = "");

    /// Executes a Lua file.
    /** \param sFile The name of the file
    *   \note This function wil throw an exception if any error occurs.
    *         Don't forget to catch them.
    */
    void do_file(const std::string& sFile);

    /// Executes a string containing Lua instructions.
    /** \param sStr The string to execute
    *   \note This function wil throw an exception if any error occurs.
    *         Don't forget to catch them.
    */
    void do_string(const std::string& sStr);

    /// Executes a Lua function.
    /** \param sFunctionName The name of the function to execute
    *   \note This function wil throw an exception if any error occurs.
    *         Don't forget to catch them.
    */
    void call_function(const std::string& sFunctionName);

    /// Executes a Lua function with arguments.
    /** \param sFunctionName  The name of the function to execute
    *   \param lArgumentStack The agument stack (order matters)
    *   \note This function wil throw an exception if any error occurs.
    *         Don't forget to catch them.
    */
    void call_function(const std::string& sFunctionName, const std::vector<var>& lArgumentStack);

    /// Binds a C++ function to a Lua function.
    /** \param sFunctionName The name of the Lua function
    *   \param mFunction     The C++ function to bind
    */
    void reg(const std::string& sFunctionName, c_function mFunction);

    /// regs a Lunar class to be used on this state.
    template<class T>
    void reg()
    {
        Lunar<T>::reg(pLua_);
    }

    /// Prints an error string in the log file with the Lua tag.
    /** \param sError The error string to output
    *   \note This function will print out the current file and line
    */
    void print_error(const std::string& sError);

    /// Formats a raw error string.
    /** \param sError The raw error string
    *   \note Calls the custom error function if provided,
    *         else formats as :<br>
    *         [source]:line: error
    */
    std::string format_error(const std::string& sError);

    /// Sets a custom error formatting function.
    /** \param pFunc The error function
    *   \note The string that is passed to this error function only
    *         contains the raw error message from Lua : no indication
    *         on the location of the error.
    *   \note The default implementation of this function returns :<br>
    *         [source]:line: error<br>
    *   \note See format_error().
    */
    void set_lua_error_function(c_function pFunc);

    /// Returns the current error formatting function.
    /** \return The current error formatting function
    *   \note See set_lua_error_function() for more information.
    *   \note This method is mostly usefull if you want to save the
    *         current error function, use another one temporarily,
    *         and restore the previous one.
    */
    c_function get_lua_error_function() const;

    /// Sets a custom error printing function.
    /** \param pFunc The error printing function
    *   \note By default, errors are sent through std::cerr.
    */
    void set_print_error_function(print_function pFunc);

    /// Checks if a variable is serializable.
    /** \param iIndex The index on the stack of the variable
    *   \return 'true' for strings, numbers, booleans and tables.
    */
    bool is_serializable(int iIndex = -1);

    /// Writes the content of a global variable in a string.
    /** \param sName The name of the global variable
    *   \return The content of the variable
    */
    std::string serialize_global(const std::string& sName);

    /// Writes the content of a variable in a string.
    /** \param sTab   Number of space to put in front of each line
    *   \param iIndex The index on the stack of the variable
    *   \note Can only serialize strings, numbers, booleans and tables.
    *   \return The content of the variable
    */
    std::string serialize(const std::string& sTab = "", int iIndex = -1);

    /// Puts a number on the stack.
    /** \param dValue The value to push on the stack (converted to float)
    */
    void push_number(double dValue);

    /// Puts a boolean on the stack.
    /** \param bValue The value to push on the stack
    */
    void push_bool(bool bValue);

    /// Puts a string on the stack.
    /** \param sValue The value to push on the stack
    */
    void push_string(const std::string& sValue);

    /// Puts a value on the stack.
    /** \param vValue The value to push on the stack
    */
    void push(const var& vValue);

    /// pushes a copy of the value at the given index on the stack.
    /** \param iIndex The index of the value to push
    */
    void push_value(int iIndex);

    /// Puts "nil" (null) on the stack.
    /** \param uiNumber The number of "nil" to push
    */
    void push_nil(uint uiNumber = 1);

    /// Puts the value of a global Lua variable on the stack.
    /** \param sName The name of this variable
    */
    void push_global(const std::string& sName);

    /// Puts a user data (C++ pointer) on the stack.
    /** \param pData The pointer to put on the stack
    */
    template<class T>
    void push_userdata(T* pData)
    {
        lua_pushlightuserdata(pLua_, static_cast<void*>(pData));
    }

    /// pushes a new Lunar object on the stack.
    /** \return A pointer to the Lunar object
    */
    template<class T>
    T* push_new()
    {
        return Lunar<T>::push_new(pLua_);
    }

    /// Sets the value of a global Lua variable.
    /** \param sName The name of this variable
    *   \note The value taken is taken at the top of the stack,
    *         and popped.
    */
    void set_global(const std::string& sName);

    /// Creates a new empty table and pushes it on the stack.
    void new_table();

    /// Iterates over the table at the given index.
    /** \param iIndex The index of the table to iterate on
    *   \note Typical loop (with table at index i) :<br>
    *         for (pLua->push_nil(); pLua->next(i); pLua->pop())
    */
    bool next(int iIndex = -2);

    /// Removes the value at the top of the stack.
    /** \param uiNumber The number of value to remove
    */
    void pop(uint uiNumber = 1);

    /// Returns the value at the given index converted to a number.
    /** \param iIndex The index at which to search for the value
    *   \return The value at the given index converted to a number
    */
    double get_number(int iIndex = -1);

    /// Returns the value at the given index converted to a bool.
    /** \param iIndex The index at which to search for the value
    *   \return The value at the given index converted to a bool
    */
    bool get_bool(int iIndex = -1);

    /// Returns the value at the given index converted to a string.
    /** \param iIndex The index at which to search for the value
    *   \return The value at the given index converted to a string
    */
    std::string get_string(int iIndex = -1);

    /// Returns the value at the given index.
    /** \param iIndex The index at which to search for the value
    *   \return The value at the given index
    */
    var get_value(int iIndex = -1);

    /// Returns the user data (C++ pointer) at the given index.
    template<class T>
    T* get_userdata(int iIndex = -1)
    {
        return static_cast<T*>(lua_touserdata(pLua_, iIndex));
    }

    /// Returns the Lunar object at the given index.
    /** \param iIndex The index at which to search for the value
    *   \return The Lunar object at the given index
    */
    template<class T>
    T* get(int iIndex = -1) const
    {
        return Lunar<T>::wide_check(pLua_, iIndex);
    }

    /// Returns the number of value on the stack.
    /** \return The number of value on the stack
    */
    uint get_top();

    /// Returns the type of the value on the stack.
    /** \param iIndex The index of the value to analyse
    *   \return The type
    */
    type get_type(int iIndex = -1);

    /// Returns the name of a type.
    /** \param mType The type to serialize
    *   \return The name of the type
    */
    std::string get_type_name(type mType);

    /// Puts a global variable on the stack.
    /** \param sName The name of the global variable
    *   \note The name can contain tables, for example :
    *         "MyTable.MyVariable" is a valid input.
    */
    void get_global(const std::string& sName);

    /// Reads an int from Lua.
    /** \param sName         The name of the variable (global scope)
    *   \param bCritical     If 'true', an error will be printed if
    *                        the variable is not found. Else, it will
    *                        be assigned its default value
    *   \param iDefaultValue The default value
    *   \return The int value
    */
    int get_global_int(const std::string& sName, bool bCritical = true, int iDefaultValue = 0);

    /// Reads a float from Lua.
    /** \param sName         The name of the variable (global scope)
    *   \param bCritical     If 'true', an error will be printed if
    *                        the variable is not found. Else, it will
    *                        be assigned its default value
    *   \param dDefaultValue The default value
    *   \return The float value
    */
    double get_global_double(const std::string& sName, bool bCritical = true, double dDefaultValue = 0.0);

    /// Wrapper to read a string from Lua.
    /** \param sName         The name of the variable (global scope)
    *   \param bCritical     If 'true', an error will be printed if
    *                        the variable is not found. Else, it will
    *                        be assigned its default value
    *   \param sDefaultValue The default value
    *   \return The string value
    */
    std::string get_global_string(const std::string& sName, bool bCritical = true, const std::string& sDefaultValue = "");

    /// Reads a bool from Lua.
    /** \param sName         The name of the variable (global scope)
    *   \param bCritical     If 'true', an error will be printed if
    *                        the variable is not found. Else, it will
    *                        be assigned its default value
    *   \param bDefaultValue The default value
    *   \return The bool value
    */
    bool get_global_bool(const std::string& sName, bool bCritical = true, bool bDefaultValue = false);

    /// Puts a value from a Lua table on the stack.
    /** \param sName  The name of the key associated to the value
    *   \param iIndex The position of the table in the stack
    *   \note Puts 'nil' if the key couldn't be found.
    */
    void get_field(const std::string& sName, int iIndex = -1);

    /// Puts a value from a Lua table on the stack.
    /** \param iID    The id of the key associated to the value
    *   \param iIndex The position of the table in the stack
    *   \note Puts 'nil' if the key couldn't be found.
    */
    void get_field(int iID, int iIndex = -1);

    /// Reads an int from a Lua table.
    /** \param sName         The name of the key associated to the value
    *   \param bCritical     If 'true', an error will be printed if
    *                        the variable is not found. Else, it will
    *                        be assigned its default value
    *   \param iDefaultValue The default value
    *   \param bSetValue     If 'true' and the key wasn't found in the table,
    *                        this function will create that key in the Lua table
    *                        and assign it its default value
    *   \return The int value
    *   \note The table that will be used to read the value should be at the top of
    *         the stack just before you call that function.
    */
    int get_field_int(const std::string& sName, bool bCritical = true, int iDefaultValue = 0, bool bSetValue = false);

    /// Reads a float from a Lua table.
    /** \param sName         The name of the key associated to the value
    *   \param bCritical     If 'true', an error will be printed if
    *                        the variable is not found. Else, it will
    *                        be assigned its default value
    *   \param dDefaultValue The default value
    *   \param bSetValue     If 'true' and the key wasn't found in the table,
    *                        this function will create that key in the Lua table
    *                        and assign it its default value
    *   \return The float value
    *   \note The table that will be used to read the value should be at the top of
    *         the stack just before you call that function.
    */
    double get_field_double(const std::string& sName, bool bCritical = true, double dDefaultValue = 0.0, bool bSetValue = false);

    /// Reads a string from a Lua table.
    /** \param sName         The name of the key associated to the value
    *   \param bCritical     If 'true', an error will be printed if
    *                        the variable is not found. Else, it will
    *                        be assigned its default value
    *   \param sDefaultValue The default value
    *   \param bSetValue     If 'true' and the key wasn't found in the table,
    *                        this function will create that key in the Lua table
    *                        and assign it its default value
    *   \return The string value
    *   \note The table that will be used to read the value should be at the top of
    *         the stack just before you call that function.
    */
    std::string get_field_string(const std::string& sName, bool bCritical = true, const std::string& sDefaultValue = "", bool bSetValue = false);

    /// Reads a bool from a Lua table.
    /** \param sName         The name of the key associated to the value
    *   \param bCritical     If 'true', an error will be printed if
    *                        the variable is not found. Else, it will
    *                        be assigned its default value
    *   \param bDefaultValue The default value
    *   \param bSetValue     If 'true' and the key wasn't found in the table,
    *                        this function will create that key in the Lua table
    *                        and assign it its default value
    *   \return The bool value
    *   \note The table that will be used to read the value should be at the top of
    *         the stack just before you call that function.
    */
    bool get_field_bool(const std::string& sName, bool bCritical = true, bool bDefaultValue = false, bool bSetValue = false);

    /// Writes a value into a Lua table.
    /** \param sName The name of the key associated to the value
    *   \note The value to put into the table must be at the top of the stack.<br>
    *         The table must be at the index just before the value.<br>
    *         pops the value from the stack.
    */
    void set_field(const std::string& sName);

    /// Writes a value into a Lua table.
    /** \param iID The ID of the key associated to the value
    *   \note The value to put into the table must be at the top of the stack.<br>
    *         The table must be at the index just before the value.<br>
    *         pops the value from the stack.
    */
    void set_field(int iID);

    /// Writes an int into a Lua table.
    /** \param sName  The name of the key associated to the value
    *   \param iValue The value to set
    *   \note The table that will be used to write the value should be at the top of
    *         the stack just before you call that function.
    */
    void set_field_int(const std::string& sName, int iValue);

    /// Writes a float into a Lua table.
    /** \param sName  The name of the key associated to the value
    *   \param dValue The value to set
    *   \note The table that will be used to write the value should be at the top of
    *         the stack just before you call that function.
    */
    void set_field_double(const std::string& sName, double dValue);

    /// Writes a string into a Lua table.
    /** \param sName  The name of the key associated to the value
    *   \param sValue The value to set
    *   \note The table that will be used to write the value should be at the top of
    *         the stack just before you call that function.
    */
    void set_field_string(const std::string& sName, const std::string& sValue);

    /// Writes a bool into a Lua table.
    /** \param sName  The name of the key associated to the value
    *   \param bValue The value to set
    *   \note The table that will be used to write the value should be at the top of
    *         the stack just before you call that function.
    */
    void set_field_bool(const std::string& sName, bool bValue);

    /// Writes an int into a Lua table.
    /** \param iID    The ID of the key associated to the value
    *   \param iValue The value to set
    *   \note The table that will be used to write the value should be at the top of
    *         the stack just before you call that function.
    */
    void set_field_int(int iID, int iValue);

    /// Writes a float into a Lua table.
    /** \param iID    The ID of the key associated to the value
    *   \param dValue The value to set
    *   \note The table that will be used to write the value should be at the top of
    *         the stack just before you call that function.
    */
    void set_field_double(int iID, double dValue);

    /// Writes a string into a Lua table.
    /** \param iID    The ID of the key associated to the value
    *   \param sValue The value to set
    *   \note The table that will be used to write the value should be at the top of
    *         the stack just before you call that function.
    */
    void set_field_string(int iID, const std::string& sValue);

    /// Writes a bool into a Lua table.
    /** \param iID    The ID of the key associated to the value
    *   \param bValue The value to set
    *   \note The table that will be used to write the value should be at the top of
    *         the stack just before you call that function.
    */
    void set_field_bool(int iID, bool bValue);

    /// Changes the stack size.
    /** \param uiSize The new size of the stack
    *   \note If the stack has more elements, they will be erased.<br>
    *         If is has less, the stack will be filled with nil.
    */
    void set_top(uint uiSize);

    std::string sComString;

    static state* get_state(lua_State* pLua);

private :

    static std::map<lua_State*, state*> lLuaStateMap_;

    lua_State* pLua_;

    c_function pErrorFunction_;
    print_function pPrintFunction_;
};
}
}

#endif
