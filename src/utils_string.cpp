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
    std::size_t uiStart = s.find_first_not_of(sPatterns);
    if (uiStart == s.npos)
    {
        s.clear();
        return;
    }
    else
        s = s.erase(0, uiStart);

    std::size_t uiEnd = s.find_last_not_of(sPatterns);
    if (uiEnd != s.npos)
        s = s.erase(uiEnd+1);
}

void replace(string& s, const string& sPattern, const string& sReplacement)
{
    std::size_t uiPos = s.find(sPattern);

    while (uiPos != s.npos)
    {
        s.replace(uiPos, sPattern.length(), sReplacement);
        uiPos = s.find(sPattern, uiPos + sReplacement.length());
    }
}

uint count_occurrences(const string& s, const string& sPattern)
{
    uint uiCount = 0;
    std::size_t uiPos = s.find(sPattern);
    while (uiPos != s.npos)
    {
        ++uiCount;
        uiPos = s.find(sPattern, uiPos+1);
    }

    return uiCount;
}

template<typename T>
std::vector<std::basic_string<T>> cut_template(const std::basic_string<T>& s,
    const std::basic_string<T>& sDelim)
{
    std::vector<std::basic_string<T>> lPieces;
    std::size_t uiPos = s.find(sDelim);
    std::size_t uiLastPos = 0u;
    std::size_t uiCurSize = 0u;

    while (uiPos != std::basic_string<T>::npos)
    {
        uiCurSize = uiPos - uiLastPos;
        if (uiCurSize != 0)
            lPieces.push_back(s.substr(uiLastPos, uiCurSize));
        uiLastPos = uiPos + sDelim.size();
        uiPos = s.find(sDelim, uiLastPos);
    }

    lPieces.push_back(s.substr(uiLastPos));

    return lPieces;
}

std::vector<string> cut(const string& s, const string& sDelim)
{
    return cut_template(s, sDelim);
}

std::vector<ustring> cut(const ustring& s, const ustring& sDelim)
{
    return cut_template(s, sDelim);
}

template<typename T>
std::vector<std::basic_string<T>> cut_each_template(const std::basic_string<T>& s,
    const std::basic_string<T>& sDelim)
{
    std::vector<std::basic_string<T>> lPieces;
    std::size_t uiPos = s.find(sDelim);
    std::size_t uiLastPos = 0u;
    std::size_t uiCurSize = 0u;

    while (uiPos != std::basic_string<T>::npos)
    {
        uiCurSize = uiPos - uiLastPos;
        lPieces.push_back(s.substr(uiLastPos, uiCurSize));
        uiLastPos = uiPos + sDelim.size();
        uiPos = s.find(sDelim, uiLastPos);
    }

    lPieces.push_back(s.substr(uiLastPos));

    return lPieces;
}

std::vector<string> cut_each(const string& s, const string& sDelim)
{
    return cut_each_template(s, sDelim);
}

std::vector<ustring> cut_each(const ustring& s, const ustring& sDelim)
{
    return cut_each_template(s, sDelim);
}

template<typename T>
std::pair<std::basic_string<T>,std::basic_string<T>> cut_first_template(
    const std::basic_string<T>& s, const std::basic_string<T>& sDelim)
{
    std::size_t uiPos = s.find(sDelim);
    if (uiPos == std::basic_string<T>::npos)
        return {};

    return {s.substr(0, uiPos), s.substr(uiPos + 1u)};
}

std::pair<string,string> cut_first(const string& s, const string& sDelim)
{
    return cut_first_template(s, sDelim);
}

std::pair<ustring,ustring> cut_first(const ustring& s, const ustring& sDelim)
{
    return cut_first_template(s, sDelim);
}

bool has_no_content(const string& s)
{
    if (s.empty())
        return true;

    for (std::size_t i = 0; i < s.length(); ++i)
    {
        if (s[i] != ' ' && s[i] != '\t')
            return false;
    }

    return true;
}

