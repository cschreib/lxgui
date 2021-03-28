#ifndef LXGUI_UTILS_STRING_HPP
#define LXGUI_UTILS_STRING_HPP

#include "lxgui/utils.hpp"
#include "lxgui/utils_variant.hpp"

#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <algorithm>

/** \cond NOT_REMOVE_FROM_DOC
*/
namespace lxgui {
namespace utils
{
    using string = std::string;
    using ustring = std::u32string;

    void trim(string& s, char cPattern);
    void trim(string& s, const string& sPatterns);
    void replace(string& s, const string& sPattern, const string& sReplacement);

    uint count_occurrences(const string& s, const string& sPattern);

    template<typename T>
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

    template<typename T>
    std::vector<std::basic_string<T>> cut(const std::basic_string<T>& s, const T* sDelim)
    {
        return cut(s, std::basic_string<T>(sDelim));
    }

    template<typename T>
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

    template<typename T>
    std::vector<std::basic_string<T>> cut(const std::basic_string<T>& s, const T* sDelim, uint uiMaxCut)
    {
        return cut(s, std::basic_string<T>(sDelim), uiMaxCut);
    }

    template<typename T>
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

    template<typename T>
    std::vector<std::basic_string<T>> cut_each(const std::basic_string<T>& s, const T* sDelim)
    {
        return cut_each(s, std::basic_string<T>(sDelim));
    }

    template<typename T>
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

    template<typename T>
    std::vector<std::basic_string<T>> cut_each(const std::basic_string<T>& s, const T* sDelim, uint uiMaxCut)
    {
        return cut_each(s, std::basic_string<T>(sDelim), uiMaxCut);
    }

    template<typename T>
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

    bool has_no_content(const std::string& s);

    ustring  utf8_to_unicode(const string& s);
    char32_t utf8_to_unicode(char c);

    string  unicode_to_utf8(const ustring& s);
    char    unicode_to_utf8(char32_t c);

    uint    hex_to_uint(const string& s);

    long string_to_int(const string& s);
    long string_to_int(const ustring& s);

    unsigned long string_to_uint(const string& s);
    unsigned long  string_to_uint(const ustring& s);

    double string_to_float(const string& s);
    double string_to_float(const ustring& s);

    bool string_to_bool(const string& s);
    bool string_to_bool(const ustring& s);

    bool is_number(const string& s);
    bool is_number(const ustring& s);
    bool is_number(char s);
    bool is_boolean(const string& s);
    bool is_boolean(const ustring& s);

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
    string to_string(void* p);

    template<typename T>
    string to_string(T* p)
    {
        if (p != nullptr)
            return to_string(static_cast<void*>(p));
        else
            return "NULL";
    }

    string to_string(const utils::variant& mValue);

    template<typename ... Args>
    ustring to_ustring(Args&& ... args)
    {
        return utils::utf8_to_unicode(to_string(std::forward<Args>(args)...));
    }
}
}

/** \endcond
*/

#endif
