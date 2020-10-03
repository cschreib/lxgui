#ifndef LXGUI_UTILS_VARIANT_HPP
#define LXGUI_UTILS_VARIANT_HPP

#include "lxgui/utils.hpp"

#include <utility>
#include <variant>
#include <string>
#include <type_traits>

namespace lxgui {
namespace utils {

    struct empty {};

    using variant = std::variant<
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
        std::string,
        empty>;

    template<typename T>
    T& get(variant& vValue)
    {
        if constexpr (std::is_enum_v<T>)
            return reinterpret_cast<const T&>(std::get<std::underlying_type_t<T>>(vValue));
        else
            return std::get<T>(vValue);
    }

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
