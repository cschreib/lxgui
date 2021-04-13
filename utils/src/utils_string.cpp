#include "lxgui/utils_string.hpp"

#include <sstream>
#include <cmath>
#include <locale>
#include <codecvt>

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
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
    return cvt.from_bytes(s);
}

string unicode_to_utf8(const ustring& s)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
    return cvt.to_bytes(s);
}

uint hex_to_uint(const string& s)
{
    uint i;
    string_stream ss;
    ss << s;
    ss >> std::hex >> i;
    return i;
}

long string_to_int(const string& s)
{
    long i;
    string_stream ss(s);
    ss >> i;
    return i;
}

long string_to_int(const ustring& s)
{
    return string_to_int(unicode_to_utf8(s));
}

unsigned long string_to_uint(const string& s)
{
    unsigned long ui;
    string_stream ss(s);
    ss >> ui;
    return ui;
}

unsigned long string_to_uint(const ustring& s)
{
    return string_to_uint(unicode_to_utf8(s));
}

double string_to_float(const string& s)
{
    double d;
    string_stream ss(s);
    ss >> d;
    return d;
}

double string_to_float(const ustring& s)
{
    return string_to_float(unicode_to_utf8(s));
}

template<typename T>
string value_to_string(T mValue)
{
    string_stream sStream;
    sStream << mValue;
    return sStream.str();
}

bool is_number(const string& s)
{
    string_stream mTemp(s);
    double dValue;
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
    if (s == "true")
        return true;
    else
        return false;
}

