#include "lxgui/utils_string.hpp"

#include <sstream>
#include <cmath>

/** \cond NOT_REMOVE_FROM_DOC
*/
namespace lxgui {
namespace utils
{
typedef std::stringstream string_stream;

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

bool has_no_content(const std::string& s)
{
    if (s.empty())
        return true;

    for (size_t i = 0; i < s.length(); ++i)
    {
        if (s[i] != ' ' && s[i] != '	')
            return false;
    }

    return true;
}

bool starts_with(const string& s, const string& sPattern)
{
    size_t n = std::min(s.size(), sPattern.size());
    for (size_t i = 0; i < n; ++i)
        if (s[i] != sPattern[i]) return false;

    return true;
}

bool ends_with(const string& s, const string& sPattern)
{
    size_t ss = s.size();
    size_t ps = sPattern.size();
    size_t n = std::min(ss, ps);
    for (size_t i = 1; i <= n; ++i)
        if (s[ss-i] != sPattern[ps-i]) return false;

    return true;
}

ustring UTF8_to_unicode(const string& s)
{
    static unsigned char MAX_ANSI = 127;
    static unsigned char ESC_C2   = 194;
    static unsigned char ESC_C3   = 195;

    ustring sResult;

    unsigned char cEscape = 0;

    for (unsigned char c : s)
    {
        if (c <= MAX_ANSI)
        {
            sResult.push_back(c);
        }
        else
        {
            if (c == ESC_C2 || c == ESC_C3)
            {
                cEscape = c;
                continue;
            }

            if (cEscape != 0)
            {
                if (cEscape == ESC_C3)
                {
                    // 192 : offset of for "c3" (195) escaped characters (accentuated)
                    // 128 : start offset for these characters (the first one is "c3 80")
                    sResult.push_back(192 + c - 128);
                }
                else if (cEscape == ESC_C2)
                {
                    // 192 : offset of for "c2" (194) escaped characters (misc)
                    // 128 : start offset for these characters (the first one is "c2 80")
                    sResult.push_back(128 + c - 128);
                }
            }
        }
    }

    return sResult;
}

char32_t UTF8_to_unicode(char c)
{
    static unsigned char MAX_ANSI = 127;

    unsigned char uc = (unsigned char)c;

    if (uc <= MAX_ANSI)
        return uc;
    else
        return 0u;
}

string unicode_to_UTF8(const ustring& s)
{
    static uint MAX_ANSI = 127;
    static uint ESC_C2   = 194;
    static uint ESC_C3   = 195;

    string sResult;

    for (auto c : s)
    {
        if (c <= MAX_ANSI)
        {
            sResult.push_back(c);
        }
        else
        {
            if (c < 192)
            {
                sResult.push_back((unsigned char)ESC_C2);
                sResult.push_back((unsigned char)(128 + c - 128));
            }
            else
            {
                sResult.push_back((unsigned char)ESC_C3);
                sResult.push_back((unsigned char)(128 + c - 192));
            }
        }
    }

    return sResult;
}

char unicode_to_UTF8(char32_t c)
{
    static uint MAX_ANSI = 127;
    if (c <= MAX_ANSI)
        return (unsigned char)c;
    else
        return '\0';
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
    return string_to_int(unicode_to_UTF8(s));
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
    return string_to_uint(unicode_to_UTF8(s));
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
    return string_to_float(unicode_to_UTF8(s));
}

string int_to_string(const long& i)
{
    string_stream sStream;
    sStream << i;
    return sStream.str();
}

string uint_to_string(const unsigned long& ui)
{
    string_stream sStream;
    sStream << ui;
    return sStream.str();
}

string float_to_string(float f)
{
    string_stream sStream;
    //sStream.precision(s_float_t<T>::DIGIT);
    sStream << f;
    return sStream.str();
}

string double_to_string(double d)
{
    string_stream sStream;
    //sStream.precision(s_float_t<T>::DIGIT);
    sStream << d;
    return sStream.str();
}

bool is_number(const std::string& s)
{
    string_stream mTemp(s);
    double dValue;
    mTemp >> dValue;
    return !mTemp.fail();
}

bool is_number(char s)
{
    return '0' <= s && s <= '9';
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
    if (s == UTF8_to_unicode("true"))
        return true;
    else
        return false;
}

bool is_boolean(const std::string& s)
{
    return (s == "false") || (s == "true");
}

string to_string(const string& s, uint uiNbr)
{
    string sReturn;

    for (uint uiCounter = 0; uiCounter < uiNbr; ++uiCounter)
        sReturn += s;

    return sReturn;
}

string to_string(const char* s)
{
    return s;
}

string to_string(const char* s, uint uiNbr)
{
    string sReturn;

    for (uint uiCounter = 0; uiCounter < uiNbr; ++uiCounter)
        sReturn += s;

    return sReturn;
}

string to_string(char* s)
{
    return s;
}

string to_string(char* s, uint uiNbr)
{
    string sReturn;

    for (uint uiCounter = 0; uiCounter < uiNbr; ++uiCounter)
        sReturn += s;

    return sReturn;
}

string to_string(char c)
{
    string s;
    s.push_back(c);
    return s;
}

string to_string(char c, uint uiNbr)
{
    return string(uiNbr, c);
}

string to_string(int i)
{
    return int_to_string(i);
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
    return uint_to_string(ui);
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
    return int_to_string(i);
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
    return uint_to_string(ui);
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
    return float_to_string(f);
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
    return double_to_string(f);
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
}
}
