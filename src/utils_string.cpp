#include "lxgui/utils_string.hpp"

#include "utf8.h"

#include <sstream>

/** \cond NOT_REMOVE_FROM_DOC
 */
namespace lxgui::utils {

string_view trim(string_view s, char c_pattern) {
    std::size_t ui_start = s.find_first_not_of(c_pattern);
    if (ui_start == s.npos)
        return {};

    s = s.substr(ui_start);

    std::size_t ui_end = s.find_last_not_of(c_pattern);
    if (ui_end != s.npos)
        s = s.substr(0, ui_end + 1);

    return s;
}

string_view trim(string_view s, string_view patterns) {
    std::size_t ui_start = s.find_first_not_of(patterns);
    if (ui_start == s.npos)
        return {};

    s = s.substr(ui_start);

    std::size_t ui_end = s.find_last_not_of(patterns);
    if (ui_end != s.npos)
        s = s.substr(0, ui_end + 1);

    return s;
}

void replace(string& s, string_view pattern, string_view replacement) {
    std::size_t ui_pos = s.find(pattern);

    while (ui_pos != s.npos) {
        s.replace(ui_pos, pattern.length(), replacement);
        ui_pos = s.find(pattern, ui_pos + replacement.length());
    }
}

std::size_t count_occurrences(string_view s, string_view pattern) {
    std::size_t ui_count = 0;
    std::size_t ui_pos   = s.find(pattern);
    while (ui_pos != s.npos) {
        ++ui_count;
        ui_pos = s.find(pattern, ui_pos + 1);
    }

    return ui_count;
}

template<typename T>
std::vector<std::basic_string_view<T>>
cut_template(std::basic_string_view<T> s, std::basic_string_view<T> delim) {
    std::vector<std::basic_string_view<T>> pieces;
    std::size_t                            ui_pos      = s.find(delim);
    std::size_t                            ui_last_pos = 0u;
    std::size_t                            ui_cur_size = 0u;

    while (ui_pos != std::basic_string_view<T>::npos) {
        ui_cur_size = ui_pos - ui_last_pos;
        if (ui_cur_size != 0)
            pieces.push_back(s.substr(ui_last_pos, ui_cur_size));
        ui_last_pos = ui_pos + delim.size();
        ui_pos      = s.find(delim, ui_last_pos);
    }

    pieces.push_back(s.substr(ui_last_pos));

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
    std::size_t                            ui_pos      = s.find(delim);
    std::size_t                            ui_last_pos = 0u;
    std::size_t                            ui_cur_size = 0u;

    while (ui_pos != std::basic_string_view<T>::npos) {
        ui_cur_size = ui_pos - ui_last_pos;
        pieces.push_back(s.substr(ui_last_pos, ui_cur_size));
        ui_last_pos = ui_pos + delim.size();
        ui_pos      = s.find(delim, ui_last_pos);
    }

    pieces.push_back(s.substr(ui_last_pos));

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
    std::size_t ui_pos = s.find(delim);
    if (ui_pos == std::basic_string_view<T>::npos)
        return {};

    return {s.substr(0, ui_pos), s.substr(ui_pos + 1u)};
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

template<typename T>
bool from_string_template(string_view s, T& v) {
    std::istringstream ss{std::string(s)};
    ss.imbue(std::locale::classic());
    ss >> v;

    if (!ss.fail()) {
        if (ss.eof())
            return true;

        std::string rem;
        ss >> rem;

        return rem.find_first_not_of(" \t") == rem.npos;
    }

    return false;
}

bool from_string(string_view s, int& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, long& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, long long& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, unsigned& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, unsigned long& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, unsigned long long& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, float& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, double& v) {
    return from_string_template(s, v);
}

bool from_string(string_view s, bool& v) {
    v = s == "true";
    return v || s == "false";
}

bool from_string(string_view s, string& v) {
    v = s;
    return true;
}

template<typename T>
bool from_string_template(ustring_view s, T& v) {
    return from_string(unicode_to_utf8(s), v);
}

bool from_string(ustring_view s, int& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, long& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, long long& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, unsigned& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, unsigned long& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, unsigned long long& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, float& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, double& v) {
    return from_string_template(s, v);
}

bool from_string(ustring_view s, bool& v) {
    v = s == U"true";
    return v || s == U"false";
}

bool from_string(ustring_view s, ustring& v) {
    v = s;
    return true;
}

bool is_number(string_view s) {
    std::istringstream temp{std::string(s)};
    temp.imbue(std::locale::classic());

    double d_value = 0;
    temp >> d_value;

    return !temp.fail();
}

bool is_number(ustring_view s) {
    return is_number(unicode_to_utf8(s));
}

bool is_number(char s) {
    return '0' <= s && s <= '9';
}

bool is_number(char32_t s) {
    return U'0' <= s && s <= U'9';
}

bool is_integer(string_view s) {
    std::istringstream temp{std::string(s)};
    temp.imbue(std::locale::classic());

    std::int64_t value = 0;
    temp >> value;

    return !temp.fail();
}

bool is_integer(ustring_view s) {
    return is_integer(unicode_to_utf8(s));
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

string to_string(void* p) {
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
