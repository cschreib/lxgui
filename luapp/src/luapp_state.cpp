#include "lxgui/luapp_state.hpp"
#include "lxgui/luapp_exception.hpp"
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_filesystem.hpp>
#include <lxgui/utils_range.hpp>
#include <iostream>

namespace lxgui {
namespace lua
{
const luaL_Reg lualibs[] = {
    {"",              luaopen_base},
    {LUA_LOADLIBNAME, luaopen_package},
    {LUA_TABLIBNAME,  luaopen_table},
    {LUA_IOLIBNAME,   luaopen_io},
    {LUA_OSLIBNAME,   luaopen_os},
    {LUA_STRLIBNAME,  luaopen_string},
    {LUA_MATHLIBNAME, luaopen_math},
    {LUA_DBLIBNAME,   luaopen_debug},
    {nullptr,         nullptr}
};

void open_libs(lua_State* pLua_)
{
    for (const luaL_Reg* lib = lualibs; lib->func != nullptr; ++lib)
    {
        #ifdef LXGUI_LUA51
        lua_pushcfunction(pLua_, lib->func);
        lua_pushstring(pLua_, lib->name);
        lua_call(pLua_, 1, 0);
        #else
        luaL_requiref(pLua_, lib->name, lib->func, 1);
        lua_pop(pLua_, 1);
        #endif
    }
}

int  l_treat_error(lua_State*);
void default_print_function(const std::string&);

state::state()
{
    pErrorFunction_ = &l_treat_error;
    pPrintFunction_ = &default_print_function;

    bOwning_ = true;
    pLua_ = luaL_newstate();
    if (!pLua_)
        throw lua::exception("state", "Error while initializing Lua.");

    open_libs(pLua_);
}

state::state(lua_State* pLua)
{
    pErrorFunction_ = &l_treat_error;
    pPrintFunction_ = &default_print_function;

    bOwning_ = false;
    pLua_ = pLua;
    if (!pLua_)
        throw lua::exception("state", "Cannot wrap an invalid Lua state.");
}

state::~state()
{
    if (pLua_ && bOwning_)
        lua_close(pLua_);
}

lua_State* state::get_state()
{
    return pLua_;
}

int l_treat_error(lua_State* pLua)
{
    if (!lua_isstring(pLua, -1))
        return 0;

    lua_Debug d;

    lua_getstack(pLua, 1, &d);
    lua_getinfo(pLua, "Sl", &d);

    std::string sError = std::string(d.short_src) + ":" + utils::to_string(int(d.currentline)) + ": " + std::string(lua_tostring(pLua, -1));
    lua_pushstring(pLua, sError.c_str());

    return 1;
}

void default_print_function(const std::string& s)
{
    std::cerr << s << std::endl;
}

void state::do_file(const std::string& sFile)
{
    if (utils::file_exists(sFile))
    {
        lua_pushcfunction(pLua_, pErrorFunction_);
        uint uiFuncPos = get_top();

        if (luaL_loadfile(pLua_, sFile.c_str()) != 0)
        {
            if (lua_isstring(pLua_, -1))
            {
                std::string sError = lua_tostring(pLua_, -1);
                lua_pop(pLua_, 1);
                lua_remove(pLua_, uiFuncPos);
                throw lua::exception(sError);
            }
            else
            {
                lua_remove(pLua_, uiFuncPos);
                throw lua::exception("state", "Cannot load file : \""+sFile+"\"");
            }
        }

        int iError = lua_pcall(pLua_, 0, LUA_MULTRET, -2);
        if (iError != 0)
        {
            if (lua_isstring(pLua_, -1))
            {
                std::string sError = lua_tostring(pLua_, -1);
                lua_pop(pLua_, 1);
                lua_remove(pLua_, uiFuncPos);
                throw lua::exception(sError);
            }
            else
            {
                lua_pop(pLua_, 1);
                lua_remove(pLua_, uiFuncPos);
                throw lua::exception("state", "Unhandled error.");
            }
        }

        lua_remove(pLua_, uiFuncPos);
    }
    else
        throw lua::exception("state", "do_file : cannot open \""+sFile+"\".");
}

void state::do_string(const std::string& sStr)
{
    lua_pushcfunction(pLua_, pErrorFunction_);
    uint uiFuncPos = get_top();

    if (luaL_loadstring(pLua_, sStr.c_str()) != 0)
    {
        if (lua_isstring(pLua_, -1))
        {
            std::string sError = lua_tostring(pLua_, -1);
            lua_settop(pLua_, uiFuncPos-1);
            throw lua::exception(sError);
        }
        else
        {
            lua_settop(pLua_, uiFuncPos-1);
            throw lua::exception("state", "Unhandled error.");
        }
    }

    int iError = lua_pcall(pLua_, 0, LUA_MULTRET, -2);
    if (iError != 0)
    {
        if (lua_isstring(pLua_, -1))
        {
            std::string sError = lua_tostring(pLua_, -1);
            lua_settop(pLua_, uiFuncPos-1);
            throw lua::exception(sError);
        }
        else
        {
            lua_settop(pLua_, uiFuncPos-1);
            throw lua::exception("state", "Unhandled error.");
        }
    }

    lua_remove(pLua_, uiFuncPos);
}

void state::call_function(const std::string& sFunctionName)
{
    lua_pushcfunction(pLua_, pErrorFunction_);
    uint uiFuncPos = get_top();

    get_global(sFunctionName);

    if (lua_isnil(pLua_, -1))
    {
        lua_settop(pLua_, uiFuncPos-1);
        throw lua::exception("state", "function \""+sFunctionName+"\" does not exist.");
    }

    if (!lua_isfunction(pLua_, -1))
    {
        lua::exception mExcept("state", "\""+sFunctionName+"\" is not a function ("+
            get_type_name(get_type())+" : "+utils::to_string(get_value())+")."
        );
        lua_settop(pLua_, uiFuncPos-1);
        throw mExcept;
    }

    bool bObject = false;
    for (auto iter : utils::range::reverse_iterator(sFunctionName))
    {
        if (*iter == '.')
            break;

        if (*iter == ':')
        {
            std::string sObject = sFunctionName.substr(0, sFunctionName.size() - 1 - (iter - sFunctionName.rbegin()));
            get_global(sObject);
            if (!lua_isnil(pLua_, -1))
                bObject = true;
            else
                lua_pop(pLua_, 1);
        }
    }

    int iError;
    if (bObject)
        iError = lua_pcall(pLua_, 1, LUA_MULTRET, -3);
    else
        iError = lua_pcall(pLua_, 0, LUA_MULTRET, -2);

    if (iError != 0)
    {
        if (lua_isstring(pLua_, -1))
        {
            lua::exception mExcept(lua_tostring(pLua_, -1));
            lua_settop(pLua_, uiFuncPos-1);
            throw mExcept;
        }
        else
        {
            lua_settop(pLua_, uiFuncPos-1);
            throw lua::exception("state", "Unhandled error.");
        }
    }

    lua_remove(pLua_, uiFuncPos);
}

void state::call_function(const std::string& sFunctionName, const std::vector<utils::variant>& lArgumentStack)
{
    lua_pushcfunction(pLua_, pErrorFunction_);
    uint uiFuncPos = get_top();

    get_global(sFunctionName);

    if (lua_isnil(pLua_, -1))
    {
        lua_settop(pLua_, uiFuncPos-1);
        throw lua::exception("state", "function \""+sFunctionName+"\" does not exist.");
    }

    if (lua_isfunction(pLua_, -1))
    {
        lua_settop(pLua_, uiFuncPos-1);
        throw lua::exception("state", "\""+sFunctionName+"\" is not a function ("+
            get_type_name(get_type())+" : "+utils::to_string(get_value())+")");
    }

    for (const auto& mVar : lArgumentStack)
        push(mVar);

    bool bObject = false;
    for (auto iter : utils::range::reverse_iterator(sFunctionName))
    {
        if (*iter == '.')
            break;

        if (*iter == ':')
        {
            std::string sObject = sFunctionName.substr(0, sFunctionName.size() - 1 - (iter - sFunctionName.rbegin()));
            get_global(sObject);
            if (!lua_isnil(pLua_, -1))
                bObject = true;
            else
                lua_pop(pLua_, 1);
        }
    }

    int iError;
    if (bObject)
        iError = lua_pcall(pLua_, lArgumentStack.size() + 1, LUA_MULTRET, -3-lArgumentStack.size());
    else
        iError = lua_pcall(pLua_, lArgumentStack.size(), LUA_MULTRET, -2-lArgumentStack.size());

    if (iError != 0)
    {
        if (lua_isstring(pLua_, -1))
        {
            std::string sError = lua_tostring(pLua_, -1);
            lua_settop(pLua_, uiFuncPos-1);
            throw lua::exception(sError);
        }
        else
        {
            lua_settop(pLua_, uiFuncPos-1);
            throw lua::exception("state", "Unhandled error.");
        }
    }

    lua_remove(pLua_, uiFuncPos);
}

void state::reg(const std::string& sFunctionName, c_function mFunction)
{
    lua_register(pLua_, sFunctionName.c_str(), mFunction);
}

std::string state::format_error(const std::string& sError)
{
    push_string(sError);
    (*pErrorFunction_)(pLua_);
    std::string sResult = get_string();
    pop(2);

    return sResult;
}

void state::print_error(const std::string& sError)
{
    (*pPrintFunction_)(format_error(sError));
}

void state::set_lua_error_function(c_function pFunc)
{
    if (pFunc != 0)
        pErrorFunction_ = pFunc;
    else
        pErrorFunction_ = &l_treat_error;
}

c_function state::get_lua_error_function() const
{
    return pErrorFunction_;
}

void state::set_print_error_function(print_function pFunc)
{
    if (pFunc != 0)
        pPrintFunction_ = pFunc;
    else
        pPrintFunction_ = &default_print_function;
}

bool state::is_serializable(int iIndex)
{
    int iAbsoluteIndex = iIndex >= 0 ? iIndex : int(get_top()+1) + iIndex;
    if (lua_getmetatable(pLua_, iAbsoluteIndex))
    {
        pop();
        get_field("serialize", iAbsoluteIndex);
        if (get_type() == type::FUNCTION)
        {
            pop();
            return true;
        }
        pop();
    }

    type mType = get_type(iIndex);
    return (mType == type::BOOLEAN) || (mType == type::NUMBER) ||
           (mType == type::STRING)  || (mType == type::TABLE) || (mType == type::NIL);
}

std::string state::serialize_global(const std::string& sName)
{
    get_global(sName);

    std::string sContent;
    if (is_serializable())
        sContent = sName+" = "+serialize()+";";

    pop();

    return sContent;
}

std::string state::serialize(const std::string& sTab, int iIndex)
{
    int iAbsoluteIndex = iIndex >= 0 ? iIndex : int(get_top()+1) + iIndex;
    std::string sResult;

    if (lua_getmetatable(pLua_, iAbsoluteIndex))
    {
        pop();
        get_field("serialize", iAbsoluteIndex);
        if (get_type() == type::FUNCTION)
        {
            push_value(iAbsoluteIndex);
            push_string(sTab);
            int iError = lua_pcall(pLua_, 2, 1, 0);
            if (iError == 0)
            {
                sResult += get_string();
                pop();
                return sResult;
            }
        }
        else
            pop();
    }

    type mType = get_type(iIndex);
    switch (mType)
    {
        case type::NIL :
            sResult += "nil";
            break;

        case type::BOOLEAN :
            sResult += utils::to_string(get_bool(iIndex));
            break;

        case type::NUMBER :
            sResult += utils::to_string(get_number(iIndex));
            break;

        case type::STRING :
            sResult += "\""+get_string(iIndex)+"\"";
            break;

        case type::TABLE :
        {
            sResult += "{";

            std::string sContent = "\n";
            push_value(iIndex);
            for (push_nil(); next(); pop())
            {
                if (is_serializable())
                {
                    sContent += sTab + "    [" + serialize(sTab + "    ", -2) + "] = "
                        + serialize(sTab + "    ", -1) + ",\n";
                }
            }
            pop();

            if (sContent != "\n")
                sResult += sContent + sTab;

            sResult += "}";
            break;
        }

        case type::NONE :
        case type::FUNCTION :
        case type::THREAD :
        default : break;
    }

    return sResult;
}

void state::push_number(double dValue)
{
    lua_pushnumber(pLua_, dValue);
}

void state::push_bool(bool bValue)
{
    lua_pushboolean(pLua_, bValue);
}

void state::push_string(const std::string& sValue)
{
    lua_pushstring(pLua_, sValue.c_str());
}

void state::push_nil(uint uiNumber)
{
    for (uint ui = 0u; ui < uiNumber; ++ui)
        lua_pushnil(pLua_);
}

void state::push(const utils::variant& vValue)
{
    std::visit([&](const auto& mInnerValue)
    {
        using inner_type = std::decay_t<decltype(mInnerValue)>;
        if constexpr (std::is_same_v<inner_type, utils::empty>)
            push_nil();
        else if constexpr (std::is_same_v<inner_type, bool>)
            push_bool(mInnerValue);
        else if constexpr (std::is_same_v<inner_type, std::string>)
            push_string(mInnerValue);
        else if constexpr (std::is_arithmetic_v<inner_type>)
            push_number(mInnerValue);
        else
            static_assert(!std::is_same_v<inner_type, inner_type>, "unsupported type");
    }, vValue);
}

void state::push_value(int iIndex)
{
    lua_pushvalue(pLua_, iIndex);
}

void state::push_global(const std::string& sName)
{
    get_global(sName);
}

void state::set_global(const std::string& sName)
{
    std::vector<std::string> lDecomposedName = utils::cut(sName, std::vector<char>{':', '.'});
    if (lDecomposedName.size() == 1)
    {
        lua_setglobal(pLua_, sName.c_str());
        return;
    }

    const std::string sVarName = lDecomposedName.back();
    lDecomposedName.pop_back();

    // Start at 1 to pop the value the user has put on the stack.
    uint uiCounter = 1;

    lua_getglobal(pLua_, lDecomposedName.front().c_str());
    lDecomposedName.erase(lDecomposedName.begin());
    ++uiCounter;

    if (!lua_isnil(pLua_, -1))
    {
        for (const auto& sWord : lDecomposedName)
        {
            lua_getfield(pLua_, -1, sWord.c_str());
            ++uiCounter;
            if (lua_isnil(pLua_, -1))
            {
                pop(uiCounter);
                return;
            }
        }
    }

    lua_pushvalue(pLua_, -(int)uiCounter);
    lua_setfield(pLua_, -2, sVarName.c_str());
    pop(uiCounter);
}

void state::new_table()
{
    lua_newtable(pLua_);
}

bool state::next(int iIndex)
{
    int res = lua_next(pLua_, iIndex);
    return res != 0;
}

void state::pop(uint uiNumber)
{
    lua_pop(pLua_, (int)uiNumber);
}

double state::get_number(int iIndex)
{
    return lua_tonumber(pLua_, iIndex);
}

bool state::get_bool(int iIndex)
{
    return lua_toboolean(pLua_, iIndex) != 0;
}

std::string state::get_string(int iIndex)
{
    return lua_tostring(pLua_, iIndex);
}

utils::variant state::get_value(int iIndex)
{
    int type = lua_type(pLua_, iIndex);
    switch (type)
    {
        case LUA_TBOOLEAN : return get_bool(iIndex);
        case LUA_TNUMBER : return get_number(iIndex);
        case LUA_TSTRING : return get_string(iIndex);
        default : return utils::empty{};
    }
}

uint state::get_top()
{
    return lua_gettop(pLua_);
}

type state::get_type(int iIndex)
{
    int iType = lua_type(pLua_, iIndex);
    switch (iType)
    {
        case LUA_TBOOLEAN :       return type::BOOLEAN;
        case LUA_TFUNCTION :      return type::FUNCTION;
        case LUA_TLIGHTUSERDATA : return type::LIGHTUSERDATA;
        case LUA_TNIL :           return type::NIL;
        case LUA_TNONE :          return type::NONE;
        case LUA_TNUMBER :        return type::NUMBER;
        case LUA_TSTRING :        return type::STRING;
        case LUA_TTABLE :         return type::TABLE;
        case LUA_TTHREAD :        return type::THREAD;
        case LUA_TUSERDATA :      return type::USERDATA;
        default :                 return type::NONE;
    }
}

std::string state::get_type_name(type mType)
{
    switch (mType)
    {
        case type::BOOLEAN :       return lua_typename(pLua_, LUA_TBOOLEAN);
        case type::FUNCTION :      return lua_typename(pLua_, LUA_TFUNCTION);
        case type::LIGHTUSERDATA : return lua_typename(pLua_, LUA_TLIGHTUSERDATA);
        case type::NIL :           return lua_typename(pLua_, LUA_TNIL);
        case type::NONE :          return lua_typename(pLua_, LUA_TNONE);
        case type::NUMBER :        return lua_typename(pLua_, LUA_TNUMBER);
        case type::STRING :        return lua_typename(pLua_, LUA_TSTRING);
        case type::TABLE :         return lua_typename(pLua_, LUA_TTABLE);
        case type::THREAD :        return lua_typename(pLua_, LUA_TTHREAD);
        case type::USERDATA :      return lua_typename(pLua_, LUA_TUSERDATA);
        default :                 return "";
    }
}

void state::get_global(const std::string& sName)
{
    std::vector<std::string> lDecomposedName = utils::cut(sName, std::vector<char>{':','.'});
    lua_getglobal(pLua_, lDecomposedName.front().c_str());

    if (lDecomposedName.size() == 1)
        return;

    lDecomposedName.erase(lDecomposedName.begin());

    for (const auto& sWord : lDecomposedName)
    {
        const type mType = get_type(-1);
        if (mType != type::TABLE && mType != type::USERDATA)
        {
            lua_pop(pLua_, 1);
            lua_pushnil(pLua_);
            return;
        }

        lua_getfield(pLua_, -1, sWord.c_str());
        lua_remove(pLua_, -2);
    }
}

int state::get_global_int(const std::string& sName, bool bCritical, int iDefaultValue)
{
    int i;
    get_global(sName);

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");

        i = iDefaultValue;
    }
    else if (!lua_isnumber(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        print_error("\"" + sName + "\" is expected to be a number");
        i = iDefaultValue;
    }
    else
    {
        i = lua_tonumber(pLua_, -1);
        lua_pop(pLua_, 1);
    }

    return i;
}

double state::get_global_double(const std::string& sName, bool bCritical, double fDefaultValue)
{
    double f;
    get_global(sName);

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");

        f = fDefaultValue;
    }
    else if (!lua_isnumber(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        print_error("\"" + sName + "\" is expected to be a number");
        f = fDefaultValue;
    }
    else
    {
        f = lua_tonumber(pLua_, -1);
        lua_pop(pLua_, 1);
    }

    return f;
}

std::string state::get_global_string(const std::string& sName, bool bCritical, const std::string& sDefaultValue)
{
    std::string s;
    get_global(sName);

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");

        s = sDefaultValue;
    }
    else if (!lua_isstring(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        print_error("\"" + sName + "\" is expected to be a string");
        s = sDefaultValue;
    }
    else
    {
        s = lua_tostring(pLua_, -1);
        lua_pop(pLua_, 1);
    }

    return s;
}

bool state::get_global_bool(const std::string& sName, bool bCritical, bool bDefaultValue)
{
    bool b;
    get_global(sName);

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");

        b = bDefaultValue;
    }
    else if (!lua_isboolean(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        print_error("\"" + sName + "\" is expected to be a bool");
        b = bDefaultValue;
    }
    else
    {
        b = (lua_toboolean(pLua_, -1) != 0);
        lua_pop(pLua_, 1);
    }

    return b;
}

void state::get_field(const std::string& sName, int iIndex)
{
    lua_getfield(pLua_, iIndex, sName.c_str());
}

void state::get_field(int iID, int iIndex)
{
    lua_pushnumber(pLua_, iID);
    if (iIndex >= 0)
        lua_gettable(pLua_, iIndex);
    else
        lua_gettable(pLua_, iIndex - 1);
}

int state::get_field_int(const std::string& sName, bool bCritical, int iDefaultValue, bool bSetValue)
{
    int i = 0;
    lua_getfield(pLua_, -1, sName.c_str());

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");
        else if (bSetValue)
            set_field_int(sName, iDefaultValue);

        i = iDefaultValue;
    }
    else if (!lua_isnumber(pLua_, -1))
    {
        print_error("Field is expected to be a number");
        lua_pop(pLua_, 1);
        i = iDefaultValue;
    }
    else
    {
        i = lua_tonumber(pLua_, -1);
        lua_pop(pLua_, 1);
    }

