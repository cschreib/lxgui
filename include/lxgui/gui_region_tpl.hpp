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
    static T* get(lua_State*, const lxgui::utils::observer_ptr<T>& p_pointer) noexcept {
        return p_pointer.get();
    }

    static bool is_null(lua_State*, const lxgui::utils::observer_ptr<T>& p_pointer) noexcept {
        return p_pointer.expired();
    }
};

} // namespace sol
/** \endcond
 */

template<typename T>
void sol_lua_check_access(
    sol::types<T>, lua_State* p_lua, int i_index, sol::stack::record& /*mTracking*/) {
    // NB: not sure why, but using mTracking here leads to issues later on, so
    // ignore it for now.

    sol::optional<lxgui::utils::observer_ptr<T>&> m_optional =
        sol::stack::check_get<lxgui::utils::observer_ptr<T>&>(
            p_lua, i_index, sol::no_panic /*, mTracking*/);

    if (!m_optional.has_value())
        return;

    if (m_optional->expired())
        throw sol::error("object has been deleted");
}

namespace lxgui::gui {

inline utils::observer_ptr<region>
get_object(manager& m_manager, const std::variant<std::string, region*>& m_parent) {
    return std::visit(
        [&](const auto& m_value) -> utils::observer_ptr<region> {
            using data_type = std::decay_t<decltype(m_value)>;
            if constexpr (std::is_same_v<data_type, std::string>) {
                if (utils::has_no_content(m_value))
                    return nullptr;

                auto p_parent = m_manager.get_root().get_registry().get_region_by_name(m_value);
                if (!p_parent)
                    throw sol::error("no region with name \"" + m_value + "\"");

                return p_parent;
            } else {
                return observer_from(m_value);
            }
        },
        m_parent);
}

template<typename T>
utils::observer_ptr<T>
get_object(manager& m_manager, const std::variant<std::string, T*>& m_parent) {
    return std::visit(
        [&](const auto& m_value) -> utils::observer_ptr<T> {
            using data_type = std::decay_t<decltype(m_value)>;
            if constexpr (std::is_same_v<data_type, std::string>) {
                if (utils::has_no_content(m_value))
                    return nullptr;

                auto p_parent_object =
                    m_manager.get_root().get_registry().get_region_by_name(m_value);
                if (!p_parent_object)
                    throw sol::error("no region with name \"" + m_value + "\"");

                auto p_parent = down_cast<T>(p_parent_object);
                if (!p_parent)
                    throw sol::error(
                        "region \"" + m_value + "\" is not a " + std::string(T::class_name));

                return p_parent;
            } else {
                return observer_from(m_value);
            }
        },
        m_parent);
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
void region::create_glue_(T* p_self) {
    get_lua_().globals()[s_lua_name_] = observer_from(p_self);
}

} // namespace lxgui::gui

#endif
