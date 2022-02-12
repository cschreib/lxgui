#include "lxgui/utils_filesystem.hpp"

#include "lxgui/utils_std.hpp"
#include "lxgui/utils_string.hpp"

#include <filesystem>

namespace lxgui::utils {

bool file_exists(const std::string& s_file) {
    return std::filesystem::exists(s_file) && std::filesystem::is_regular_file(s_file);
}

string_vector get_directory_list(const std::string& s_rel_path) {
    string_vector l_dir_list;
    for (const auto& m_entry : std::filesystem::directory_iterator(s_rel_path)) {
        if (m_entry.is_directory())
            l_dir_list.push_back(m_entry.path().filename().u8string());
    }

    return l_dir_list;
}

string_vector get_file_list(const std::string& s_rel_path, bool b_with_path) {
    string_vector l_file_list;
    for (const auto& m_entry : std::filesystem::directory_iterator(s_rel_path)) {
        if (m_entry.is_regular_file()) {
            if (b_with_path)
                l_file_list.push_back(m_entry.path().relative_path().u8string());
            else
                l_file_list.push_back(m_entry.path().filename().u8string());
        }
    }

    return l_file_list;
}

string_vector
get_file_list(const std::string& s_rel_path, bool b_with_path, const std::string& s_extensions) {
    auto l_extensions = utils::cut(s_extensions, ",");
    for (auto& s_extension : l_extensions)
        s_extension = utils::trim(s_extension, ' ');

    string_vector l_file_list;
    for (const auto& m_entry : std::filesystem::directory_iterator(s_rel_path)) {
        if (m_entry.is_regular_file()) {
            if (utils::find(l_extensions, m_entry.path().extension().u8string()) !=
                l_extensions.end()) {
                if (b_with_path)
                    l_file_list.push_back(m_entry.path().relative_path().u8string());
                else
                    l_file_list.push_back(m_entry.path().filename().u8string());
            }
        }
    }

    return l_file_list;
}

bool make_directory(const std::string& s_path) {
    std::filesystem::create_directories(s_path);
    return std::filesystem::exists(s_path) && std::filesystem::is_directory(s_path);
}

std::string get_file_extension(const std::string& s_file) {
    return std::filesystem::path(s_file).extension().u8string();
}

} // namespace lxgui::utils