    return i;
}

double state::get_field_double(const std::string& sName, bool bCritical, double fDefaultValue, bool bSetValue)
{
    double f = 0.0f;
    lua_getfield(pLua_, -1, sName.c_str());

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");
        else if (bSetValue)
            set_field_double(sName, fDefaultValue);

        f = fDefaultValue;
    }
    else if (!lua_isnumber(pLua_, -1))
    {
        print_error("Field is expected to be a number");
        lua_pop(pLua_, 1);
        f = fDefaultValue;
    }
    else
    {
        f = lua_tonumber(pLua_, -1);
        lua_pop(pLua_, 1);
    }

    return f;
}

std::string state::get_field_string(const std::string& sName, bool bCritical, const std::string& sDefaultValue, bool bSetValue)
{
    std::string s;
    lua_getfield(pLua_, -1, sName.c_str());

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");
        else if (bSetValue)
            set_field_string(sName, sDefaultValue);

        s = sDefaultValue;
    }
    else if (!lua_isstring(pLua_, -1))
    {
        print_error("Field is expected to be a string");
        lua_pop(pLua_, 1);
        s = sDefaultValue;
    }
    else
    {
        s = lua_tostring(pLua_, -1);
        lua_pop(pLua_, 1);
    }

    return s;
}

bool state::get_field_bool(const std::string& sName, bool bCritical, bool bDefaultValue, bool bSetValue)
{
    bool b = false;
    lua_getfield(pLua_, -1, sName.c_str());

    if (lua_isnil(pLua_, -1))
    {
        lua_pop(pLua_, 1);
        if (bCritical)
            print_error("Missing " + sName + " attribute");
        else if (bSetValue)
            set_field_bool(sName, bDefaultValue);

        b = bDefaultValue;
    }
    else if (!lua_isboolean(pLua_, -1))
    {
        print_error("Field is expected to be a bool");
        lua_pop(pLua_, 1);
        b = bDefaultValue;
    }
    else
    {
        b = (lua_toboolean(pLua_, -1) != 0);
        lua_pop(pLua_, 1);
    }

    return b;
}

