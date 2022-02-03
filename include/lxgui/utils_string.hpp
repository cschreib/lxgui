#ifndef LXGUI_UTILS_STRING_HPP
#define LXGUI_UTILS_STRING_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/utils_variant.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

/** \cond INCLUDE_INTERNALS_IN_DOC
 */
namespace lxgui { namespace utils {

using string       = std::string;
using string_view  = std::string_view;
using ustring      = std::u32string;
using ustring_view = std::u32string_view;

[[nodiscard]] string_view trim(string_view s, char cPattern);
[[nodiscard]] string_view trim(string_view s, string_view sPatterns);

void replace(string& s, string_view sPattern, string_view sReplacement);

[[nodiscard]] std::size_t count_occurrences(string_view s, string_view sPattern);

[[nodiscard]] std::vector<string_view>  cut(string_view s, string_view sDelim);
[[nodiscard]] std::vector<ustring_view> cut(ustring_view s, ustring_view sDelim);

[[nodiscard]] std::vector<string_view>  cut_each(string_view s, string_view sDelim);
[[nodiscard]] std::vector<ustring_view> cut_each(ustring_view s, ustring_view sDelim);

[[nodiscard]] std::pair<string_view, string_view>   cut_first(string_view s, string_view sDelim);
[[nodiscard]] std::pair<ustring_view, ustring_view> cut_first(ustring_view s, ustring_view sDelim);

[[nodiscard]] bool starts_with(string_view s, string_view sPattern);
[[nodiscard]] bool ends_with(string_view s, string_view sPattern);

[[nodiscard]] bool has_no_content(string_view s);

[[nodiscard]] ustring utf8_to_unicode(string_view s);
[[nodiscard]] string  unicode_to_utf8(ustring_view s);

[[nodiscard]] std::size_t hex_to_uint(string_view s);

bool from_string(string_view, int&);
bool from_string(string_view, long&);
bool from_string(string_view, long long&);
bool from_string(string_view, unsigned&);
bool from_string(string_view, unsigned long&);
bool from_string(string_view, unsigned long long&);
bool from_string(string_view, float&);
bool from_string(string_view, double&);
bool from_string(string_view, bool&);
bool from_string(string_view, string&);

bool from_string(ustring_view, int&);
bool from_string(ustring_view, long&);
bool from_string(ustring_view, long long&);
bool from_string(ustring_view, unsigned&);
bool from_string(ustring_view, unsigned long&);
bool from_string(ustring_view, unsigned long long&);
bool from_string(ustring_view, float&);
bool from_string(ustring_view, double&);
bool from_string(ustring_view, bool&);
bool from_string(ustring_view, ustring&);

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
[[nodiscard]] string to_string(void* p);

template<typename T>
[[nodiscard]] string to_string(T* p) {
    if (p != nullptr)
        return to_string(static_cast<void*>(p));
    else
        return "null";
}

[[nodiscard]] string to_string(const utils::variant& mValue);

template<typename... Args>
[[nodiscard]] ustring to_ustring(Args&&... args) {
    return utils::utf8_to_unicode(to_string(std::forward<Args>(args)...));
}

}} // namespace lxgui::utils

/** \endcond
 */

#endif
