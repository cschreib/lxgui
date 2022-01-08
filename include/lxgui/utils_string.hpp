#ifndef LXGUI_UTILS_STRING_HPP
#define LXGUI_UTILS_STRING_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/utils.hpp"
#include "lxgui/utils_variant.hpp"

#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <algorithm>

/** \cond INCLUDE_INTERNALS_IN_DOC
*/
namespace lxgui {
namespace utils
{
    using string = std::string;
    using ustring = std::u32string;

    void trim(string& s, char cPattern);
    void trim(string& s, const string& sPatterns);
    void replace(string& s, const string& sPattern, const string& sReplacement);

    std::size_t count_occurrences(const string& s, const string& sPattern);

    std::vector<string> cut(const string& s, const string& sDelim);
    std::vector<ustring> cut(const ustring& s, const ustring& sDelim);

    std::vector<string> cut_each(const string& s, const string& sDelim);
    std::vector<ustring> cut_each(const ustring& s, const ustring& sDelim);

    std::pair<string,string> cut_first(const string& s, const string& sDelim);
    std::pair<ustring,ustring> cut_first(const ustring& s, const ustring& sDelim);

    bool starts_with(const string& s, const string& sPattern);
    bool ends_with(const string& s, const string& sPattern);

    bool has_no_content(const string& s);

    ustring utf8_to_unicode(const string& s);
    string  unicode_to_utf8(const ustring& s);

    std::size_t hex_to_uint(const string& s);

    bool from_string(const string&, int&);
    bool from_string(const string&, long&);
    bool from_string(const string&, long long&);
    bool from_string(const string&, unsigned&);
    bool from_string(const string&, unsigned long&);
    bool from_string(const string&, unsigned long long&);
    bool from_string(const string&, float&);
    bool from_string(const string&, double&);
    bool from_string(const string&, bool&);
    bool from_string(const string&, string&);

    bool from_string(const ustring&, int&);
    bool from_string(const ustring&, long&);
    bool from_string(const ustring&, long long&);
    bool from_string(const ustring&, unsigned&);
    bool from_string(const ustring&, unsigned long&);
    bool from_string(const ustring&, unsigned long long&);
    bool from_string(const ustring&, float&);
    bool from_string(const ustring&, double&);
    bool from_string(const ustring&, bool&);
    bool from_string(const ustring&, ustring&);

    bool is_number(const string& s);
    bool is_number(const ustring& s);
    bool is_number(char s);
    bool is_number(char32_t s);
    bool is_integer(const string& s);
    bool is_integer(const ustring& s);
    bool is_integer(char s);
    bool is_integer(char32_t s);
    bool is_boolean(const string& s);
    bool is_boolean(const ustring& s);

    bool is_whitespace(char c);
    bool is_whitespace(char32_t c);

    string to_string(int v);
    string to_string(long v);
    string to_string(long long v);
    string to_string(unsigned v);
    string to_string(unsigned long v);
    string to_string(unsigned long long v);
    string to_string(float v);
    string to_string(double v);
    string to_string(bool v);
    string to_string(bool b);
    string to_string(void* p);

    template<typename T>
    string to_string(T* p)
    {
        if (p != nullptr)
            return to_string(static_cast<void*>(p));
        else
            return "null";
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