void state::set_field(const std::string& sName)
{
    lua_pushstring(pLua_, sName.c_str());
    lua_pushvalue(pLua_, -2);
    lua_settable(pLua_, -4);
    lua_pop(pLua_, 1);
}

void state::set_field(int iID)
{
    lua_pushnumber(pLua_, iID);
    lua_pushvalue(pLua_, -2);
    lua_settable(pLua_, -4);
    lua_pop(pLua_, 1);
}

void state::set_field_int(const std::string& sName, int iValue)
{
    lua_pushstring(pLua_, sName.c_str());
    lua_pushnumber(pLua_, iValue);
    lua_settable(pLua_, -3);
}

void state::set_field_double(const std::string& sName, double fValue)
{
    lua_pushstring(pLua_, sName.c_str());
    lua_pushnumber(pLua_, fValue);
    lua_settable(pLua_, -3);
}

void state::set_field_string(const std::string& sName, const std::string& sValue)
{
    lua_pushstring(pLua_, sName.c_str());
    lua_pushstring(pLua_, sValue.c_str());
    lua_settable(pLua_, -3);
}

void state::set_field_bool(const std::string& sName, bool bValue)
{
    lua_pushstring(pLua_, sName.c_str());
    lua_pushboolean(pLua_, bValue);
    lua_settable(pLua_, -3);
}

void state::set_field_int(int iID, int iValue)
{
    lua_pushnumber(pLua_, iID);
    lua_pushnumber(pLua_, iValue);
    lua_settable(pLua_, -3);
}

void state::set_field_double(int iID, double fValue)
{
    lua_pushnumber(pLua_, iID);
    lua_pushnumber(pLua_, fValue);
    lua_settable(pLua_, -3);
}

void state::set_field_string(int iID, const std::string& sValue)
{
    lua_pushnumber(pLua_, iID);
    lua_pushstring(pLua_, sValue.c_str());
    lua_settable(pLua_, -3);
}

void state::set_field_bool(int iID, bool bValue)
{
    lua_pushnumber(pLua_, iID);
    lua_pushboolean(pLua_, bValue);
    lua_settable(pLua_, -3);
}

void state::set_top(uint uiSize)
{
    lua_settop(pLua_, uiSize);
}
}
}
