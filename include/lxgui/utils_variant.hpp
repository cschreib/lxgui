#ifndef LXGUI_UTILS_VARIANT_HPP
#define LXGUI_UTILS_VARIANT_HPP

#include "lxgui/utils.hpp"

#include <utility>
#include <variant>
#include <string>
#include <type_traits>

namespace lxgui {
namespace utils {

    /// Empty type, used in the implementation of utils::variant
    struct empty {};

    /// Type-erased value for passing arguments to events
    /** \note This contains all the basic numerical types up to 64bit precision.
    *         Any other scalar types (such as charXX_t, or strongly typed enums)
    *         can be represented as one of these. Use utils::get() to enable
    *         automatic support for strongly typed enums.
    *   \note The utils::empty type is used to indicate an un-specified value.
    *         This is also the state of a default constructed utils::variant.
    *         When transferred to Lua, utils::empty is translated as nil.
    */
    using variant = std::variant<
        empty,
        bool,
        std::int8_t,
        std::int16_t,
        std::int32_t,
        std::int64_t,
        std::uint8_t,
        std::uint16_t,
        std::uint32_t,
        std::uint64_t,
        float,
        double,
        std::string>;

    /// Retreive the value stored in an utils::variant
    /** \param vValue The variant
    *   \return The stored value, or throws std::bad_variant_access if the type was incorrect
    *   \note This wrapper around std::get() enables automatic support for strongly typed enums.
    */
    template<typename T>
    T& get(variant& vValue)
    {
        if constexpr (std::is_enum_v<T>)
            return reinterpret_cast<T&>(std::get<std::underlying_type_t<T>>(vValue));
        else
            return std::get<T>(vValue);
    }

    /// Retreive the value stored in an utils::variant
    /** \param vValue The variant
    *   \return The stored value, or throws std::bad_variant_access if the type was incorrect
    *   \note This wrapper around std::get() enables automatic support for strongly typed enums.
    */
    template<typename T>
    const T& get(const variant& vValue)
    {
        if constexpr (std::is_enum_v<T>)
            return reinterpret_cast<const T&>(std::get<std::underlying_type_t<T>>(vValue));
        else
            return std::get<T>(vValue);
    }
}
}

#endif
