#ifndef LXGUI_UTILS_VARIANT_HPP
#define LXGUI_UTILS_VARIANT_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace lxgui::utils {

/// Empty type, used in the implementation of utils::variant
struct empty {};

/// Type-erased value for passing arguments to events
/** \note This contains all the basic numerical types up to 64bit precision.
 *       Any other scalar types (such as charXX_t, or strongly typed enums)
 *       can be represented as one of these. Use utils::get() to enable
 *       automatic support for strongly typed enums.
 * \note The utils::empty type is used to indicate an un-specified value.
 *       This is also the state of a default constructed utils::variant.
 *       When transferred to Lua, utils::empty is translated as nil.
 */
using variant = std::variant<
    empty,
    bool,
    std::int64_t,
    std::int32_t,
    std::int16_t,
    std::int8_t,
    std::uint64_t,
    std::uint32_t,
    std::uint16_t,
    std::uint8_t,
    double,
    float,
    std::string>;

/// Retreive the value stored in an utils::variant
/** \param value The variant
 * \return The stored value, or throws std::bad_variant_access if the type was incorrect
 * \note This wrapper around std::get() enables automatic support for strongly typed enums.
 */
template<typename T>
T& get(variant& value) {
    if constexpr (std::is_enum_v<T>)
        return reinterpret_cast<T&>(std::get<std::underlying_type_t<T>>(value));
    else
        return std::get<T>(value);
}

/// Retreive the value stored in an utils::variant
/** \param value The variant
 * \return The stored value, or throws std::bad_variant_access if the type was incorrect
 * \note This wrapper around std::get() enables automatic support for strongly typed enums.
 */
template<typename T>
const T& get(const variant& value) {
    if constexpr (std::is_enum_v<T>)
        return reinterpret_cast<const T&>(std::get<std::underlying_type_t<T>>(value));
    else
        return std::get<T>(value);
}

} // namespace lxgui::utils

#endif
