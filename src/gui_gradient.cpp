#include "lxgui/gui_gradient.hpp"

namespace lxgui::gui {

gradient::gradient(orientation orientation, const color& min_color, const color& max_color) :
    orientation_(orientation), min_color_(min_color), max_color_(max_color) {}

const color& gradient::get_min_color() const {
    return min_color_;
}

const color& gradient::get_max_color() const {
    return max_color_;
}

gradient::orientation gradient::get_orientation() const {
    return orientation_;
}

} // namespace lxgui::gui