bool string_to_bool(const ustring& s)
{
    if (s == U"true")
        return true;
    else
        return false;
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

string to_string(int i)
{
    return value_to_string(i);
}

string to_string(int i, uint uiCharNbr)
{
    string sReturn;

    if (i >= 0)
    {
        sReturn = to_string(i);

        while (sReturn.length() < uiCharNbr)
            sReturn = '0' + sReturn;
    }
    else
    {
        sReturn = to_string(-i);

        while (sReturn.length() < uiCharNbr)
            sReturn = '0' + sReturn;

        sReturn = '-' + sReturn;
    }

    return sReturn;
}

string to_string(uint ui)
{
    return value_to_string(ui);
}

string to_string(uint ui, uint uiCharNbr)
{
    string sReturn = to_string(ui);

    while (sReturn.length() < uiCharNbr)
        sReturn = '0' + sReturn;

    return sReturn;
}

string to_string(long i)
{
    return value_to_string(i);
}

string to_string(long i, uint uiCharNbr)
{
    string sReturn;

    if (i >= 0)
    {
        sReturn = to_string(i);

        while (sReturn.length() < uiCharNbr)
            sReturn = '0' + sReturn;
    }
    else
    {
        sReturn = to_string(-i);

        while (sReturn.length() < uiCharNbr)
            sReturn = '0' + sReturn;

        sReturn = '-' + sReturn;
    }

    return sReturn;
}

string to_string(ulong ui)
{
    return value_to_string(ui);
}

string to_string(ulong ui, uint uiCharNbr)
{
    string sReturn = to_string(ui);

    while (sReturn.length() < uiCharNbr)
        sReturn = '0' + sReturn;

    return sReturn;
}

string to_string(float f)
{
    return value_to_string(f);
}

string to_string(float f, uint uiDigitNbr)
{
    string sReturn;

    float fTemp = f;
    if (fTemp < 0.0f)
    {
        sReturn.push_back('-');
        fTemp *= -1.0f;
    }

    if (fTemp < 1.0f)
    {
        sReturn += "0.";

        uint uiZeroes = 0;

        do
        {
            fTemp *= 10.0f;
            ++uiZeroes;
        }
        while (fTemp < 1.0f);

        sReturn.append(uiZeroes, '0');

        for (uint i = 0; i < uiDigitNbr; ++i)
            fTemp *= 10.0f;

        sReturn += to_string((uint)fTemp);
    }
    else
    {
        float fTemp2 = floor(fTemp);
        uint uiInt = (uint)fTemp2;
        sReturn += to_string(uiInt);

        uint uiSize = sReturn.size();
        if (sReturn[0] == '-')
            --uiSize;

        if (uiSize > uiDigitNbr)
        {
            uint uiExcess = uiSize - uiDigitNbr;

            sReturn.erase(sReturn.length() - uiExcess, uiExcess);
            sReturn.append(uiExcess, '0');
            return sReturn;
        }

        sReturn.push_back('.');

        uint uiRemaining = uiDigitNbr - uiSize;

        fTemp2 = fTemp - fTemp2;
        for (uint i = 0; i < uiRemaining; ++i)
            fTemp2 *= 10.0f;

        sReturn += to_string((uint)fTemp2, uiRemaining);
    }

    return sReturn;
}

string to_string(float f, uint uiIntCharNbr, uint uiFracCharNbr)
{
    string sReturn;

    float fTemp = f;
    if (fTemp < 0.0f)
    {
        sReturn.push_back('-');
        fTemp *= -1.0f;
    }

    float fTemp2 = floor(fTemp);

    sReturn = to_string((uint)fTemp2, uiIntCharNbr);

    if (uiFracCharNbr > 0)
    {
        fTemp2 = fTemp - fTemp2;
        for (uint i = 0; i < uiFracCharNbr; ++i)
            fTemp2 *= 10.0f;

        sReturn.push_back('.');
        sReturn += to_string((uint)fTemp2, uiFracCharNbr);
    }

    return sReturn;
}

string to_string(double f)
{
    return value_to_string(f);
}

string to_string(double f, uint uiDigitNbr)
{
    string sReturn;

    double fTemp = f;
    if (fTemp < 0.0)
    {
        sReturn.push_back('-');
        fTemp *= -1.0;
    }

    if (fTemp < 1.0)
    {
        sReturn += "0.";

        uint uiZeroes = 0;

        do
        {
            fTemp *= 10.0;
            ++uiZeroes;
        }
        while (fTemp < 1.0);

        sReturn.append(uiZeroes, '0');

        for (uint i = 0; i < uiDigitNbr; ++i)
            fTemp *= 10.0;

        sReturn += to_string((uint)fTemp);
    }
    else
    {
        double fTemp2 = floor(fTemp);
        uint uiInt = (uint)fTemp2;
        sReturn += to_string(uiInt);

        uint uiSize = sReturn.size();
        if (sReturn[0] == '-')
            --uiSize;

        if (uiSize > uiDigitNbr)
        {
            uint uiExcess = uiSize - uiDigitNbr;

            sReturn.erase(sReturn.length() - uiExcess, uiExcess);
            sReturn.append(uiExcess, '0');
            return sReturn;
        }

        sReturn.push_back('.');

        uint uiRemaining = uiDigitNbr - uiSize;

        fTemp2 = fTemp - fTemp2;
        for (uint i = 0; i < uiRemaining; ++i)
            fTemp2 *= 10.0;

        sReturn += to_string((uint)fTemp2, uiRemaining);
    }

    return sReturn;
}

string to_string(double f, uint uiIntCharNbr, uint uiFracCharNbr)
{
    string sReturn;

    double fTemp = f;
    if (fTemp < 0.0)
    {
        sReturn.push_back('-');
        fTemp *= -1.0;
    }

    double fTemp2 = floor(fTemp);

    sReturn = to_string((uint)fTemp2, uiIntCharNbr);

    if (uiFracCharNbr > 0)
    {
        fTemp2 = fTemp - fTemp2;
        for (uint i = 0; i < uiFracCharNbr; ++i)
            fTemp2 *= 10.0;

        sReturn.push_back('.');
        sReturn += to_string((uint)fTemp2, uiFracCharNbr);
    }

    return sReturn;
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
