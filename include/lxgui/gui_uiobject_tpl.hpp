#ifndef LXGUI_GUI_UIOBJECT_TPL_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils_observer.hpp>
#include <lxgui/gui_uiobject.hpp>
#include <sol/state.hpp>

namespace sol
{
    template<typename T>
    struct unique_usertype_traits<lxgui::utils::observer_ptr<T>>
    {
        static T* get(lua_State*, const lxgui::utils::observer_ptr<T>& pPointer) noexcept
        {
            return pPointer.get();
        }

        static bool is_null(lua_State*, const lxgui::utils::observer_ptr<T>& pPointer) noexcept
        {
            return pPointer.expired();
        }
    };
}

template<typename T>
void sol_lua_check_access(sol::types<T>, lua_State* pLua, int iIndex, sol::stack::record& mTracking)
{
    sol::optional<lxgui::utils::observer_ptr<T>&> mOptional =
        sol::stack::check_get<lxgui::utils::observer_ptr<T>&>(
            pLua, iIndex, sol::no_panic, mTracking);

    if (!mOptional.has_value())
        return;

    if (mOptional->expired())
        throw sol::error("object has been deleted");
}

namespace lxgui {
namespace gui
{
    template<typename T>
    void uiobject::create_glue_()
    {
        get_lua_().globals()[sLuaName_] = observer_from(this);
    }
}
}

#endif
