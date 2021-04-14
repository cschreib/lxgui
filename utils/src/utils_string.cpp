#include "lxgui/utils_string.hpp"
#include "utf8.h"

#include <sstream>

/** \cond NOT_REMOVE_FROM_DOC
*/
namespace lxgui {
namespace utils
{
using string_stream = std::stringstream;

void trim(string& s, char cPattern)
{
    while (!s.empty() && s[0] == cPattern)
        s.erase(0, 1);

    while (!s.empty() && s.back() == cPattern)
        s.erase(s.size()-1, 1);
}

void trim(std::string& s, const std::string& sPatterns)
{
    size_t uiStart = s.find_first_not_of(sPatterns);
    if (uiStart == s.npos)
    {
        s.clear();
        return;
    }
    else
        s = s.erase(0, uiStart);

    size_t uiEnd = s.find_last_not_of(sPatterns);
    if (uiEnd != s.npos)
        s = s.erase(uiEnd+1);
}

void replace(string& s, const string& sPattern, const string& sReplacement)
{
    size_t uiPos = s.find(sPattern);

    while (uiPos != s.npos)
    {
        s.replace(uiPos, sPattern.length(), sReplacement);
        uiPos = s.find(sPattern, uiPos + sReplacement.length());
    }
}

uint count_occurrences(const string& s, const string& sPattern)
{
    uint uiCount = 0;
    size_t uiPos = s.find(sPattern);
    while (uiPos != s.npos)
    {
        ++uiCount;
        uiPos = s.find(sPattern, uiPos+1);
    }

    return uiCount;
}

bool has_no_content(const string& s)
{
    if (s.empty())
        return true;

    for (size_t i = 0; i < s.length(); ++i)
    {
        if (s[i] != ' ' && s[i] != '\t')
            return false;
    }

    return true;
}

bool starts_with(const string& s, const string& sPattern)
{
    size_t n = std::min(s.size(), sPattern.size());
    for (size_t i = 0; i < n; ++i)
    {
        if (s[i] != sPattern[i])
            return false;
    }

    return true;
}

bool ends_with(const string& s, const string& sPattern)
{
    size_t ss = s.size();
    size_t ps = sPattern.size();
    size_t n = std::min(ss, ps);
    for (size_t i = 1; i <= n; ++i)
    {
        if (s[ss-i] != sPattern[ps-i])
            return false;
    }

    return true;
}

ustring utf8_to_unicode(const string& s)
{
    return utf8::utf8to32(s);
}

string unicode_to_utf8(const ustring& s)
{
    return utf8::utf32to8(s);
}

uint hex_to_uint(const string& s)
{
    uint i = 0;
    string_stream ss;
    ss << s;
    ss >> std::hex >> i;
    return i;
}

int string_to_int(const string& s)
{
    int i = 0;
    string_stream ss(s);
    ss >> i;
    return i;
}

int string_to_int(const ustring& s)
{
    return string_to_int(unicode_to_utf8(s));
}

uint string_to_uint(const string& s)
{
    uint ui = 0;
    string_stream ss(s);
    ss >> ui;
    return ui;
}

uint string_to_uint(const ustring& s)
{
    return string_to_uint(unicode_to_utf8(s));
}

float string_to_float(const string& s)
{
    float d = 0;
    string_stream ss(s);
    ss >> d;
    return d;
}

float string_to_float(const ustring& s)
{
    return string_to_float(unicode_to_utf8(s));
}

double string_to_double(const string& s)
{
    double d = 0;
    string_stream ss(s);
    ss >> d;
    return d;
}

double string_to_double(const ustring& s)
{
    return string_to_double(unicode_to_utf8(s));
}

bool is_number(const string& s)
{
    string_stream mTemp(s);
    double dValue = 0;
    mTemp >> dValue;
    return !mTemp.fail();
}

bool is_number(const ustring& s)
{
    return is_number(unicode_to_utf8(s));
}

bool is_number(char s)
{
    return '0' <= s && s <= '9';
}

bool is_number(char32_t s)
{
    return U'0' <= s && s <= U'9';
}

bool string_to_bool(const string& s)
{
    return s == "true";
}

bool string_to_bool(const ustring& s)
{
    return s == U"true";
}

bool is_boolean(const string& s)
{
    return (s == "false") || (s == "true");
}

bool is_boolean(const ustring& s)
{
    return (s == U"false") || (s == U"true");
}

bool is_whitespace(char c)
{
    return c == '\n' || c == ' ' || c == '\t' || c == '\r';
}

bool is_whitespace(char32_t c)
{
    return c == U'\n' || c == U' ' || c == U'\t' || c == '\r';
}

template<typename T>
string value_to_string(T mValue)
{
    string_stream sStream;
    sStream << mValue;
    return sStream.str();
}

string to_string(int i)
{
    return value_to_string(i);
}

string to_string(uint i)
{
    return value_to_string(i);
}

string to_string(long i)
{
    return value_to_string(i);
}

string to_string(ulong ui)
{
    return value_to_string(ui);
}

string to_string(float f)
{
    return value_to_string(f);
}

string to_string(double f)
{
    return value_to_string(f);
}

string to_string(bool b)
{
    return b ? "true" : "false";
}

string to_string(void* p)
{
    string_stream sStream;
    sStream << p;
    return sStream.str();
}

std::string to_string(const utils::variant& mValue)
{
    return std::visit([&](const auto& mInnerValue) -> std::string
    {
        using inner_type = std::decay_t<decltype(mInnerValue)>;
        if constexpr (std::is_same_v<inner_type, utils::empty>)
            return "<none>";
        else if constexpr (std::is_same_v<inner_type, bool>)
            return to_string(mInnerValue);
        else
            return value_to_string(mInnerValue);
    }, mValue);
}
}
}
