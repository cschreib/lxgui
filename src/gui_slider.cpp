#include "lxgui/gui_slider.hpp"

#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"

#include <algorithm>
#include <sstream>

namespace lxgui::gui {

void step_value(float& value, float step) {
    // Makes the value a multiple of the step,
    // value = N*step, where N is an integer.
    if (step != 0.0f)
        value = std::round(value / step) * step;
}

slider::slider(utils::control_block& block, manager& mgr, const frame_core_attributes& attr) :
    frame(block, mgr, attr) {

    initialize_(*this, attr);

    enable_mouse(true);
    enable_drag(input::mouse_button::left);
}

std::string slider::serialize(const std::string& tab) const {
    std::ostringstream str;

    str << base::serialize(tab);
    str << tab << "  # Orientation: " << utils::to_string(orientation_) << "\n";
    str << tab << "  # Value     : " << value_ << "\n";
    str << tab << "  # Min value : " << min_value_ << "\n";
    str << tab << "  # Max value : " << max_value_ << "\n";
    str << tab << "  # Step      : " << value_step_ << "\n";
    str << tab << "  # Click out : " << allow_clicks_outside_thumb_ << "\n";

    return str.str();
}

bool slider::can_use_script(const std::string& script_name) const {
    return base::can_use_script(script_name) || script_name == "OnValueChanged";
}

void slider::fire_script(const std::string& script_name, const event_data& data) {
    alive_checker checker(*this);
    base::fire_script(script_name, data);
    if (!checker.is_alive())
        return;

    if (script_name == "OnDragStart") {
        if (thumb_texture_ &&
            thumb_texture_->is_in_region({data.get<float>(2), data.get<float>(3)})) {
            anchor& a = thumb_texture_->modify_point(point::center);

            get_manager().get_root().start_moving(
                thumb_texture_, &a,
                orientation_ == orientation::horizontal ? constraint::x : constraint::y,
                [&]() { constrain_thumb_(); });

            is_thumb_dragged_ = true;
        }
    } else if (script_name == "OnDragStop") {
        if (thumb_texture_) {
            if (get_manager().get_root().is_moving(*thumb_texture_))
                get_manager().get_root().stop_moving();

            is_thumb_dragged_ = false;
        }
    } else if (script_name == "OnMouseDown") {
        if (allow_clicks_outside_thumb_) {
            const vector2f apparent_size = get_apparent_dimensions();

            float value;
            if (orientation_ == orientation::horizontal) {
                float offset = data.get<float>(2) - borders_.left;
                value        = offset / apparent_size.x;
                set_value(value * (max_value_ - min_value_) + min_value_);
            } else {
                float offset = data.get<float>(3) - borders_.top;
                value        = offset / apparent_size.y;
                set_value(value * (max_value_ - min_value_) + min_value_);
            }
        }
    }
}

void slider::copy_from(const region& obj) {
    base::copy_from(obj);

    const slider* slider_obj = down_cast<slider>(&obj);
    if (!slider_obj)
        return;

    this->set_value_step(slider_obj->get_value_step());
    this->set_min_value(slider_obj->get_min_value());
    this->set_max_value(slider_obj->get_max_value());
    this->set_value(slider_obj->get_value());
    this->set_thumb_draw_layer(slider_obj->get_thumb_draw_layer());
    this->set_orientation(slider_obj->get_orientation());
    this->set_allow_clicks_outside_thumb(slider_obj->are_clicks_outside_thumb_allowed());

    if (const texture* thumb = slider_obj->get_thumb_texture().get()) {
        region_core_attributes attr;
        attr.name        = thumb->get_name();
        attr.inheritance = {slider_obj->get_thumb_texture()};

        auto tex = this->create_layered_region<texture>(thumb->get_draw_layer(), std::move(attr));

        if (tex) {
            tex->set_manually_inherited(true);
            tex->notify_loaded();
            this->set_thumb_texture(tex);
        }
    }
}

void slider::constrain_thumb_() {
    if (max_value_ == min_value_)
        return;

    const vector2f apparent_size = get_apparent_dimensions();

    if ((orientation_ == orientation::horizontal && apparent_size.x <= 0) ||
        (orientation_ == orientation::vertical && apparent_size.y <= 0))
        return;

    float old_value = value_;

    if (is_thumb_dragged_) {
        if (orientation_ == orientation::horizontal)
            value_ = thumb_texture_->get_point(point::center).offset.x / apparent_size.x;
        else
            value_ = thumb_texture_->get_point(point::center).offset.y / apparent_size.y;

        value_ = value_ * (max_value_ - min_value_) + min_value_;
        value_ = std::clamp(value_, min_value_, max_value_);
        step_value(value_, value_step_);
    }

    float coef = (value_ - min_value_) / (max_value_ - min_value_);

    anchor& a = thumb_texture_->modify_point(point::center);

    vector2f new_offset;
    if (orientation_ == orientation::horizontal)
        new_offset = vector2f(apparent_size.x * coef, 0);
    else
        new_offset = vector2f(0, apparent_size.y * coef);

    if (new_offset != a.offset) {
        a.offset = new_offset;
        thumb_texture_->notify_borders_need_update();
    }

    if (value_ != old_value)
        fire_script("OnValueChanged");
}

void slider::set_min_value(float min_value) {
    if (min_value == min_value_)
        return;

    min_value_ = min_value;
    if (min_value_ > max_value_)
        min_value_ = max_value_;
    else
        step_value(min_value_, value_step_);

    if (value_ < min_value_) {
        value_ = min_value_;
        fire_script("OnValueChanged");
    }

    notify_thumb_texture_needs_update_();
}

void slider::set_max_value(float max_value) {
    if (max_value == max_value_)
        return;

    max_value_ = max_value;
    if (max_value_ < min_value_)
        max_value_ = min_value_;
    else
        step_value(max_value_, value_step_);

    if (value_ > max_value_) {
        value_ = max_value_;
        fire_script("OnValueChanged");
    }

    notify_thumb_texture_needs_update_();
}

void slider::set_min_max_values(float min_value, float max_value) {
    if (min_value == min_value_ && max_value == max_value_)
        return;

    min_value_ = std::min(min_value, max_value);
    max_value_ = std::max(min_value, max_value);
    step_value(min_value_, value_step_);
    step_value(max_value_, value_step_);

    if (value_ > max_value_ || value_ < min_value_) {
        value_ = std::clamp(value_, min_value_, max_value_);
        fire_script("OnValueChanged");
    }

    notify_thumb_texture_needs_update_();
}

void slider::set_value(float value, bool silent) {
    value = std::clamp(value, min_value_, max_value_);
    step_value(value, value_step_);

    if (value == value_)
        return;

    value_ = value;

    if (!silent)
        fire_script("OnValueChanged");

    notify_thumb_texture_needs_update_();
}

void slider::set_value_step(float value_step) {
    if (value_step_ == value_step)
        return;

    value_step_ = value_step;

    step_value(min_value_, value_step_);
    step_value(max_value_, value_step_);

    float old_value = value_;
    value_          = std::clamp(value_, min_value_, max_value_);
    step_value(value_, value_step_);

    if (value_ != old_value)
        fire_script("OnValueChanged");

    notify_thumb_texture_needs_update_();
}

void slider::set_thumb_texture(utils::observer_ptr<texture> tex) {
    thumb_texture_ = std::move(tex);
    if (!thumb_texture_)
        return;

    thumb_texture_->set_draw_layer(thumb_layer_);
    thumb_texture_->clear_all_points();
    thumb_texture_->set_point(
        point::center, thumb_texture_->get_parent().get() == this ? "$parent" : name_,
        orientation_ == orientation::horizontal ? point::left : point::top);

    notify_thumb_texture_needs_update_();
}

void slider::set_orientation(orientation orient) {
    if (orientation_ == orient)
        return;

    orientation_ = orient;

    if (thumb_texture_) {
        thumb_texture_->set_point(
            point::center, name_,
            orientation_ == orientation::horizontal ? point::left : point::top);
    }

    notify_thumb_texture_needs_update_();
}

void slider::set_thumb_draw_layer(layer thumb_layer) {
    thumb_layer_ = thumb_layer;

    if (thumb_texture_)
        thumb_texture_->set_draw_layer(thumb_layer_);
}

float slider::get_min_value() const {
    return min_value_;
}

float slider::get_max_value() const {
    return max_value_;
}

float slider::get_value() const {
    return value_;
}

float slider::get_value_step() const {
    return value_step_;
}

orientation slider::get_orientation() const {
    return orientation_;
}

layer slider::get_thumb_draw_layer() const {
    return thumb_layer_;
}

void slider::set_allow_clicks_outside_thumb(bool allow) {
    allow_clicks_outside_thumb_ = allow;
}

bool slider::are_clicks_outside_thumb_allowed() const {
    return allow_clicks_outside_thumb_;
}

bool slider::is_in_region(const vector2f& position) const {
    if (allow_clicks_outside_thumb_) {
        if (base::is_in_region(position))
            return true;
    }

    return thumb_texture_ && thumb_texture_->is_in_region(position);
}

void slider::update_thumb_texture_() {
    if (!thumb_texture_)
        return;

    if (max_value_ == min_value_) {
        thumb_texture_->hide();
        return;
    } else
        thumb_texture_->show();

    constrain_thumb_();
}

void slider::notify_borders_need_update() {
    base::notify_borders_need_update();
    notify_thumb_texture_needs_update_();
}

void slider::notify_thumb_texture_needs_update_() {
    update_thumb_texture_();
}

} // namespace lxgui::gui
