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
    static T* get(lua_State*, const lxgui::utils::observer_ptr<T>& pointer) noexcept {
        return pointer.get();
    }

    static bool is_null(lua_State*, const lxgui::utils::observer_ptr<T>& pointer) noexcept {
        return pointer.expired();
    }
};

} // namespace sol
/** \endcond
 */

template<typename T>
void sol_lua_check_access(
    sol::types<T>, lua_State* lua, int index, sol::stack::record& /*tracking*/) {
    // NB: not sure why, but using tracking here leads to issues later on, so
    // ignore it for now.

    sol::optional<lxgui::utils::observer_ptr<T>&> optional =
        sol::stack::check_get<lxgui::utils::observer_ptr<T>&>(
            lua, index, sol::no_panic /*, tracking*/);

    if (!optional.has_value())
        return;

    if (optional->expired())
        throw sol::error("object has been deleted");
}

namespace lxgui::gui {

inline utils::observer_ptr<region>
get_object(manager& mgr, const std::variant<std::string, region*>& parent) {
    return std::visit(
        [&](const auto& value) -> utils::observer_ptr<region> {
            using data_type = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<data_type, std::string>) {
                if (utils::has_no_content(value))
                    return nullptr;

                auto parent = mgr.get_root().get_registry().get_region_by_name(value);
                if (!parent)
                    throw sol::error("no region with name \"" + value + "\"");

                return parent;
            } else {
                return observer_from(value);
            }
        },
        parent);
}

template<typename T>
utils::observer_ptr<T> get_object(manager& mgr, const std::variant<std::string, T*>& parent) {
    return std::visit(
        [&](const auto& value) -> utils::observer_ptr<T> {
            using data_type = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<data_type, std::string>) {
                if (utils::has_no_content(value))
                    return nullptr;

                auto parent_object = mgr.get_root().get_registry().get_region_by_name(value);
                if (!parent_object)
                    throw sol::error("no region with name \"" + value + "\"");

                auto parent = down_cast<T>(parent_object);
                if (!parent)
                    throw sol::error(
                        "region \"" + value + "\" is not a " + std::string(T::class_name));

                return parent;
            } else {
                return observer_from(value);
            }
        },
        parent);
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
        return [](T& self, Args... args) { return (self.*Function)(std::move(args)...); };
    }
};

template<typename R, typename T, typename... Args, R (T::*Function)(Args...) const>
struct member_function_holder<R (T::*)(Args...) const, Function> {
    static constexpr auto make_free_function() {
        return [](const T& self, Args... args) { return (self.*Function)(std::move(args)...); };
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
void region::create_glue_(T* self) {
    get_lua_().globals()[lua_name_] = observer_from(self);
}

} // namespace lxgui::gui

#endif
