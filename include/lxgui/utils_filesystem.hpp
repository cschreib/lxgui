#ifndef LXGUI_UTILS_FILESYSTEM_HPP
#define LXGUI_UTILS_FILESYSTEM_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

#include <string>
#include <vector>

namespace lxgui::utils {

typedef std::vector<std::string> string_vector;

bool file_exists(const std::string& s_file);

bool          make_directory(const std::string& s_path);
string_vector get_directory_list(const std::string& s_rel_path);
string_vector get_file_list(const std::string& s_rel_path, bool b_with_path = false);
string_vector
            get_file_list(const std::string& s_rel_path, bool b_with_path, const std::string& s_extensions);
std::string get_file_extension(const std::string& s_file);

} // namespace lxgui::utils

#endif
