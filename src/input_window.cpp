#include "lxgui/input_window.hpp"

#include "lxgui/input_source.hpp"

namespace lxgui { namespace input {

window::window(source& mSource) : mSource_(mSource) {
    mConnection_ = mSource.on_window_resized.connect([&](const gui::vector2ui& mDimensions) {
        // Forward
        on_window_resized(mDimensions);
    });
}

utils::ustring window::get_clipboard_content() {
    return mSource_.get_clipboard_content();
}

void window::set_clipboard_content(const utils::ustring& sContent) {
    return mSource_.set_clipboard_content(sContent);
}

void window::set_mouse_cursor(const std::string& sFileName, const gui::vector2i& mHotSpot) {
    return mSource_.set_mouse_cursor(sFileName, mHotSpot);
}

void window::reset_mouse_cursor() {
    return mSource_.reset_mouse_cursor();
}

const gui::vector2ui& window::get_dimensions() const {
    return mSource_.get_window_dimensions();
}

float window::get_interface_scaling_factor_hint() const {
    return mSource_.get_interface_scaling_factor_hint();
}

const source& window::get_source() const {
    return mSource_;
}

source& window::get_source() {
    return mSource_;
}

}} // namespace lxgui::input
