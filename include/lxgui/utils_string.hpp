#ifndef LXGUI_UTILS_STRING_HPP
#define LXGUI_UTILS_STRING_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/utils_variant.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <magic_enum.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

/** \cond INCLUDE_INTERNALS_IN_DOC
 */
namespace lxgui::utils {

using string       = std::string;
using string_view  = std::string_view;
using ustring      = std::u32string;
using ustring_view = std::u32string_view;

[[nodiscard]] string_view trim(string_view s, char c_pattern);
[[nodiscard]] string_view trim(string_view s, string_view patterns);

void replace(string& s, string_view pattern, string_view replacement);

[[nodiscard]] std::size_t count_occurrences(string_view s, string_view pattern);

[[nodiscard]] std::vector<string_view>  cut(string_view s, string_view delim);
[[nodiscard]] std::vector<ustring_view> cut(ustring_view s, ustring_view delim);

[[nodiscard]] std::vector<string_view>  cut_each(string_view s, string_view delim);
[[nodiscard]] std::vector<ustring_view> cut_each(ustring_view s, ustring_view delim);

[[nodiscard]] std::pair<string_view, string_view>   cut_first(string_view s, string_view delim);
[[nodiscard]] std::pair<ustring_view, ustring_view> cut_first(ustring_view s, ustring_view delim);

[[nodiscard]] bool starts_with(string_view s, string_view pattern);
[[nodiscard]] bool ends_with(string_view s, string_view pattern);

[[nodiscard]] bool has_no_content(string_view s);

[[nodiscard]] ustring utf8_to_unicode(string_view s);
[[nodiscard]] string  unicode_to_utf8(ustring_view s);

[[nodiscard]] std::size_t hex_to_uint(string_view s);

namespace impl {

template<typename T>
std::optional<T> from_string(string_view);

template<>
std::optional<int> from_string<int>(string_view);

template<>
std::optional<long> from_string<long>(string_view);

template<>
std::optional<long long> from_string<long long>(string_view);

template<>
std::optional<unsigned> from_string<unsigned>(string_view);

template<>
std::optional<unsigned long> from_string<unsigned long>(string_view);

template<>
std::optional<unsigned long long> from_string<unsigned long long>(string_view);

template<>
std::optional<float> from_string<float>(string_view);

template<>
std::optional<double> from_string<double>(string_view);

template<>
std::optional<bool> from_string<bool>(string_view);

template<>
std::optional<string> from_string<string>(string_view);

template<typename T>
std::optional<T> from_string(ustring_view);

template<>
std::optional<int> from_string<int>(ustring_view);

template<>
std::optional<long> from_string<long>(ustring_view);

template<>
std::optional<long long> from_string<long long>(ustring_view);

template<>
std::optional<unsigned> from_string<unsigned>(ustring_view);

template<>
std::optional<unsigned long> from_string<unsigned long>(ustring_view);

template<>
std::optional<unsigned long long> from_string<unsigned long long>(ustring_view);

template<>
std::optional<float> from_string<float>(ustring_view);

template<>
std::optional<double> from_string<double>(ustring_view);

template<>
std::optional<bool> from_string<bool>(ustring_view);

template<>
std::optional<ustring> from_string<ustring>(ustring_view);

} // namespace impl

template<typename T>
[[nodiscard]] std::optional<T> from_string(string_view s) {
    if constexpr (std::is_enum_v<T>) {
        return magic_enum::enum_cast<T>(s, magic_enum::case_insensitive);
    } else {
        return impl::from_string<T>(s);
    }
}

template<typename T>
[[nodiscard]] std::optional<T> from_string(ustring_view s) {
    if constexpr (std::is_enum_v<T>) {
        return magic_enum::enum_cast<T>(unicode_to_utf8(s), magic_enum::case_insensitive);
    } else {
        return impl::from_string<T>(s);
    }
}

[[nodiscard]] bool is_number(string_view s);
[[nodiscard]] bool is_number(ustring_view s);
[[nodiscard]] bool is_number(char s);
[[nodiscard]] bool is_number(char32_t s);
[[nodiscard]] bool is_integer(string_view s);
[[nodiscard]] bool is_integer(ustring_view s);
[[nodiscard]] bool is_integer(char s);
[[nodiscard]] bool is_integer(char32_t s);
[[nodiscard]] bool is_boolean(string_view s);
[[nodiscard]] bool is_boolean(ustring_view s);

[[nodiscard]] bool is_whitespace(char c);
[[nodiscard]] bool is_whitespace(char32_t c);

[[nodiscard]] string to_string(int v);
[[nodiscard]] string to_string(long v);
[[nodiscard]] string to_string(long long v);
[[nodiscard]] string to_string(unsigned v);
[[nodiscard]] string to_string(unsigned long v);
[[nodiscard]] string to_string(unsigned long long v);
[[nodiscard]] string to_string(float v);
[[nodiscard]] string to_string(double v);
[[nodiscard]] string to_string(bool v);
[[nodiscard]] string to_string(bool b);
[[nodiscard]] string to_string(const void* p);

template<typename T, typename enable = std::enable_if_t<std::is_enum_v<T>>>
[[nodiscard]] string to_string(T v) {
    return string{magic_enum::enum_name(v)};
}

template<typename T>
[[nodiscard]] string to_string(const T* p) {
    return p != nullptr ? to_string(static_cast<const void*>(p)) : "null";
}

template<typename T>
[[nodiscard]] string to_string(T* p) {
    return p != nullptr ? to_string(static_cast<void*>(p)) : "null";
}

[[nodiscard]] string to_string(const utils::variant& value);

template<typename... Args>
[[nodiscard]] ustring to_ustring(Args&&... args) {
    return utils::utf8_to_unicode(to_string(std::forward<Args>(args)...));
}

} // namespace lxgui::utils

/** \endcond
 */

#endif
