#include "lxgui/utils_string.hpp"

#include "utf8.h"

#include <sstream>

/** \cond NOT_REMOVE_FROM_DOC
 */
namespace lxgui::utils {

string_view trim(string_view s, char c_pattern) {
    std::size_t start = s.find_first_not_of(c_pattern);
    if (start == s.npos)
        return {};

    s = s.substr(start);

    std::size_t end = s.find_last_not_of(c_pattern);
    if (end != s.npos)
        s = s.substr(0, end + 1);

    return s;
}

string_view trim(string_view s, string_view patterns) {
    std::size_t start = s.find_first_not_of(patterns);
    if (start == s.npos)
        return {};

    s = s.substr(start);

    std::size_t end = s.find_last_not_of(patterns);
    if (end != s.npos)
        s = s.substr(0, end + 1);

    return s;
}

void replace(string& s, string_view pattern, string_view replacement) {
    std::size_t pos = s.find(pattern);

    while (pos != s.npos) {
        s.replace(pos, pattern.length(), replacement);
        pos = s.find(pattern, pos + replacement.length());
    }
}

std::size_t count_occurrences(string_view s, string_view pattern) {
    std::size_t count = 0;
    std::size_t pos   = s.find(pattern);
    while (pos != s.npos) {
        ++count;
        pos = s.find(pattern, pos + 1);
    }

    return count;
}

template<typename T>
std::vector<std::basic_string_view<T>>
cut_template(std::basic_string_view<T> s, std::basic_string_view<T> delim) {
    std::vector<std::basic_string_view<T>> pieces;
    std::size_t                            pos      = s.find(delim);
    std::size_t                            last_pos = 0u;
    std::size_t                            cur_size = 0u;

    while (pos != std::basic_string_view<T>::npos) {
        cur_size = pos - last_pos;
        if (cur_size != 0)
            pieces.push_back(s.substr(last_pos, cur_size));
        last_pos = pos + delim.size();
        pos      = s.find(delim, last_pos);
    }

    pieces.push_back(s.substr(last_pos));

    return pieces;
}

std::vector<string_view> cut(string_view s, string_view delim) {
    return cut_template(s, delim);
}

std::vector<ustring_view> cut(ustring_view s, ustring_view delim) {
    return cut_template(s, delim);
}

template<typename T>
std::vector<std::basic_string_view<T>>
cut_each_template(std::basic_string_view<T> s, std::basic_string_view<T> delim) {
    std::vector<std::basic_string_view<T>> pieces;
    std::size_t                            pos      = s.find(delim);
    std::size_t                            last_pos = 0u;
    std::size_t                            cur_size = 0u;

    while (pos != std::basic_string_view<T>::npos) {
        cur_size = pos - last_pos;
        pieces.push_back(s.substr(last_pos, cur_size));
        last_pos = pos + delim.size();
        pos      = s.find(delim, last_pos);
    }

    pieces.push_back(s.substr(last_pos));

    return pieces;
}

std::vector<string_view> cut_each(string_view s, string_view delim) {
    return cut_each_template(s, delim);
}

std::vector<ustring_view> cut_each(ustring_view s, ustring_view delim) {
    return cut_each_template(s, delim);
}

template<typename T>
std::pair<std::basic_string_view<T>, std::basic_string_view<T>>
cut_first_template(std::basic_string_view<T> s, std::basic_string_view<T> delim) {
    std::size_t pos = s.find(delim);
    if (pos == std::basic_string_view<T>::npos)
        return {};

    return {s.substr(0, pos), s.substr(pos + 1u)};
}

std::pair<string_view, string_view> cut_first(string_view s, string_view delim) {
    return cut_first_template(s, delim);
}

std::pair<ustring_view, ustring_view> cut_first(ustring_view s, ustring_view delim) {
    return cut_first_template(s, delim);
}

bool has_no_content(string_view s) {
    if (s.empty())
        return true;

    for (std::size_t i = 0; i < s.length(); ++i) {
        if (s[i] != ' ' && s[i] != '\t')
            return false;
    }

    return true;
}

bool starts_with(string_view s, string_view pattern) {
    std::size_t n = std::min(s.size(), pattern.size());
    for (std::size_t i = 0; i < n; ++i) {
        if (s[i] != pattern[i])
            return false;
    }

    return true;
}

bool ends_with(string_view s, string_view pattern) {
    std::size_t ss = s.size();
    std::size_t ps = pattern.size();
    std::size_t n  = std::min(ss, ps);
    for (std::size_t i = 1; i <= n; ++i) {
        if (s[ss - i] != pattern[ps - i])
            return false;
    }

    return true;
}

ustring utf8_to_unicode(string_view s) {
    return utf8::utf8to32(s);
}

string unicode_to_utf8(ustring_view s) {
    return utf8::utf32to8(s);
}

std::size_t hex_to_uint(string_view s) {
    std::size_t        i = 0;
    std::istringstream ss{std::string(s)};
    ss.imbue(std::locale::classic());
    ss >> std::hex >> i;
    return i;
}

namespace impl {

template<typename T>
std::optional<T> from_string_template(const std::locale& loc, string_view s) {
    std::istringstream ss{std::string(s)};
    ss.imbue(loc);

    T v;
    ss >> v;

    if (!ss.fail()) {
        if (ss.eof())
            return v;

        std::string rem;
        ss >> rem;

        if (rem.find_first_not_of(" \t") == rem.npos)
            return v;
    }

    return {};
}

template<typename T>
std::optional<T> from_string_template(const std::locale& loc, ustring_view s) {
    return from_string_template<T>(loc, unicode_to_utf8(s));
}

// ----- locale, utf8 string

template<>
std::optional<int> from_string<int>(const std::locale& loc, string_view s) {
    return from_string_template<int>(loc, s);
}

template<>
std::optional<long> from_string<long>(const std::locale& loc, string_view s) {
    return from_string_template<long>(loc, s);
}

template<>
std::optional<long long> from_string<long long>(const std::locale& loc, string_view s) {
    return from_string_template<long long>(loc, s);
}

template<>
std::optional<unsigned> from_string<unsigned>(const std::locale& loc, string_view s) {
    return from_string_template<unsigned>(loc, s);
}

template<>
std::optional<unsigned long> from_string<unsigned long>(const std::locale& loc, string_view s) {
    return from_string_template<unsigned long>(loc, s);
}

template<>
std::optional<unsigned long long>
from_string<unsigned long long>(const std::locale& loc, string_view s) {
    return from_string_template<unsigned long long>(loc, s);
}

template<>
std::optional<float> from_string<float>(const std::locale& loc, string_view s) {
    return from_string_template<float>(loc, s);
}

template<>
std::optional<double> from_string<double>(const std::locale& loc, string_view s) {
    return from_string_template<double>(loc, s);
}

// ----- locale, utf32 string

template<>
std::optional<int> from_string<int>(const std::locale& loc, ustring_view s) {
    return from_string_template<int>(loc, s);
}

template<>
std::optional<long> from_string<long>(const std::locale& loc, ustring_view s) {
    return from_string_template<long>(loc, s);
}

template<>
std::optional<long long> from_string<long long>(const std::locale& loc, ustring_view s) {
    return from_string_template<long long>(loc, s);
}

template<>
std::optional<unsigned> from_string<unsigned>(const std::locale& loc, ustring_view s) {
    return from_string_template<unsigned>(loc, s);
}

template<>
std::optional<unsigned long> from_string<unsigned long>(const std::locale& loc, ustring_view s) {
    return from_string_template<unsigned long>(loc, s);
}

template<>
std::optional<unsigned long long>
from_string<unsigned long long>(const std::locale& loc, ustring_view s) {
    return from_string_template<unsigned long long>(loc, s);
}

template<>
std::optional<float> from_string<float>(const std::locale& loc, ustring_view s) {
    return from_string_template<float>(loc, s);
}

template<>
std::optional<double> from_string<double>(const std::locale& loc, ustring_view s) {
    return from_string_template<double>(loc, s);
}

// ----- C locale, utf8 string

template<>
std::optional<int> from_string<int>(string_view s) {
    return from_string_template<int>(std::locale::classic(), s);
}

template<>
std::optional<long> from_string<long>(string_view s) {
    return from_string_template<long>(std::locale::classic(), s);
}

template<>
std::optional<long long> from_string<long long>(string_view s) {
    return from_string_template<long long>(std::locale::classic(), s);
}

template<>
std::optional<unsigned> from_string<unsigned>(string_view s) {
    return from_string_template<unsigned>(std::locale::classic(), s);
}

template<>
std::optional<unsigned long> from_string<unsigned long>(string_view s) {
    return from_string_template<unsigned long>(std::locale::classic(), s);
}

template<>
std::optional<unsigned long long> from_string<unsigned long long>(string_view s) {
    return from_string_template<unsigned long long>(std::locale::classic(), s);
}

template<>
std::optional<float> from_string<float>(string_view s) {
    return from_string_template<float>(std::locale::classic(), s);
}

template<>
std::optional<double> from_string<double>(string_view s) {
    return from_string_template<double>(std::locale::classic(), s);
}

template<>
std::optional<bool> from_string<bool>(string_view s) {
    if (s == "true")
        return true;
    if (s == "false")
        return false;
    return {};
}

template<>
std::optional<string> from_string<string>(string_view s) {
    return string{s};
}

// ----- C locale, utf32 string

template<>
std::optional<int> from_string<int>(ustring_view s) {
    return from_string_template<int>(std::locale::classic(), s);
}

template<>
std::optional<long> from_string<long>(ustring_view s) {
    return from_string_template<long>(std::locale::classic(), s);
}

template<>
std::optional<long long> from_string<long long>(ustring_view s) {
    return from_string_template<long long>(std::locale::classic(), s);
}

template<>
std::optional<unsigned> from_string<unsigned>(ustring_view s) {
    return from_string_template<unsigned>(std::locale::classic(), s);
}

template<>
std::optional<unsigned long> from_string<unsigned long>(ustring_view s) {
    return from_string_template<unsigned long>(std::locale::classic(), s);
}

template<>
std::optional<unsigned long long> from_string<unsigned long long>(ustring_view s) {
    return from_string_template<unsigned long long>(std::locale::classic(), s);
}

template<>
std::optional<float> from_string<float>(ustring_view s) {
    return from_string_template<float>(std::locale::classic(), s);
}

template<>
std::optional<double> from_string<double>(ustring_view s) {
    return from_string_template<double>(std::locale::classic(), s);
}

template<>
std::optional<bool> from_string<bool>(ustring_view s) {
    if (s == U"true")
        return true;
    if (s == U"false")
        return false;
    return {};
}

template<>
std::optional<ustring> from_string<ustring>(ustring_view s) {
    return ustring{s};
}

} // namespace impl

bool is_number(const std::locale& loc, string_view s) {
    return impl::from_string<double>(loc, s).has_value();
}

bool is_number(const std::locale& loc, ustring_view s) {
    return impl::from_string<double>(loc, s).has_value();
}

bool is_integer(const std::locale& loc, string_view s) {
    return impl::from_string<std::int64_t>(loc, s).has_value();
}

bool is_integer(const std::locale& loc, ustring_view s) {
    return impl::from_string<std::int64_t>(loc, s).has_value();
}

bool is_number(string_view s) {
    return is_number(std::locale::classic(), s);
}

bool is_number(ustring_view s) {
    return is_number(std::locale::classic(), s);
}

bool is_integer(string_view s) {
    return is_integer(std::locale::classic(), s);
}

bool is_integer(ustring_view s) {
    return is_integer(std::locale::classic(), s);
}

bool is_number(char s) {
    return '0' <= s && s <= '9';
}

bool is_number(char32_t s) {
    return U'0' <= s && s <= U'9';
}

bool is_integer(char s) {
    return is_number(s);
}

bool is_integer(char32_t s) {
    return is_number(s);
}

bool is_boolean(string_view s) {
    return (s == "false") || (s == "true");
}

bool is_boolean(ustring_view s) {
    return (s == U"false") || (s == U"true");
}

bool is_whitespace(char c) {
    return c == '\n' || c == ' ' || c == '\t' || c == '\r';
}

bool is_whitespace(char32_t c) {
    return c == U'\n' || c == U' ' || c == U'\t' || c == '\r';
}

template<typename T>
string to_string_template(T value) {
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << value;
    return ss.str();
}

string to_string(int v) {
    return to_string_template(v);
}

string to_string(long v) {
    return to_string_template(v);
}

string to_string(long long v) {
    return to_string_template(v);
}

string to_string(unsigned v) {
    return to_string_template(v);
}

string to_string(unsigned long v) {
    return to_string_template(v);
}

string to_string(unsigned long long v) {
    return to_string_template(v);
}

string to_string(float v) {
    return to_string_template(v);
}

string to_string(double v) {
    return to_string_template(v);
}

string to_string(bool b) {
    return b ? "true" : "false";
}

string to_string(const void* p) {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << p;
    return stream.str();
}

std::string to_string(const utils::variant& value) {
    return std::visit(
        [&](const auto& inner_value) -> std::string {
            using inner_type = std::decay_t<decltype(inner_value)>;
            if constexpr (std::is_same_v<inner_type, utils::empty>)
                return "<none>";
            else
                return to_string(inner_value);
        },
        value);
}

} // namespace lxgui::utils

/** \endcond
 */
