#ifndef LXGUI_GUI_REGION_TPL_HPP
#define LXGUI_GUI_REGION_TPL_HPP

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/gui_root.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_observer.hpp"
#include "lxgui/utils_string.hpp"

#include <sol/state.hpp>

/** \cond INCLUDE_INTERNALS_IN_DOC
 */
namespace sol {

template<typename T>
struct unique_usertype_traits<lxgui::utils::observer_ptr<T>> {
    static T* get(lua_State*, const lxgui::utils::observer_ptr<T>& pPointer) noexcept {
        return pPointer.get();
    }

    static bool is_null(lua_State*, const lxgui::utils::observer_ptr<T>& pPointer) noexcept {
        return pPointer.expired();
    }
};

} // namespace sol
/** \endcond
 */

template<typename T>
void sol_lua_check_access(
    sol::types<T>, lua_State* pLua, int iIndex, sol::stack::record& /*mTracking*/) {
    // NB: not sure why, but using mTracking here leads to issues later on, so
    // ignore it for now.

    sol::optional<lxgui::utils::observer_ptr<T>&> mOptional =
        sol::stack::check_get<lxgui::utils::observer_ptr<T>&>(
            pLua, iIndex, sol::no_panic /*, mTracking*/);

    if (!mOptional.has_value())
        return;

    if (mOptional->expired())
        throw sol::error("object has been deleted");
}

namespace lxgui::gui {

inline utils::observer_ptr<region>
get_object(manager& mManager, const std::variant<std::string, region*>& mParent) {
    return std::visit(
        [&](const auto& mValue) -> utils::observer_ptr<region> {
            using data_type = std::decay_t<decltype(mValue)>;
            if constexpr (std::is_same_v<data_type, std::string>) {
                if (utils::has_no_content(mValue))
                    return nullptr;

                auto pParent = mManager.get_root().get_registry().get_region_by_name(mValue);
                if (!pParent)
                    throw sol::error("no region with name \"" + mValue + "\"");

                return pParent;
            } else {
                return observer_from(mValue);
            }
        },
        mParent);
}

template<typename T>
utils::observer_ptr<T> get_object(manager& mManager, const std::variant<std::string, T*>& mParent) {
    return std::visit(
        [&](const auto& mValue) -> utils::observer_ptr<T> {
            using data_type = std::decay_t<decltype(mValue)>;
            if constexpr (std::is_same_v<data_type, std::string>) {
                if (utils::has_no_content(mValue))
                    return nullptr;

                auto pParentObject = mManager.get_root().get_registry().get_region_by_name(mValue);
                if (!pParentObject)
                    throw sol::error("no region with name \"" + mValue + "\"");

                auto pParent = down_cast<T>(pParentObject);
                if (!pParent)
                    throw sol::error(
                        "region \"" + mValue + "\" is not a " + std::string(T::CLASS_NAME));

                return pParent;
            } else {
                return observer_from(mValue);
            }
        },
        mParent);
}

#if defined(LXGUI_COMPILER_EMSCRIPTEN)
// Workaround for compiler crash in Emscripten; explicitly convert member
// function pointers to free functions. sol2 is able to do this automatically,
// but Emscripten/clang is not happy about it.
template<typename T, T F>
struct member_function_holder;

template<typename R, typename T, typename... Args, R (T::*Function)(Args...)>
struct member_function_holder<R (T::*)(Args...), Function> {
    static constexpr auto make_free_function() {
        return [](T& mSelf, Args... args) { return (mSelf.*Function)(std::move(args)...); };
    }
};

template<typename R, typename T, typename... Args, R (T::*Function)(Args...) const>
struct member_function_holder<R (T::*)(Args...) const, Function> {
    static constexpr auto make_free_function() {
        return [](const T& mSelf, Args... args) { return (mSelf.*Function)(std::move(args)...); };
    }
};

template<auto T>
constexpr auto member_function() {
    return member_function_holder<decltype(T), T>::make_free_function();
}
#else
// Simply use the member function pointer directly for all other compilers.
template<auto T>
constexpr auto member_function() {
    return T;
}
#endif

template<typename T>
void region::create_glue_(T* pSelf) {
    get_lua_().globals()[sLuaName_] = observer_from(pSelf);
}

} // namespace lxgui::gui

#endif
