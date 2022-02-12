#include "lxgui/gui_gradient.hpp"

namespace lxgui::gui {

gradient::gradient(orientation m_orientation, const color& m_min_color, const color& m_max_color) :
    m_orientation_(m_orientation), m_min_color_(m_min_color), m_max_color_(m_max_color) {}

const color& gradient::get_min_color() const {
    return m_min_color_;
}

const color& gradient::get_max_color() const {
    return m_max_color_;
}

gradient::orientation gradient::get_orientation() const {
    return m_orientation_;
}

} // namespace lxgui::gui
