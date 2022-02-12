#include "lxgui/input_window.hpp"

#include "lxgui/input_source.hpp"

namespace lxgui::input {

window::window(source& m_source) : m_source_(m_source) {
    m_connection_ = m_source.on_window_resized.connect([&](const gui::vector2ui& m_dimensions) {
        // Forward
        on_window_resized(m_dimensions);
    });
}

utils::ustring window::get_clipboard_content() {
    return m_source_.get_clipboard_content();
}

void window::set_clipboard_content(const utils::ustring& s_content) {
    return m_source_.set_clipboard_content(s_content);
}

void window::set_mouse_cursor(const std::string& s_file_name, const gui::vector2i& m_hot_spot) {
    return m_source_.set_mouse_cursor(s_file_name, m_hot_spot);
}

void window::reset_mouse_cursor() {
    return m_source_.reset_mouse_cursor();
}

const gui::vector2ui& window::get_dimensions() const {
    return m_source_.get_window_dimensions();
}

float window::get_interface_scaling_factor_hint() const {
    return m_source_.get_interface_scaling_factor_hint();
}

const source& window::get_source() const {
    return m_source_;
}

source& window::get_source() {
    return m_source_;
}

} // namespace lxgui::input
