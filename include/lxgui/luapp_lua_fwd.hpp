#ifndef LUAPP_LUA_FWD_HPP
#define LUAPP_LUA_FWD_HPP

extern "C"
{
    typedef struct lua_State lua_State;
}

namespace lxgui {
namespace lua
{
    /// Binds a C++ member function to a Lua function
    template<typename T>
    struct lunar_binding
    {
        using mfp = int (T::*)(lua_State *L);

        const char* name;
        mfp         mfunc;
    };
}
}

#endif
