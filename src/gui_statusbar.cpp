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

status_bar::status_bar(utils::control_block& block, manager& mgr) : frame(block, mgr) {
    type_.push_back(class_name);
}

std::string status_bar::serialize(const std::string& tab) const {
    std::ostringstream str;

    str << base::serialize(tab);
    str << tab << "  # Orientation: ";
    switch (orientation_) {
    case orientation::horizontal: str << "HORIZONTAL"; break;
    case orientation::vertical: str << "VERTICAL"; break;
    }
    str << "\n";
    str << tab << "  # Reversed   : " << is_reversed_ << "\n";
    str << tab << "  # Value      : " << value_ << "\n";
    str << tab << "  # Min value  : " << min_value_ << "\n";
    str << tab << "  # Max value  : " << max_value_ << "\n";

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

void status_bar::copy_from(const region& obj) {
    base::copy_from(obj);

    const status_bar* bar_obj = down_cast<status_bar>(&obj);
    if (!bar_obj)
        return;

    this->set_min_value(bar_obj->get_min_value());
    this->set_max_value(bar_obj->get_max_value());
    this->set_value(bar_obj->get_value());
    this->set_bar_draw_layer(bar_obj->get_bar_draw_layer());
    this->set_orientation(bar_obj->get_orientation());
    this->set_reversed(bar_obj->is_reversed());

    if (const texture* bar = bar_obj->get_bar_texture().get()) {
        region_core_attributes attr;
        attr.name        = bar->get_name();
        attr.inheritance = {bar_obj->get_bar_texture()};

        auto bar_texture =
            this->create_layered_region<texture>(bar->get_draw_layer(), std::move(attr));

        if (bar_texture) {
            bar_texture->set_special();
            bar_texture->notify_loaded();
            this->set_bar_texture(bar_texture);
        }
    }
}

void status_bar::set_min_value(float min_value) {
    if (min_value != min_value_) {
        min_value_ = min_value;
        if (min_value_ > max_value_)
            min_value_ = max_value_;
        value_ = value_ > max_value_ ? max_value_ : (value_ < min_value_ ? min_value_ : value_);
        notify_bar_texture_needs_update_();
    }
}

void status_bar::set_max_value(float max_value) {
    if (max_value != max_value_) {
        max_value_ = max_value;
        if (max_value_ < min_value_)
            max_value_ = min_value_;
        value_ = value_ > max_value_ ? max_value_ : (value_ < min_value_ ? min_value_ : value_);
        notify_bar_texture_needs_update_();
    }
}

void status_bar::set_min_max_values(float min_value, float max_value) {
    if (min_value != min_value_ || max_value != max_value_) {
        min_value_ = std::min(min_value, max_value);
        max_value_ = std::max(min_value, max_value);
        value_     = value_ > max_value_ ? max_value_ : (value_ < min_value_ ? min_value_ : value_);
        notify_bar_texture_needs_update_();
    }
}

void status_bar::set_value(float value) {
    value = value > max_value_ ? max_value_ : (value < min_value_ ? min_value_ : value);
    if (value != value_) {
        value_ = value;
        notify_bar_texture_needs_update_();
    }
}

void status_bar::set_bar_draw_layer(layer bar_layer) {
    bar_layer_ = bar_layer;
    if (bar_texture_)
        bar_texture_->set_draw_layer(bar_layer_);
}

void status_bar::set_bar_draw_layer(const std::string& bar_layer_name) {
    layer bar_layer;
    if (bar_layer_name == "ARTWORK")
        bar_layer = layer::artwork;
    else if (bar_layer_name == "BACKGROUND")
        bar_layer = layer::background;
    else if (bar_layer_name == "BORDER")
        bar_layer = layer::border;
    else if (bar_layer_name == "HIGHLIGHT")
        bar_layer = layer::highlight;
    else if (bar_layer_name == "OVERLAY")
        bar_layer = layer::overlay;
    else {
        gui::out << gui::warning << "gui::" << type_.back() << " : Unknown layer type : \""
                 << bar_layer_name << "\". Using \"ARTWORK\"." << std::endl;
        bar_layer = layer::artwork;
    }

    set_bar_draw_layer(bar_layer);
}

void status_bar::set_bar_texture(utils::observer_ptr<texture> bar_texture) {
    bar_texture_ = std::move(bar_texture);
    if (!bar_texture_)
        return;

    bar_texture_->set_draw_layer(bar_layer_);
    bar_texture_->clear_all_points();

    std::string parent = bar_texture_->get_parent().get() == this ? "$parent" : name_;

    if (is_reversed_)
        bar_texture_->set_point(anchor_point::top_right, parent);
    else
        bar_texture_->set_point(anchor_point::bottom_left, parent);

    initial_text_coords_ = select_uvs(bar_texture_->get_tex_coord());
    notify_bar_texture_needs_update_();
}

void status_bar::set_bar_color(const color& bar_color) {
    create_bar_texture_();

    bar_color_ = bar_color;
    bar_texture_->set_solid_color(bar_color_);
}

void status_bar::set_orientation(orientation orient) {
    if (orient == orientation_)
        return;

    orientation_ = orient;
    notify_bar_texture_needs_update_();
}

void status_bar::set_orientation(const std::string& orientation_name) {
    orientation orient = orientation::horizontal;
    if (orientation_name == "VERTICAL")
        orient = orientation::vertical;
    else if (orientation_name == "HORIZONTAL")
        orient = orientation::horizontal;
    else {
        gui::out << gui::warning << "gui::" << type_.back() << " : Unknown orientation : \""
                 << orientation_name << "\". Using \"HORIZONTAL\"." << std::endl;
    }

    set_orientation(orient);
}

void status_bar::set_reversed(bool reversed) {
    if (reversed == is_reversed_)
        return;

    is_reversed_ = reversed;

    if (bar_texture_) {
        if (is_reversed_)
            bar_texture_->set_point(anchor_point::top_right);
        else
            bar_texture_->set_point(anchor_point::bottom_left);

        if (!is_virtual_)
            bar_texture_->notify_borders_need_update();
    }
}

float status_bar::get_min_value() const {
    return min_value_;
}

float status_bar::get_max_value() const {
    return max_value_;
}

float status_bar::get_value() const {
    return value_;
}

layer status_bar::get_bar_draw_layer() const {
    return bar_layer_;
}

const color& status_bar::get_bar_color() const {
    return bar_color_;
}

status_bar::orientation status_bar::get_orientation() const {
    return orientation_;
}

bool status_bar::is_reversed() const {
    return is_reversed_;
}

void status_bar::create_bar_texture_() {
    if (bar_texture_)
        return;

    auto bar_texture = create_layered_region<texture>(bar_layer_, "$parentBarTexture");
    if (!bar_texture)
        return;

    bar_texture->set_special();
    bar_texture->notify_loaded();
    set_bar_texture(bar_texture);
}

void status_bar::create_glue() {
    create_glue_(this);
}

void status_bar::update(float delta) {
    if (update_bar_texture_flag_ && bar_texture_) {
        float coef = (value_ - min_value_) / (max_value_ - min_value_);

        if (orientation_ == orientation::horizontal)
            bar_texture_->set_relative_dimensions(vector2f(coef, 1.0f));
        else
            bar_texture_->set_relative_dimensions(vector2f(1.0f, coef));

        std::array<float, 4> uvs = initial_text_coords_;
        if (orientation_ == orientation::horizontal) {
            if (is_reversed_)
                uvs[0] = (uvs[0] - uvs[2]) * coef + uvs[2];
            else
                uvs[2] = (uvs[2] - uvs[0]) * coef + uvs[0];
        } else {
            if (is_reversed_)
                uvs[3] = (uvs[3] - uvs[1]) * coef + uvs[1];
            else
                uvs[1] = (uvs[1] - uvs[3]) * coef + uvs[3];
        }

        bar_texture_->set_tex_rect(uvs);

        update_bar_texture_flag_ = false;
    }

    alive_checker checker(*this);
    base::update(delta);
    if (!checker.is_alive())
        return;
}

void status_bar::notify_bar_texture_needs_update_() {
    update_bar_texture_flag_ = true;
}

} // namespace lxgui::gui