bool starts_with(const string& s, const string& sPattern)
{
    std::size_t n = std::min(s.size(), sPattern.size());
    for (std::size_t i = 0; i < n; ++i)
    {
        if (s[i] != sPattern[i])
            return false;
    }

    return true;
}

bool ends_with(const string& s, const string& sPattern)
{
    std::size_t ss = s.size();
    std::size_t ps = sPattern.size();
    std::size_t n = std::min(ss, ps);
    for (std::size_t i = 1; i <= n; ++i)
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

template<typename T>
bool from_string_template(const string& s, T& v)
{
    std::istringstream ss(s);
    ss >> v;

    if (!ss.fail())
    {
        if (ss.eof())
            return true;

        std::string rem;
        ss >> rem;

        return rem.find_first_not_of(" \t") == rem.npos;
    }

    return false;
}

bool from_string(const string& s, int& v)
{
    return from_string_template(s, v);
}

bool from_string(const string& s, long& v)
{
    return from_string_template(s, v);
}

bool from_string(const string& s, long long& v)
{
    return from_string_template(s, v);
}

bool from_string(const string& s, unsigned& v)
{
    return from_string_template(s, v);
}

bool from_string(const string& s, unsigned long& v)
{
    return from_string_template(s, v);
}

bool from_string(const string& s, unsigned long long& v)
{
    return from_string_template(s, v);
}

bool from_string(const string& s, float& v)
{
    return from_string_template(s, v);
}

bool from_string(const string& s, double& v)
{
    return from_string_template(s, v);
}

bool from_string(const string& s, bool& v)
{
    v = s == "true";
    return v || s == "false";
}

bool from_string(const string& s, string& v)
{
    v = s;
    return true;
}

template<typename T>
bool from_string_template(const ustring& s, T& v)
{
    return from_string(unicode_to_utf8(s), v);
}

bool from_string(const ustring& s, int& v)
{
    return from_string_template(s, v);
}

bool from_string(const ustring& s, long& v)
{
    return from_string_template(s, v);
}

bool from_string(const ustring& s, long long& v)
{
    return from_string_template(s, v);
}

bool from_string(const ustring& s, unsigned& v)
{
    return from_string_template(s, v);
}

bool from_string(const ustring& s, unsigned long& v)
{
    return from_string_template(s, v);
}

bool from_string(const ustring& s, unsigned long long& v)
{
    return from_string_template(s, v);
}

bool from_string(const ustring& s, float& v)
{
    return from_string_template(s, v);
}

bool from_string(const ustring& s, double& v)
{
    return from_string_template(s, v);
}

bool from_string(const ustring& s, bool& v)
{
    v = s == U"true";
    return v || s == U"false";
}

bool from_string(const ustring& s, ustring& v)
{
    v = s;
    return true;
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

bool is_integer(const string& s)
{
    string_stream mTemp(s);
    std::int64_t iValue = 0;
    mTemp >> iValue;
    return !mTemp.fail();
}

bool is_integer(const ustring& s)
{
    return is_integer(unicode_to_utf8(s));
}

bool is_integer(char s)
{
    return is_number(s);
}

bool is_integer(char32_t s)
{
    return is_number(s);
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
string to_string_template(T mValue)
{
    return std::to_string(mValue);
}

string to_string(int v)
{
    return to_string_template(v);
}

string to_string(long v)
{
    return to_string_template(v);
}

string to_string(long long v)
{
    return to_string_template(v);
}

string to_string(unsigned v)
{
    return to_string_template(v);
}

string to_string(unsigned long v)
{
    return to_string_template(v);
}

string to_string(unsigned long long v)
{
    return to_string_template(v);
}

string to_string(float v)
{
    return to_string_template(v);
}

string to_string(double v)
{
    return to_string_template(v);
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
        else
            return to_string(mInnerValue);
    }, mValue);
}
}
}
