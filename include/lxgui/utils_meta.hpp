#ifndef LXGUI_UTILS_META_HPP
#define LXGUI_UTILS_META_HPP

#include "lxgui/lxgui.hpp"

#include <type_traits>

namespace lxgui::utils {

namespace impl {
template<typename T>
struct first_function_argument_;

template<typename R, typename T, typename... Args>
struct first_function_argument_<R (*)(T, Args...)> {
    using type = T;
};

template<typename R, typename F, typename T, typename... Args>
struct first_function_argument_<R (F::*)(T, Args...)> {
    using type = T;
};

template<typename R, typename F, typename T, typename... Args>
struct first_function_argument_<R (F::*)(T, Args...) const> {
    using type = T;
};
} // namespace impl

template<typename T>
using first_function_argument =
    typename impl::first_function_argument_<decltype(&std::decay_t<T>::operator())>::type;

} // namespace lxgui::utils

#endif
