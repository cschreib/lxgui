#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#include "lxgui/utils.hpp"
#include <string>
#include <vector>
#include <deque>
#include <array>
#include <iostream>
#include <algorithm>

/** \cond NOT_REMOVE_FROM_DOC
*/
namespace utils
{
typedef std::string                 string;
typedef std::basic_string<char32_t> ustring;

void trim(string& s, char cPattern);
void trim(string& s, const string& sPatterns);
void replace(string& s, const string& sPattern, const string& sReplacement);

uint count_occurrences(const string& s, const string& sPattern);

template<class T>
std::vector<std::basic_string<T>> cut(const std::basic_string<T>& s, const std::basic_string<T>& sDelim)
{
    std::vector<std::basic_string<T>> lPieces;
    size_t uiPos = s.find(sDelim);
    size_t uiLastPos = 0u;
    size_t uiCurSize = 0u;

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

template<class T>
std::vector<std::basic_string<T>> cut(const std::basic_string<T>& s, const T* sDelim)
{
    return cut(s, std::basic_string<T>(sDelim));
}

template<class T>
std::vector<std::basic_string<T>> cut(const std::basic_string<T>& s, const std::basic_string<T>& sDelim, uint uiMaxCut)
{
    std::vector<std::basic_string<T>> lPieces;
    size_t uiPos = s.find(sDelim);
    size_t uiLastPos = 0u;
    size_t uiCurSize = 0u;
    uint uiCount = 0u;

    while (uiPos != std::basic_string<T>::npos)
    {
        uiCurSize = uiPos - uiLastPos;
        if (uiCurSize != 0)
            lPieces.push_back(s.substr(uiLastPos, uiCurSize));
        uiLastPos = uiPos + sDelim.size();
        uiPos = s.find(sDelim, uiLastPos);
        ++uiCount;

        if (uiCount >= uiMaxCut)
            break;
    }

    lPieces.push_back(s.substr(uiLastPos));

    return lPieces;
}

template<class T>
std::vector<std::basic_string<T>> cut(const std::basic_string<T>& s, const T* sDelim, uint uiMaxCut)
{
    return cut(s, std::basic_string<T>(sDelim), uiMaxCut);
}

template<class T>
std::vector<std::basic_string<T>> cut_each(const std::basic_string<T>& s, const std::basic_string<T>& sDelim)
{
    std::vector<std::basic_string<T>> lPieces;
    size_t uiPos = s.find(sDelim);
    size_t uiLastPos = 0u;
    size_t uiCurSize = 0u;

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

template<class T>
std::vector<std::basic_string<T>> cut_each(const std::basic_string<T>& s, const T* sDelim)
{
    return cut_each(s, std::basic_string<T>(sDelim));
}

template<class T>
std::vector<std::basic_string<T>> cut_each(const std::basic_string<T>& s, const std::basic_string<T>& sDelim, uint uiMaxCut)
{
    std::vector<std::basic_string<T>> lPieces;
    size_t uiPos = s.find(sDelim);
    size_t uiLastPos = 0u;
    size_t uiCurSize = 0u;
    uint uiCount = 0u;

    while (uiPos != std::basic_string<T>::npos)
    {
        uiCurSize = uiPos - uiLastPos;
        lPieces.push_back(s.substr(uiLastPos, uiCurSize));
        uiLastPos = uiPos + sDelim.size();
        uiPos = s.find(sDelim, uiLastPos);

        if (uiCount >= uiMaxCut)
            break;
    }

    lPieces.push_back(s.substr(uiLastPos));

    return lPieces;
}

template<class T>
std::vector<std::basic_string<T>> cut_each(const std::basic_string<T>& s, const T* sDelim, uint uiMaxCut)
{
    return cut_each(s, std::basic_string<T>(sDelim), uiMaxCut);
}

template<class T>
std::vector<std::basic_string<T>> cut(const std::basic_string<T>& s, const std::vector<T>& lDelims)
{
    std::vector<std::basic_string<T>> lPieces;

    std::basic_string<T> sTemp;
    for (auto c : s)
    {
        if (std::find(lDelims.begin(), lDelims.end(), c) != lDelims.end())
        {
            if (!sTemp.empty())
            {
                lPieces.push_back(sTemp);
                sTemp.clear();
            }
        }
        else
            sTemp.push_back(c);
    }

    if (!sTemp.empty())
        lPieces.push_back(sTemp);

    return lPieces;
}

bool starts_with(const string& s, const string& sPattern);
bool ends_with(const string& s, const string& sPattern);

template<class T>
typename std::vector<T>::iterator find(std::vector<T>& v, const T& s)
{
    return std::find(v.begin(), v.end(), s);
}

template<class T>
typename std::vector<T>::const_iterator find(const std::vector<T>& v, const T& s)
{
    return std::find(v.begin(), v.end(), s);
}

bool has_no_content(const std::string& s);

ustring  UTF8_to_unicode(const string& s);
char32_t UTF8_to_unicode(char c);

string  unicode_to_UTF8(const ustring& s);
char    unicode_to_UTF8(char32_t c);

uint    hex_to_uint(const string& s);

long string_to_int(const string& s);
long string_to_int(const ustring& s);

unsigned long string_to_uint(const string& s);
unsigned long  string_to_uint(const ustring& s);

double string_to_float(const string& s);
double string_to_float(const ustring& s);

bool string_to_bool(const string& s);
bool string_to_bool(const ustring& s);

string int_to_string(const long& i);
string uint_to_string(const unsigned long& ui);

string float_to_string(float f);
string double_to_string(double d);

bool is_number(const std::string& s);
bool is_number(char s);
bool is_boolean(const std::string& s);

string to_string(const string& s, uint uiNbr);

string to_string(const char* s);
string to_string(const char* s, uint uiNbr);

string to_string(char* s);
string to_string(char* s, uint uiNbr);

template<uint N>
string to_string(const char s[N])
{
    return s;
}

template<uint N>
string to_string(const char s[N], uint uiNbr)
{
    string sReturn;

    for (uint uiCounter = 0; uiCounter < uiNbr; ++uiCounter)
        sReturn += s;

    return sReturn;
}

template<uint N>
string to_string(char s[N])
{
    return s;
}

template<uint N>
string to_string(char s[N], uint uiNbr)
{
    string sReturn;

    for (uint uiCounter = 0; uiCounter < uiNbr; ++uiCounter)
        sReturn += s;

    return sReturn;
}

string to_string(char c);
string to_string(char c, uint uiNbr);

string to_string(int i);
string to_string(int i, uint uiCharNbr);

string to_string(uint ui);
string to_string(uint ui, uint uiCharNbr);

string to_string(long i);
string to_string(long i, uint uiCharNbr);

string to_string(ulong ui);
string to_string(ulong ui, uint uiCharNbr);

string to_string(float f);
string to_string(float f, uint uiDigitNbr);
string to_string(float f, uint uiIntCharNbr, uint uiFraCharNbr);

string to_string(double f);
string to_string(double f, uint uiDigitNbr);
string to_string(double f, uint uiIntCharNbr, uint uiFraCharNbr);

string to_string(bool b);

template<class T>
string to_string(T* p)
{
    if (p != nullptr)
        return to_string(reinterpret_cast<std::size_t>(p));
    else
        return "NULL";
}
}

namespace std {
template<class T, size_t N>
ostream& operator << (ostream& o, const array<T, N>& a)
{
    o << "(";
    for (size_t i = 0; i < N; ++i)
    {
        if (i != N-1)
            o << a[i] << ", ";
        else
            o << a[i];
    }
    o << ")";
    return o;
}

template<class T>
ostream& operator << (ostream& o, const vector<T>& a)
{
    o << "(";
    const size_t N = a.size();
    for (size_t i = 0; i < N; ++i)
    {
        if (i != N-1)
            o << a[i] << ", ";
        else
            o << a[i];
    }
    o << ")";
    return o;
}

template<class T>
ostream& operator << (ostream& o, const deque<T>& a)
{
    o << "(";
    const size_t N = a.size();
    for (size_t i = 0; i < N; ++i)
    {
        if (i != N-1)
            o << a[i] << ", ";
        else
            o << a[i];
    }
    o << ")";
    return o;
}
}

/** \endcond
*/

#endif
