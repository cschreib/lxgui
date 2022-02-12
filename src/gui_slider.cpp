#include "lxgui/gui_slider.hpp"

#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"

#include <algorithm>
#include <sstream>

namespace lxgui::gui {

void step_value(float& f_value, float f_step) {
    // Makes the value a multiple of the step :
    // fValue = N*fStep, where N is an integer.
    if (f_step != 0.0f)
        f_value = std::round(f_value / f_step) * f_step;
}

slider::slider(utils::control_block& m_block, manager& m_manager) : frame(m_block, m_manager) {
    l_type_.push_back(class_name);
    enable_mouse(true);
    register_for_drag({"LeftButton"});
}

std::string slider::serialize(const std::string& s_tab) const {
    std::ostringstream s_str;

    s_str << base::serialize(s_tab);
    s_str << s_tab << "  # Orientation: ";
    switch (m_orientation_) {
    case orientation::horizontal: s_str << "HORIZONTAL"; break;
    case orientation::vertical: s_str << "VERTICAL"; break;
    }
    s_str << "\n";
    s_str << s_tab << "  # Value      : " << f_value_ << "\n";
    s_str << s_tab << "  # Min value  : " << f_min_value_ << "\n";
    s_str << s_tab << "  # Max value  : " << f_max_value_ << "\n";
    s_str << s_tab << "  # Step       : " << f_value_step_ << "\n";
    s_str << s_tab << "  # Click out  : " << b_allow_clicks_outside_thumb_ << "\n";

    return s_str.str();
}

bool slider::can_use_script(const std::string& s_script_name) const {
    if (base::can_use_script(s_script_name))
        return true;
    else if (s_script_name == "OnValueChanged")
        return true;
    else
        return false;
}

void slider::fire_script(const std::string& s_script_name, const event_data& m_data) {
    alive_checker m_checker(*this);
    base::fire_script(s_script_name, m_data);
    if (!m_checker.is_alive())
        return;

    if (s_script_name == "OnDragStart") {
        if (p_thumb_texture_ &&
            p_thumb_texture_->is_in_region({m_data.get<float>(1), m_data.get<float>(2)})) {
            anchor& m_anchor = p_thumb_texture_->modify_point(anchor_point::center);

            get_manager().get_root().start_moving(
                p_thumb_texture_, &m_anchor,
                m_orientation_ == orientation::horizontal ? constraint::x : constraint::y,
                [&]() { constrain_thumb_(); });

            b_thumb_moved_ = true;
        }
    } else if (s_script_name == "OnDragStop") {
        if (p_thumb_texture_) {
            if (get_manager().get_root().is_moving(*p_thumb_texture_))
                get_manager().get_root().stop_moving();

            b_thumb_moved_ = false;
        }
    } else if (s_script_name == "OnMouseDown") {
        if (b_allow_clicks_outside_thumb_) {
            const vector2f m_apparent_size = get_apparent_dimensions();

            float f_value;
            if (m_orientation_ == orientation::horizontal) {
                float f_offset = m_data.get<float>(1) - l_border_list_.left;
                f_value        = f_offset / m_apparent_size.x;
                set_value(f_value * (f_max_value_ - f_min_value_) + f_min_value_);
            } else {
                float f_offset = m_data.get<float>(2) - l_border_list_.top;
                f_value        = f_offset / m_apparent_size.y;
                set_value(f_value * (f_max_value_ - f_min_value_) + f_min_value_);
            }
        }
    }
}

void slider::copy_from(const region& m_obj) {
    base::copy_from(m_obj);

    const slider* p_slider = down_cast<slider>(&m_obj);
    if (!p_slider)
        return;

    this->set_value_step(p_slider->get_value_step());
    this->set_min_value(p_slider->get_min_value());
    this->set_max_value(p_slider->get_max_value());
    this->set_value(p_slider->get_value());
    this->set_thumb_draw_layer(p_slider->get_thumb_draw_layer());
    this->set_orientation(p_slider->get_orientation());
    this->set_allow_clicks_outside_thumb(p_slider->are_clicks_outside_thumb_allowed());

    if (const texture* p_thumb = p_slider->get_thumb_texture().get()) {
        region_core_attributes m_attr;
        m_attr.s_name        = p_thumb->get_name();
        m_attr.l_inheritance = {p_slider->get_thumb_texture()};

        auto p_texture =
            this->create_layered_region<texture>(p_thumb->get_draw_layer(), std::move(m_attr));

        if (p_texture) {
            p_texture->set_special();
            p_texture->notify_loaded();
            this->set_thumb_texture(p_texture);
        }
    }
}

void slider::constrain_thumb_() {
    if (f_max_value_ == f_min_value_)
        return;

    const vector2f m_apparent_size = get_apparent_dimensions();

    if ((m_orientation_ == orientation::horizontal && m_apparent_size.x <= 0) ||
        (m_orientation_ == orientation::vertical && m_apparent_size.y <= 0))
        return;

    float f_old_value = f_value_;

    if (b_thumb_moved_) {
        if (m_orientation_ == orientation::horizontal)
            f_value_ = p_thumb_texture_->get_point(anchor_point::center).m_offset.x / m_apparent_size.x;
        else
            f_value_ = p_thumb_texture_->get_point(anchor_point::center).m_offset.y / m_apparent_size.y;

        f_value_ = f_value_ * (f_max_value_ - f_min_value_) + f_min_value_;
        f_value_ = std::clamp(f_value_, f_min_value_, f_max_value_);
        step_value(f_value_, f_value_step_);
    }

    float f_coef = (f_value_ - f_min_value_) / (f_max_value_ - f_min_value_);

    anchor& m_anchor = p_thumb_texture_->modify_point(anchor_point::center);

    vector2f m_new_offset;
    if (m_orientation_ == orientation::horizontal)
        m_new_offset = vector2f(m_apparent_size.x * f_coef, 0);
    else
        m_new_offset = vector2f(0, m_apparent_size.y * f_coef);

    if (m_new_offset != m_anchor.m_offset) {
        m_anchor.m_offset = m_new_offset;
        p_thumb_texture_->notify_borders_need_update();
    }

    if (f_value_ != f_old_value)
        fire_script("OnValueChanged");
}

void slider::set_min_value(float f_min) {
    if (f_min != f_min_value_) {
        f_min_value_ = f_min;
        if (f_min_value_ > f_max_value_)
            f_min_value_ = f_max_value_;
        else
            step_value(f_min_value_, f_value_step_);

        if (f_value_ < f_min_value_) {
            f_value_ = f_min_value_;
            fire_script("OnValueChanged");
        }

        notify_thumb_texture_needs_update_();
    }
}

void slider::set_max_value(float f_max) {
    if (f_max != f_max_value_) {
        f_max_value_ = f_max;
        if (f_max_value_ < f_min_value_)
            f_max_value_ = f_min_value_;
        else
            step_value(f_max_value_, f_value_step_);

        if (f_value_ > f_max_value_) {
            f_value_ = f_max_value_;
            fire_script("OnValueChanged");
        }

        notify_thumb_texture_needs_update_();
    }
}

void slider::set_min_max_values(float f_min, float f_max) {
    if (f_min != f_min_value_ || f_max != f_max_value_) {
        f_min_value_ = std::min(f_min, f_max);
        f_max_value_ = std::max(f_min, f_max);
        step_value(f_min_value_, f_value_step_);
        step_value(f_max_value_, f_value_step_);

        if (f_value_ > f_max_value_ || f_value_ < f_min_value_) {
            f_value_ = std::clamp(f_value_, f_min_value_, f_max_value_);
            fire_script("OnValueChanged");
        }

        notify_thumb_texture_needs_update_();
    }
}

void slider::set_value(float f_value, bool b_silent) {
    f_value = std::clamp(f_value, f_min_value_, f_max_value_);
    step_value(f_value, f_value_step_);

    if (f_value != f_value_) {
        f_value_ = f_value;

        if (!b_silent)
            fire_script("OnValueChanged");

        notify_thumb_texture_needs_update_();
    }
}

void slider::set_value_step(float f_value_step) {
    if (f_value_step_ != f_value_step) {
        f_value_step_ = f_value_step;

        step_value(f_min_value_, f_value_step_);
        step_value(f_max_value_, f_value_step_);

        float f_old_value = f_value_;
        f_value_         = std::clamp(f_value_, f_min_value_, f_max_value_);
        step_value(f_value_, f_value_step_);

        if (f_value_ != f_old_value)
            fire_script("OnValueChanged");

        notify_thumb_texture_needs_update_();
    }
}

void slider::set_thumb_texture(utils::observer_ptr<texture> p_texture) {
    p_thumb_texture_ = std::move(p_texture);
    if (!p_thumb_texture_)
        return;

    p_thumb_texture_->set_draw_layer(m_thumb_layer_);
    p_thumb_texture_->clear_all_points();
    p_thumb_texture_->set_point(
        anchor_point::center, p_thumb_texture_->get_parent().get() == this ? "$parent" : s_name_,
        m_orientation_ == orientation::horizontal ? anchor_point::left : anchor_point::top);

    notify_thumb_texture_needs_update_();
}

void slider::set_orientation(orientation m_orientation) {
    if (m_orientation != m_orientation_) {
        m_orientation_ = m_orientation;
        if (p_thumb_texture_) {
            p_thumb_texture_->set_point(
                anchor_point::center, s_name_,
                m_orientation_ == orientation::horizontal ? anchor_point::left : anchor_point::top);
        }

        notify_thumb_texture_needs_update_();
    }
}

void slider::set_orientation(const std::string& s_orientation) {
    orientation m_orientation = orientation::horizontal;
    if (s_orientation == "VERTICAL")
        m_orientation = orientation::vertical;
    else if (s_orientation == "HORIZONTAL")
        m_orientation = orientation::horizontal;
    else {
        gui::out << gui::warning << "gui::" << l_type_.back()
                 << " : "
                    "Unknown orientation : \"" +
                        s_orientation + "\". Using \"HORIZONTAL\"."
                 << std::endl;
    }

    set_orientation(m_orientation);
}

void slider::set_thumb_draw_layer(layer m_thumb_layer) {
    m_thumb_layer_ = m_thumb_layer;
    if (p_thumb_texture_)
        p_thumb_texture_->set_draw_layer(m_thumb_layer_);
}

void slider::set_thumb_draw_layer(const std::string& s_thumb_layer) {
    if (s_thumb_layer == "ARTWORK")
        m_thumb_layer_ = layer::artwork;
    else if (s_thumb_layer == "BACKGROUND")
        m_thumb_layer_ = layer::background;
    else if (s_thumb_layer == "BORDER")
        m_thumb_layer_ = layer::border;
    else if (s_thumb_layer == "HIGHLIGHT")
        m_thumb_layer_ = layer::highlight;
    else if (s_thumb_layer == "OVERLAY")
        m_thumb_layer_ = layer::overlay;
    else {
        gui::out << gui::warning << "gui::" << l_type_.back()
                 << " : "
                    "Unknown layer type : \"" +
                        s_thumb_layer + "\". Using \"OVERLAY\"."
                 << std::endl;
        m_thumb_layer_ = layer::overlay;
    }

    if (p_thumb_texture_)
        p_thumb_texture_->set_draw_layer(m_thumb_layer_);
}

float slider::get_min_value() const {
    return f_min_value_;
}

float slider::get_max_value() const {
    return f_max_value_;
}

float slider::get_value() const {
    return f_value_;
}

float slider::get_value_step() const {
    return f_value_step_;
}

slider::orientation slider::get_orientation() const {
    return m_orientation_;
}

layer slider::get_thumb_draw_layer() const {
    return m_thumb_layer_;
}

void slider::set_allow_clicks_outside_thumb(bool b_allow) {
    b_allow_clicks_outside_thumb_ = b_allow;
}

bool slider::are_clicks_outside_thumb_allowed() const {
    return b_allow_clicks_outside_thumb_;
}

bool slider::is_in_region(const vector2f& m_position) const {
    if (b_allow_clicks_outside_thumb_) {
        if (base::is_in_region(m_position))
            return true;
    }

    return p_thumb_texture_ && p_thumb_texture_->is_in_region(m_position);
}

void slider::update_thumb_texture_() {
    if (!p_thumb_texture_)
        return;

    if (f_max_value_ == f_min_value_) {
        p_thumb_texture_->hide();
        return;
    } else
        p_thumb_texture_->show();

    constrain_thumb_();
}

void slider::notify_borders_need_update() {
    base::notify_borders_need_update();
    notify_thumb_texture_needs_update_();
}

void slider::create_glue() {
    create_glue_(this);
}

void slider::notify_thumb_texture_needs_update_() {
    update_thumb_texture_();
}

} // namespace lxgui::gui
