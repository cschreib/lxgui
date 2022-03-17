#include "lxgui/gui_font_string.hpp"

#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_renderer.hpp"

#include <sstream>

namespace lxgui::gui {

font_string::font_string(
    utils::control_block& block, manager& mgr, const region_core_attributes& attr) :
    layered_region(block, mgr, attr) {

    initialize_(*this, attr);
}

void font_string::render() const {
    if (!text_ || !is_ready_ || !is_visible())
        return;

    vector2f pos;

    if (std::isinf(text_->get_box_width())) {
        switch (align_x_) {
        case alignment_x::left: pos.x = border_list_.left; break;
        case alignment_x::center: pos.x = (border_list_.left + border_list_.right) / 2; break;
        case alignment_x::right: pos.x = border_list_.right; break;
        }
    } else {
        pos.x = border_list_.left;
    }

    if (std::isinf(text_->get_box_height())) {
        switch (align_y_) {
        case alignment_y::top: pos.y = border_list_.top; break;
        case alignment_y::middle: pos.y = (border_list_.top + border_list_.bottom) / 2; break;
        case alignment_y::bottom: pos.y = border_list_.bottom; break;
        }
    } else {
        pos.y = border_list_.top;
    }

    pos += offset_;

    text_->set_alpha(get_effective_alpha());

    if (has_shadow_) {
        text_->set_color(shadow_color_, true);
        text_->render(matrix4f::translation(round_to_pixel(pos + shadow_offset_)));
    }

    text_->set_color(text_color_);
    text_->render(matrix4f::translation(round_to_pixel(pos)));
}

std::string font_string::serialize(const std::string& tab) const {
    std::ostringstream str;

    str << base::serialize(tab);

    str << tab << "  # Font name  : " << font_name_ << "\n";
    str << tab << "  # Font height: " << height_ << "\n";
    str << tab << "  # Text ready : " << (text_ != nullptr) << "\n";
    str << tab << "  # Text       : \"" << utils::unicode_to_utf8(content_) << "\"\n";
    str << tab << "  # Outlined   : " << is_outlined_ << "\n";
    str << tab << "  # Text color : " << text_color_ << "\n";
    str << tab << "  # Spacing    : " << spacing_ << "\n";
    str << tab << "  # Justify     :\n";
    str << tab << "  #-###\n";
    str << tab << "  |   # horizontal: " << utils::to_string(align_x_) << "\n";
    str << tab << "  |   # vertical  : " << utils::to_string(align_y_) << "\n";
    str << tab << "  #-###\n";
    str << tab << "  # NonSpaceW. : " << non_space_wrap_enabled_ << "\n";
    if (has_shadow_) {
        str << tab << "  # Shadow off.: (" << shadow_offset_.x << ", " << shadow_offset_.y << ")\n";
        str << tab << "  # Shadow col.: " << shadow_color_ << "\n";
    }

    return str.str();
}

void font_string::copy_from(const region& obj) {
    base::copy_from(obj);

    const font_string* fstr_obj = down_cast<font_string>(&obj);
    if (!fstr_obj)
        return;

    std::string font_name = fstr_obj->get_font_name();
    float       height    = fstr_obj->get_font_height();
    if (!font_name.empty() && height != 0)
        this->set_font(font_name, height);

    this->set_alignment_x(fstr_obj->get_alignment_x());
    this->set_alignment_y(fstr_obj->get_alignment_y());
    this->set_spacing(fstr_obj->get_spacing());
    this->set_line_spacing(fstr_obj->get_line_spacing());
    this->set_text(fstr_obj->get_text());
    this->set_outlined(fstr_obj->is_outlined());
    if (fstr_obj->has_shadow()) {
        this->set_shadow(true);
        this->set_shadow_color(fstr_obj->get_shadow_color());
        this->set_shadow_offset(fstr_obj->get_shadow_offset());
    }
    this->set_text_color(fstr_obj->get_text_color());
    this->set_non_space_wrap(fstr_obj->can_non_space_wrap());
}

const std::string& font_string::get_font_name() const {
    return font_name_;
}

float font_string::get_font_height() const {
    return height_;
}

void font_string::set_outlined(bool is_outlined) {
    if (is_outlined_ == is_outlined)
        return;

    is_outlined_ = is_outlined;

    create_text_object_();

    notify_renderer_need_redraw();
}

bool font_string::is_outlined() const {
    return is_outlined_;
}

alignment_x font_string::get_alignment_x() const {
    return align_x_;
}

alignment_y font_string::get_alignment_y() const {
    return align_y_;
}

const color& font_string::get_shadow_color() const {
    return shadow_color_;
}

const vector2f& font_string::get_shadow_offset() const {
    return shadow_offset_;
}

const vector2f& font_string::get_offset() const {
    return offset_;
}

float font_string::get_spacing() const {
    return spacing_;
}

float font_string::get_line_spacing() const {
    return line_spacing_;
}

const color& font_string::get_text_color() const {
    return text_color_;
}

void font_string::notify_scaling_factor_updated() {
    base::notify_scaling_factor_updated();

    if (text_)
        set_font(font_name_, height_);
}

void font_string::create_text_object_() {
    if (font_name_.empty())
        return;

    std::size_t pixel_height = static_cast<std::size_t>(
        std::round(get_manager().get_interface_scaling_factor() * height_));

    auto&       renderer  = get_manager().get_renderer();
    const auto& localizer = get_manager().get_localizer();

    const auto&    code_points        = localizer.get_allowed_code_points();
    const char32_t default_code_point = localizer.get_fallback_code_point();

    std::shared_ptr<gui::font> outline_font;
    if (is_outlined_) {
        outline_font = renderer.create_atlas_font(
            "GUI", font_name_, pixel_height,
            std::min<std::size_t>(2u, static_cast<std::size_t>(std::round(0.2 * pixel_height))),
            code_points, default_code_point);
    }

    auto fnt = renderer.create_atlas_font(
        "GUI", font_name_, pixel_height, 0u, code_points, default_code_point);

    text_ = std::unique_ptr<text>(new text(renderer, fnt, outline_font));

    text_->set_scaling_factor(1.0f / get_manager().get_interface_scaling_factor());
    text_->set_remove_starting_spaces(true);
    text_->set_text(content_);
    text_->set_alignment_x(align_x_);
    text_->set_alignment_y(align_y_);
    text_->set_tracking(spacing_);
    text_->enable_word_wrap(word_wrap_enabled_, ellipsis_enabled_);
    text_->enable_formatting(formatting_enabled_);
}

void font_string::set_font(const std::string& font_name, float height) {
    font_name_ = parse_file_name(font_name);
    height_    = height;

    create_text_object_();

    if (!is_virtual_) {
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

void font_string::set_alignment_x(alignment_x justify_h) {
    if (align_x_ == justify_h)
        return;

    align_x_ = justify_h;
    if (text_) {
        text_->set_alignment_x(align_x_);

        if (!is_virtual_)
            notify_renderer_need_redraw();
    }
}

void font_string::set_alignment_y(alignment_y justify_v) {
    if (align_y_ == justify_v)
        return;

    align_y_ = justify_v;
    if (text_) {
        text_->set_alignment_y(align_y_);

        if (!is_virtual_)
            notify_renderer_need_redraw();
    }
}

void font_string::set_shadow_color(const color& shadow_color) {
    if (shadow_color_ == shadow_color)
        return;

    shadow_color_ = shadow_color;
    if (has_shadow_ && !is_virtual_)
        notify_renderer_need_redraw();
}

void font_string::set_shadow_offset(const vector2f& shadow_offset) {
    if (shadow_offset_ == shadow_offset)
        return;

    shadow_offset_ = shadow_offset;
    if (has_shadow_ && !is_virtual_)
        notify_renderer_need_redraw();
}

void font_string::set_offset(const vector2f& offset) {
    if (offset_ == offset)
        return;

    offset_ = offset;
    if (!is_virtual_)
        notify_renderer_need_redraw();
}

void font_string::set_spacing(float spacing) {
    if (spacing_ == spacing)
        return;

    spacing_ = spacing;
    if (text_) {
        text_->set_tracking(spacing_);
        if (!is_virtual_)
            notify_renderer_need_redraw();
    }
}

void font_string::set_line_spacing(float line_spacing) {
    if (line_spacing_ == line_spacing)
        return;

    line_spacing_ = line_spacing;
    if (text_) {
        text_->set_line_spacing(line_spacing_);
        if (!is_virtual_)
            notify_renderer_need_redraw();
    }
}

void font_string::set_text_color(const color& text_color) {
    if (text_color_ == text_color)
        return;

    text_color_ = text_color;
    if (!is_virtual_)
        notify_renderer_need_redraw();
}

bool font_string::can_non_space_wrap() const {
    return non_space_wrap_enabled_;
}

float font_string::get_string_height() const {
    if (text_)
        return text_->get_text_height();
    else
        return 0.0f;
}

float font_string::get_string_width() const {
    if (text_)
        return text_->get_text_width();
    else
        return 0.0f;
}

float font_string::get_string_width(const utils::ustring& content) const {
    if (text_)
        return text_->get_string_width(content);
    else
        return 0.0f;
}

const utils::ustring& font_string::get_text() const {
    return content_;
}

void font_string::set_non_space_wrap(bool can_non_space_wrap) {
    if (non_space_wrap_enabled_ == can_non_space_wrap)
        return;

    non_space_wrap_enabled_ = can_non_space_wrap;
    if (!is_virtual_)
        notify_renderer_need_redraw();
}

bool font_string::has_shadow() const {
    return has_shadow_;
}

void font_string::set_shadow(bool has_shadow) {
    if (has_shadow_ == has_shadow)
        return;

    has_shadow_ = has_shadow;
    if (!is_virtual_)
        notify_renderer_need_redraw();
}

void font_string::set_word_wrap(bool can_word_wrap, bool add_ellipsis) {
    word_wrap_enabled_ = can_word_wrap;
    ellipsis_enabled_  = add_ellipsis;
    if (text_)
        text_->enable_word_wrap(word_wrap_enabled_, ellipsis_enabled_);
}

bool font_string::can_word_wrap() const {
    return word_wrap_enabled_;
}

void font_string::enable_formatting(bool formatting) {
    formatting_enabled_ = formatting;
    if (text_)
        text_->enable_formatting(formatting_enabled_);
}

bool font_string::is_formatting_enabled() const {
    return formatting_enabled_;
}

void font_string::set_text(const utils::ustring& content) {
    if (content_ == content)
        return;

    content_ = content;
    if (text_) {
        text_->set_text(content_);
        if (!is_virtual_)
            notify_borders_need_update();
    }
}

text* font_string::get_text_object() {
    return text_.get();
}

const text* font_string::get_text_object() const {
    return text_.get();
}

void font_string::update_borders_() {
    if (!text_)
        return base::update_borders_();

//#define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

    const bool old_ready       = is_ready_;
    const auto old_border_list = border_list_;
    is_ready_                  = true;

    if (!anchor_list_.empty()) {
        float left = 0.0f, right = 0.0f, top = 0.0f, bottom = 0.0f;
        float x_center = 0.0f, y_center = 0.0f;

        DEBUG_LOG("  Read anchors");
        read_anchors_(left, right, top, bottom, x_center, y_center);

        float box_width = std::numeric_limits<float>::infinity();
        if (get_dimensions().x != 0.0f)
            box_width = get_dimensions().x;
        else if (defined_border_list_.left && defined_border_list_.right)
            box_width = right - left;

        float box_height = std::numeric_limits<float>::infinity();
        if (get_dimensions().y != 0.0f)
            box_height = get_dimensions().y;
        else if (defined_border_list_.top && defined_border_list_.bottom)
            box_height = bottom - top;

        box_width  = round_to_pixel(box_width, utils::rounding_method::nearest_not_zero);
        box_height = round_to_pixel(box_height, utils::rounding_method::nearest_not_zero);

        text_->set_box_dimensions(box_width, box_height);

        DEBUG_LOG("  Make borders");
        if (std::isinf(box_height))
            box_height = text_->get_height();
        if (std::isinf(box_width))
            box_width = text_->get_width();

        if (!make_borders_(top, bottom, y_center, box_height)) {
            is_ready_ = false;
        }

        if (!make_borders_(left, right, x_center, box_width)) {
            is_ready_ = false;
        }

        if (is_ready_) {
            if (right < left) {
                right = left + 1.0f;
            }
            if (bottom < top) {
                bottom = top + 1.0f;
            }

            border_list_.left   = left;
            border_list_.right  = right;
            border_list_.top    = top;
            border_list_.bottom = bottom;
        } else {
            border_list_ = bounds2f::zero;
        }
    } else {
        float box_width = get_dimensions().x;
        if (box_width == 0.0) {
            box_width = text_->get_width();
        }

        float box_height = get_dimensions().y;
        if (box_height == 0.0) {
            box_height = text_->get_height();
        }

        border_list_ = bounds2f(0.0, 0.0, box_width, box_height);
        is_ready_    = false;
    }

    border_list_.left   = round_to_pixel(border_list_.left);
    border_list_.right  = round_to_pixel(border_list_.right);
    border_list_.top    = round_to_pixel(border_list_.top);
    border_list_.bottom = round_to_pixel(border_list_.bottom);

    if (border_list_ != old_border_list || is_ready_ != old_ready) {
        DEBUG_LOG("  Fire redraw");
        notify_renderer_need_redraw();
    }
    DEBUG_LOG("  @");
}

} // namespace lxgui::gui
