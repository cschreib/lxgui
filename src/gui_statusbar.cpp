#include "lxgui/gui_statusbar.hpp"

#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"

#include <sstream>

namespace lxgui::gui {

std::array<float, 4> select_uvs(const std::array<float, 8>& uvs) {
    return {uvs[0], uvs[1], uvs[4], uvs[5]};
}

status_bar::status_bar(utils::control_block& m_block, manager& m_manager) :
    frame(m_block, m_manager) {
    type_.push_back(class_name);
}

std::string status_bar::serialize(const std::string& tab) const {
    std::ostringstream str;

    str << base::serialize(tab);
    str << tab << "  # Orientation: ";
    switch (m_orientation_) {
    case orientation::horizontal: str << "HORIZONTAL"; break;
    case orientation::vertical: str << "VERTICAL"; break;
    }
    str << "\n";
    str << tab << "  # Reversed   : " << b_reversed_ << "\n";
    str << tab << "  # Value      : " << f_value_ << "\n";
    str << tab << "  # Min value  : " << f_min_value_ << "\n";
    str << tab << "  # Max value  : " << f_max_value_ << "\n";

    return str.str();
}

bool status_bar::can_use_script(const std::string& script_name) const {
    if (base::can_use_script(script_name))
        return true;
    else if (script_name == "OnValueChanged")
        return true;
    else
        return false;
}

void status_bar::copy_from(const region& m_obj) {
    base::copy_from(m_obj);

    const status_bar* p_status_bar = down_cast<status_bar>(&m_obj);
    if (!p_status_bar)
        return;

    this->set_min_value(p_status_bar->get_min_value());
    this->set_max_value(p_status_bar->get_max_value());
    this->set_value(p_status_bar->get_value());
    this->set_bar_draw_layer(p_status_bar->get_bar_draw_layer());
    this->set_orientation(p_status_bar->get_orientation());
    this->set_reversed(p_status_bar->is_reversed());

    if (const texture* p_bar = p_status_bar->get_bar_texture().get()) {
        region_core_attributes m_attr;
        m_attr.name        = p_bar->get_name();
        m_attr.inheritance = {p_status_bar->get_bar_texture()};

        auto p_bar_texture =
            this->create_layered_region<texture>(p_bar->get_draw_layer(), std::move(m_attr));

        if (p_bar_texture) {
            p_bar_texture->set_special();
            p_bar_texture->notify_loaded();
            this->set_bar_texture(p_bar_texture);
        }
    }
}

void status_bar::set_min_value(float f_min) {
    if (f_min != f_min_value_) {
        f_min_value_ = f_min;
        if (f_min_value_ > f_max_value_)
            f_min_value_ = f_max_value_;
        f_value_ = f_value_ > f_max_value_ ? f_max_value_
                                           : (f_value_ < f_min_value_ ? f_min_value_ : f_value_);
        notify_bar_texture_needs_update_();
    }
}

void status_bar::set_max_value(float f_max) {
    if (f_max != f_max_value_) {
        f_max_value_ = f_max;
        if (f_max_value_ < f_min_value_)
            f_max_value_ = f_min_value_;
        f_value_ = f_value_ > f_max_value_ ? f_max_value_
                                           : (f_value_ < f_min_value_ ? f_min_value_ : f_value_);
        notify_bar_texture_needs_update_();
    }
}

void status_bar::set_min_max_values(float f_min, float f_max) {
    if (f_min != f_min_value_ || f_max != f_max_value_) {
        f_min_value_ = std::min(f_min, f_max);
        f_max_value_ = std::max(f_min, f_max);
        f_value_     = f_value_ > f_max_value_ ? f_max_value_
                                           : (f_value_ < f_min_value_ ? f_min_value_ : f_value_);
        notify_bar_texture_needs_update_();
    }
}

void status_bar::set_value(float f_value) {
    f_value =
        f_value > f_max_value_ ? f_max_value_ : (f_value < f_min_value_ ? f_min_value_ : f_value);
    if (f_value != f_value_) {
        f_value_ = f_value;
        notify_bar_texture_needs_update_();
    }
}

void status_bar::set_bar_draw_layer(layer m_bar_layer) {
    m_bar_layer_ = m_bar_layer;
    if (p_bar_texture_)
        p_bar_texture_->set_draw_layer(m_bar_layer_);
}

void status_bar::set_bar_draw_layer(const std::string& bar_layer_name) {
    if (bar_layer_name == "ARTWORK")
        m_bar_layer_ = layer::artwork;
    else if (bar_layer_name == "BACKGROUND")
        m_bar_layer_ = layer::background;
    else if (bar_layer_name == "BORDER")
        m_bar_layer_ = layer::border;
    else if (bar_layer_name == "HIGHLIGHT")
        m_bar_layer_ = layer::highlight;
    else if (bar_layer_name == "OVERLAY")
        m_bar_layer_ = layer::overlay;
    else {
        gui::out << gui::warning << "gui::" << type_.back()
                 << " : "
                    "Unknown layer type : \"" +
                        bar_layer_name + "\". Using \"ARTWORK\"."
                 << std::endl;

        m_bar_layer_ = layer::artwork;
    }

    if (p_bar_texture_)
        p_bar_texture_->set_draw_layer(m_bar_layer_);
}

void status_bar::set_bar_texture(utils::observer_ptr<texture> p_bar_texture) {
    p_bar_texture_ = std::move(p_bar_texture);
    if (!p_bar_texture_)
        return;

    p_bar_texture_->set_draw_layer(m_bar_layer_);
    p_bar_texture_->clear_all_points();

    std::string parent = p_bar_texture_->get_parent().get() == this ? "$parent" : name_;

    if (b_reversed_)
        p_bar_texture_->set_point(anchor_point::top_right, parent);
    else
        p_bar_texture_->set_point(anchor_point::bottom_left, parent);

    initial_text_coords_ = select_uvs(p_bar_texture_->get_tex_coord());
    notify_bar_texture_needs_update_();
}

void status_bar::set_bar_color(const color& m_bar_color) {
    create_bar_texture_();

    m_bar_color_ = m_bar_color;
    p_bar_texture_->set_solid_color(m_bar_color_);
}

void status_bar::set_orientation(orientation m_orientation) {
    if (m_orientation != m_orientation_) {
        m_orientation_ = m_orientation;
        notify_bar_texture_needs_update_();
    }
}

void status_bar::set_orientation(const std::string& orientation_name) {
    orientation m_orientation = orientation::horizontal;
    if (orientation_name == "VERTICAL")
        m_orientation = orientation::vertical;
    else if (orientation_name == "HORIZONTAL")
        m_orientation = orientation::horizontal;
    else {
        gui::out << gui::warning << "gui::" << type_.back() << " : Unknown orientation : \""
                 << orientation_name << "\". Using \"HORIZONTAL\"." << std::endl;
    }

    set_orientation(m_orientation);
}

void status_bar::set_reversed(bool b_reversed) {
    if (b_reversed == b_reversed_)
        return;

    b_reversed_ = b_reversed;

    if (p_bar_texture_) {
        if (b_reversed_)
            p_bar_texture_->set_point(anchor_point::top_right);
        else
            p_bar_texture_->set_point(anchor_point::bottom_left);

        if (!b_virtual_)
            p_bar_texture_->notify_borders_need_update();
    }
}

float status_bar::get_min_value() const {
    return f_min_value_;
}

float status_bar::get_max_value() const {
    return f_max_value_;
}

float status_bar::get_value() const {
    return f_value_;
}

layer status_bar::get_bar_draw_layer() const {
    return m_bar_layer_;
}

const color& status_bar::get_bar_color() const {
    return m_bar_color_;
}

status_bar::orientation status_bar::get_orientation() const {
    return m_orientation_;
}

bool status_bar::is_reversed() const {
    return b_reversed_;
}

void status_bar::create_bar_texture_() {
    if (p_bar_texture_)
        return;

    auto p_bar_texture = create_layered_region<texture>(m_bar_layer_, "$parentBarTexture");
    if (!p_bar_texture)
        return;

    p_bar_texture->set_special();
    p_bar_texture->notify_loaded();
    set_bar_texture(p_bar_texture);
}

void status_bar::create_glue() {
    create_glue_(this);
}

void status_bar::update(float f_delta) {
    if (b_update_bar_texture_ && p_bar_texture_) {
        float f_coef = (f_value_ - f_min_value_) / (f_max_value_ - f_min_value_);

        if (m_orientation_ == orientation::horizontal)
            p_bar_texture_->set_relative_dimensions(vector2f(f_coef, 1.0f));
        else
            p_bar_texture_->set_relative_dimensions(vector2f(1.0f, f_coef));

        std::array<float, 4> uvs = initial_text_coords_;
        if (m_orientation_ == orientation::horizontal) {
            if (b_reversed_)
                uvs[0] = (uvs[0] - uvs[2]) * f_coef + uvs[2];
            else
                uvs[2] = (uvs[2] - uvs[0]) * f_coef + uvs[0];
        } else {
            if (b_reversed_)
                uvs[3] = (uvs[3] - uvs[1]) * f_coef + uvs[1];
            else
                uvs[1] = (uvs[1] - uvs[3]) * f_coef + uvs[3];
        }

        p_bar_texture_->set_tex_rect(uvs);

        b_update_bar_texture_ = false;
    }

    alive_checker m_checker(*this);
    base::update(f_delta);
    if (!m_checker.is_alive())
        return;
}

void status_bar::notify_bar_texture_needs_update_() {
    b_update_bar_texture_ = true;
}

} // namespace lxgui::gui
