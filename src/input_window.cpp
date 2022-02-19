#include "lxgui/input_window.hpp"

#include "lxgui/input_source.hpp"

namespace lxgui::input {

window::window(source& src) : source_(src) {
    connection_ = src.on_window_resized.connect([&](const gui::vector2ui& dimensions) {
        // Forward
        on_window_resized(dimensions);
    });
}

utils::ustring window::get_clipboard_content() {
    return source_.get_clipboard_content();
}

void window::set_clipboard_content(const utils::ustring& content) {
    return source_.set_clipboard_content(content);
}

void window::set_mouse_cursor(const std::string& file_name, const gui::vector2i& hot_spot) {
    return source_.set_mouse_cursor(file_name, hot_spot);
}

void window::reset_mouse_cursor() {
    return source_.reset_mouse_cursor();
}

const gui::vector2ui& window::get_dimensions() const {
    return source_.get_window_dimensions();
}

float window::get_interface_scaling_factor_hint() const {
    return source_.get_interface_scaling_factor_hint();
}

const source& window::get_source() const {
    return source_;
}

source& window::get_source() {
    return source_;
}

} // namespace lxgui::input
