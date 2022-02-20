#include "lxgui/utils_filesystem.hpp"

#include "lxgui/utils_std.hpp"
#include "lxgui/utils_string.hpp"

#include <filesystem>

namespace lxgui::utils {

bool file_exists(const std::string& file) {
    return std::filesystem::exists(file) && std::filesystem::is_regular_file(file);
}

string_vector get_directory_list(const std::string& rel_path) {
    string_vector dir_list;
    for (const auto& entry : std::filesystem::directory_iterator(rel_path)) {
        if (entry.is_directory())
            dir_list.push_back(entry.path().filename().u8string());
    }

    return dir_list;
}

string_vector get_file_list(const std::string& rel_path, bool with_path) {
    string_vector file_list;
    for (const auto& entry : std::filesystem::directory_iterator(rel_path)) {
        if (entry.is_regular_file()) {
            if (with_path)
                file_list.push_back(entry.path().relative_path().u8string());
            else
                file_list.push_back(entry.path().filename().u8string());
        }
    }

    return file_list;
}

string_vector
get_file_list(const std::string& rel_path, bool with_path, const std::string& extensions) {
    auto extension_list = utils::cut(extensions, ",");
    for (auto& extension : extension_list)
        extension = utils::trim(extension, ' ');

    string_vector file_list;
    for (const auto& entry : std::filesystem::directory_iterator(rel_path)) {
        if (entry.is_regular_file()) {
            if (utils::find(extension_list, entry.path().extension().u8string()) !=
                extension_list.end()) {
                if (with_path)
                    file_list.push_back(entry.path().relative_path().u8string());
                else
                    file_list.push_back(entry.path().filename().u8string());
            }
        }
    }

    return file_list;
}

bool make_directory(const std::string& path) {
    std::filesystem::create_directories(path);
    return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

std::string get_file_extension(const std::string& file) {
    return std::filesystem::path(file).extension().u8string();
}

} // namespace lxgui::utils
