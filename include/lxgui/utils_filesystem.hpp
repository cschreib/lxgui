#ifndef LXGUI_UTILS_FILESYSTEM_HPP
#define LXGUI_UTILS_FILESYSTEM_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

#include <string>
#include <vector>

namespace lxgui::utils {

using string_vector = std::vector<std::string>;

bool file_exists(const std::string& file);

bool make_directory(const std::string& path);

string_vector get_directory_list(const std::string& rel_path);

string_vector get_file_list(const std::string& rel_path, bool b_with_path = false);

string_vector
get_file_list(const std::string& rel_path, bool b_with_path, const std::string& extensions);

std::string get_file_extension(const std::string& file);

} // namespace lxgui::utils

#endif
