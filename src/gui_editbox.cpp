#include "lxgui/gui_editbox.hpp"

#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/input_window.hpp"
#include "lxgui/utils_range.hpp"

#include <sol/state.hpp>

using namespace lxgui::input;

namespace lxgui::gui {

edit_box::edit_box(utils::control_block& block, manager& mgr) :
    frame(block, mgr),
    carret_timer_(d_blink_speed_, periodic_timer::start_type::first_tick, false) {
    type_.push_back(class_name);

    iter_carret_pos_     = unicode_text_.begin();
    iter_carret_pos_old_ = unicode_text_.begin();

    enable_mouse(true);
    register_for_drag({"LeftButton"});
}

bool edit_box::can_use_script(const std::string& script_name) const {
    if (base::can_use_script(script_name))
        return true;
    else if (
        (script_name == "OnCursorChanged") || (script_name == "OnEnterPressed") ||
        (script_name == "OnEscapePressed") || (script_name == "OnSpacePressed") ||
        (script_name == "OnTabPressed") || (script_name == "OnUpPressed") ||
        (script_name == "OnDownPressed") || (script_name == "OnTextChanged") ||
        (script_name == "OnTextSet"))
        return true;
    else
        return false;
}

void edit_box::copy_from(const region& obj) {
    base::copy_from(obj);

    const edit_box* p_edit_box = down_cast<edit_box>(&obj);
    if (!p_edit_box)
        return;

    this->set_max_letters(p_edit_box->get_max_letters());
    this->set_blink_speed(p_edit_box->get_blink_speed());
    this->set_numeric_only(p_edit_box->is_numeric_only());
    this->set_positive_only(p_edit_box->is_positive_only());
    this->set_integer_only(p_edit_box->is_integer_only());
    this->enable_password_mode(p_edit_box->is_password_mode_enabled());
    this->set_multi_line(p_edit_box->is_multi_line());
    this->set_max_history_lines(p_edit_box->get_max_history_lines());
    this->set_text_insets(p_edit_box->get_text_insets());

    if (const font_string* p_fs = p_edit_box->get_font_string().get()) {
        region_core_attributes attr;
        attr.name        = p_fs->get_name();
        attr.inheritance = {p_edit_box->get_font_string()};

        auto p_font =
            this->create_layered_region<font_string>(p_fs->get_draw_layer(), std::move(attr));

        if (p_font) {
            p_font->set_special();
            p_font->notify_loaded();
            this->set_font_string(p_font);
        }
    }
}

void edit_box::update(float delta) {
    alive_checker checker(*this);

    base::update(delta);
    if (!checker.is_alive())
        return;

    if (has_focus()) {
        carret_timer_.update(delta);

        if (carret_timer_.ticks()) {
            if (!p_carret_)
                create_carret_();

            if (p_carret_) {
                if (p_carret_->is_shown())
                    p_carret_->hide();
                else
                    p_carret_->show();
            }
        }
    }

    if (iter_carret_pos_ != iter_carret_pos_old_) {
        iter_carret_pos_old_ = iter_carret_pos_;
        fire_script("OnCursorChanged");
        if (!checker.is_alive())
            return;
    }
}

void edit_box::fire_script(const std::string& script_name, const event_data& data) {
    alive_checker checker(*this);

    // Do not fire OnKeyUp/OnKeyDown events when typing
    bool bypass_event = false;
    if (has_focus() && (script_name == "OnKeyUp" || script_name == "OnKeyDown"))
        bypass_event = true;
    if (!has_focus() && (script_name == "OnChar"))
        bypass_event = true;

    if (!bypass_event) {
        base::fire_script(script_name, data);
        if (!checker.is_alive())
            return;
    }

    if (script_name == "OnKeyDown" && has_focus()) {
        key  key_id           = data.get<key>(0);
        bool shift_is_pressed = data.get<bool>(2);
        bool ctrl_is_pressed  = data.get<bool>(3);

        if (key_id == key::k_return || key_id == key::k_numpadenter) {
            fire_script("OnEnterPressed");
            if (!checker.is_alive())
                return;
        } else if (key_id == key::k_tab) {
            fire_script("OnTabPressed");
            if (!checker.is_alive())
                return;
        } else if (key_id == key::k_up) {
            fire_script("OnUpPressed");
            if (!checker.is_alive())
                return;
        } else if (key_id == key::k_down) {
            fire_script("OnDownPressed");
            if (!checker.is_alive())
                return;
        } else if (key_id == key::k_space) {
            fire_script("OnSpacePressed");
            if (!checker.is_alive())
                return;
        } else if (key_id == key::k_escape) {
            fire_script("OnEscapePressed");
            if (!checker.is_alive())
                return;
        }

        process_key_(key_id, shift_is_pressed, ctrl_is_pressed);

        if (!checker.is_alive())
            return;
    } else if (script_name == "OnChar" && has_focus()) {
        std::uint32_t c = data.get<std::uint32_t>(1);
        if (add_char_(c)) {
            fire_script("OnTextChanged");
            if (!checker.is_alive())
                return;
        }
    } else if (script_name == "OnSizeChanged") {
        update_displayed_text_();
        update_font_string_();
        update_carret_position_();
    } else if (script_name == "OnDragStart") {
        ui_selection_end_pos_ = ui_selection_start_pos_ =
            get_letter_id_at_(vector2f(data.get<float>(1), data.get<float>(2)));
    } else if (script_name == "OnDragMove") {
        std::size_t ui_pos = get_letter_id_at_(vector2f(data.get<float>(0), data.get<float>(1)));
        if (ui_pos != ui_selection_end_pos_) {
            if (ui_pos != std::numeric_limits<std::size_t>::max()) {
                highlight_text(ui_selection_start_pos_, ui_pos);
                iter_carret_pos_ = unicode_text_.begin() + ui_pos;
                update_carret_position_();
            } else {
                std::size_t ui_temp = ui_selection_start_pos_;
                unlight_text();
                ui_selection_start_pos_ = ui_temp;
                iter_carret_pos_        = unicode_text_.begin() + ui_selection_start_pos_;
                update_carret_position_();
            }
        }
    } else if (script_name == "OnMouseDown") {
        set_focus(true);
        if (!checker.is_alive())
            return;

        unlight_text();

        move_carret_at_({data.get<float>(1), data.get<float>(2)});
    }
}

void edit_box::create_glue() {
    create_glue_(this);
}

void edit_box::set_text(const utils::ustring& content) {
    if (content == unicode_text_)
        return;

    unlight_text();
    unicode_text_ = content;
    check_text_();
    update_displayed_text_();
    iter_carret_pos_ = unicode_text_.end();
    update_font_string_();
    update_carret_position_();

    alive_checker checker(*this);

    fire_script("OnTextSet");
    if (!checker.is_alive())
        return;

    fire_script("OnTextChanged");
    if (!checker.is_alive())
        return;
}

const utils::ustring& edit_box::get_text() const {
    return unicode_text_;
}

void edit_box::unlight_text() {
    ui_selection_start_pos_ = 0u;
    ui_selection_end_pos_   = 0u;
    is_text_selected_       = false;

    if (p_highlight_)
        p_highlight_->hide();
}

void edit_box::highlight_text(std::size_t ui_start, std::size_t ui_end, bool force_update) {
    if (!p_highlight_)
        create_highlight_();

    if (!p_highlight_)
        return;

    std::size_t ui_left  = std::min(ui_start, ui_end);
    std::size_t ui_right = std::max(ui_start, ui_end);

    if (ui_selection_start_pos_ != ui_start || ui_selection_end_pos_ != ui_end || force_update) {
        if (ui_left != ui_right) {
            is_text_selected_ = true;

            if (ui_right >= ui_display_pos_ && ui_left < ui_display_pos_ + displayed_text_.size() &&
                p_font_string_ && p_font_string_->get_text_object()) {
                text* p_text = p_font_string_->get_text_object();

                if (ui_left < ui_display_pos_)
                    ui_left = 0;
                else
                    ui_left = ui_left - ui_display_pos_;

                float left_pos = text_insets_.left;
                if (ui_left < p_text->get_num_letters())
                    left_pos += p_text->get_letter_quad(ui_left)[0].pos.x;

                ui_right        = ui_right - ui_display_pos_;
                float right_pos = text_insets_.left;
                if (ui_right < displayed_text_.size()) {
                    if (ui_right < p_text->get_num_letters())
                        right_pos += p_text->get_letter_quad(ui_right)[0].pos.x;
                } else {
                    ui_right = displayed_text_.size() - 1;
                    if (ui_right < p_text->get_num_letters())
                        right_pos += p_text->get_letter_quad(ui_right)[2].pos.x;
                }

                p_highlight_->set_point(anchor_point::left, name_, vector2f(left_pos, 0));
                p_highlight_->set_point(
                    anchor_point::right, name_, anchor_point::left, vector2f(right_pos, 0));

                p_highlight_->show();
            } else
                p_highlight_->hide();
        } else {
            is_text_selected_ = false;
            p_highlight_->hide();
        }
    }

    ui_selection_start_pos_ = ui_start;
    ui_selection_end_pos_   = ui_end;
}

void edit_box::set_highlight_color(const color& c) {
    if (highlight_color_ == c)
        return;

    highlight_color_ = c;

    if (!p_highlight_)
        create_highlight_();

    if (!p_highlight_)
        return;

    p_highlight_->set_solid_color(highlight_color_);
}

void edit_box::insert_after_cursor(const utils::ustring& content) {
    if (content.empty())
        return;

    if (is_numeric_only_ && !utils::is_number(content))
        return;

    if (unicode_text_.size() + content.size() <= ui_max_letters_) {
        unlight_text();
        unicode_text_.insert(iter_carret_pos_, content.begin(), content.end());
        iter_carret_pos_ += content.size();

        update_displayed_text_();
        update_font_string_();
        update_carret_position_();
    }
}

std::size_t edit_box::get_cursor_position() const {
    return iter_carret_pos_ - unicode_text_.begin();
}

void edit_box::set_cursor_position(std::size_t ui_pos) {
    if (ui_pos == get_cursor_position())
        return;

    iter_carret_pos_ = unicode_text_.begin() + ui_pos;
    update_carret_position_();
}

void edit_box::set_max_letters(std::size_t ui_max_letters) {
    if (ui_max_letters == 0) {
        ui_max_letters_ = std::numeric_limits<std::size_t>::max();
        return;
    }

    if (ui_max_letters_ != ui_max_letters) {
        ui_max_letters_ = ui_max_letters;

        std::size_t ui_carret_pos = iter_carret_pos_ - unicode_text_.begin();

        check_text_();

        if (ui_carret_pos > ui_max_letters_) {
            iter_carret_pos_ = unicode_text_.end();
            update_displayed_text_();
            update_font_string_();
            update_carret_position_();
        } else
            iter_carret_pos_ = unicode_text_.begin() + ui_carret_pos;
    }
}

std::size_t edit_box::get_max_letters() const {
    return ui_max_letters_;
}

std::size_t edit_box::get_num_letters() const {
    return unicode_text_.size();
}

void edit_box::set_blink_speed(double d_blink_speed) {
    if (d_blink_speed_ == d_blink_speed)
        return;

    d_blink_speed_ = d_blink_speed;
    carret_timer_  = periodic_timer(d_blink_speed_, periodic_timer::start_type::first_tick, false);
}

double edit_box::get_blink_speed() const {
    return d_blink_speed_;
}

void edit_box::set_numeric_only(bool numeric_only) {
    if (is_numeric_only_ == numeric_only)
        return;

    is_numeric_only_ = numeric_only;

    if (is_numeric_only_) {
        check_text_();
        iter_carret_pos_ = unicode_text_.end();
        update_displayed_text_();
        update_carret_position_();
    }
}

void edit_box::set_positive_only(bool positive_only) {
    if (is_positive_only_ == positive_only)
        return;

    is_positive_only_ = positive_only;

    if (is_numeric_only_ && is_positive_only_) {
        check_text_();
        iter_carret_pos_ = unicode_text_.end();
        update_displayed_text_();
        update_carret_position_();
    }
}

void edit_box::set_integer_only(bool integer_only) {
    if (is_integer_only_ == integer_only)
        return;

    is_integer_only_ = integer_only;

    if (is_numeric_only_ && is_integer_only_) {
        check_text_();
        iter_carret_pos_ = unicode_text_.end();
        update_displayed_text_();
        update_carret_position_();
    }
}

bool edit_box::is_numeric_only() const {
    return is_numeric_only_;
}

bool edit_box::is_positive_only() const {
    return is_positive_only_;
}

bool edit_box::is_integer_only() const {
    return is_integer_only_;
}

void edit_box::enable_password_mode(bool enable) {
    if (is_password_mode_ == enable)
        return;

    is_password_mode_ = enable;

    update_displayed_text_();
    update_font_string_();
    update_carret_position_();
}

bool edit_box::is_password_mode_enabled() const {
    return is_password_mode_;
}

void edit_box::set_multi_line(bool multi_line) {
    if (is_multi_line_ == multi_line)
        return;

    is_multi_line_ = multi_line;

    if (p_font_string_)
        p_font_string_->set_word_wrap(is_multi_line_, is_multi_line_);

    check_text_();
    iter_carret_pos_ = unicode_text_.end();
    update_displayed_text_();
    update_carret_position_();
    clear_history();
}

bool edit_box::is_multi_line() const {
    return is_multi_line_;
}

void edit_box::set_max_history_lines(std::size_t ui_max_history_lines) {
    if (ui_max_history_lines == 0) {
        ui_max_history_lines_ = std::numeric_limits<std::size_t>::max();
        return;
    }

    if (ui_max_history_lines_ != ui_max_history_lines) {
        ui_max_history_lines_ = ui_max_history_lines;

        if (history_line_list_.size() > ui_max_history_lines_) {
            history_line_list_.erase(
                history_line_list_.begin(),
                history_line_list_.begin() + (history_line_list_.size() - ui_max_history_lines_));

            ui_current_history_line_ = std::numeric_limits<std::size_t>::max();
        }
    }
}

std::size_t edit_box::get_max_history_lines() const {
    return ui_max_history_lines_;
}

void edit_box::add_history_line(const utils::ustring& history_line) {
    if (is_multi_line_)
        return;

    history_line_list_.push_back(history_line);

    if (history_line_list_.size() > ui_max_history_lines_) {
        history_line_list_.erase(
            history_line_list_.begin(),
            history_line_list_.begin() + (history_line_list_.size() - ui_max_history_lines_));
    }

    ui_current_history_line_ = std::numeric_limits<std::size_t>::max();
}

const std::vector<utils::ustring>& edit_box::get_history_lines() const {
    return history_line_list_;
}

void edit_box::clear_history() {
    history_line_list_.clear();
    ui_current_history_line_ = std::numeric_limits<std::size_t>::max();
}

void edit_box::set_arrows_ignored(bool arrows_ignored) {
    are_arrows_ignored_ = arrows_ignored;
}

void edit_box::set_text_insets(const bounds2f& insets) {
    text_insets_ = insets;

    if (p_font_string_) {
        p_font_string_->clear_all_points();
        p_font_string_->set_point(anchor_point::top_left, text_insets_.top_left());
        p_font_string_->set_point(anchor_point::bottom_right, -text_insets_.bottom_right());

        update_displayed_text_();
        update_font_string_();
        update_carret_position_();
    }
}

const bounds2f& edit_box::get_text_insets() const {
    return text_insets_;
}

void edit_box::notify_focus(bool focus) {
    if (has_focus() == focus)
        return;

    if (focus) {
        if (!p_carret_)
            create_carret_();

        if (p_carret_)
            p_carret_->show();

        carret_timer_.zero();
    } else {
        if (p_carret_)
            p_carret_->hide();

        unlight_text();
    }

    base::notify_focus(focus);
}

void edit_box::notify_scaling_factor_updated() {
    base::notify_scaling_factor_updated();

    if (p_font_string_) {
        p_font_string_->notify_scaling_factor_updated();
        create_carret_();
    }
}

void edit_box::set_font_string(utils::observer_ptr<font_string> p_font) {
    p_font_string_ = std::move(p_font);
    if (!p_font_string_)
        return;

    p_font_string_->set_word_wrap(is_multi_line_, is_multi_line_);

    p_font_string_->set_dimensions(vector2f(0, 0));
    p_font_string_->clear_all_points();

    p_font_string_->set_point(anchor_point::top_left, text_insets_.top_left());
    p_font_string_->set_point(anchor_point::bottom_right, -text_insets_.bottom_right());

    p_font_string_->enable_formatting(false);

    create_carret_();
}

void edit_box::set_font(const std::string& font_name, float height) {
    create_font_string_();

    p_font_string_->set_font(font_name, height);

    create_carret_();
}

void edit_box::create_font_string_() {
    if (p_font_string_)
        return;

    auto p_font = create_layered_region<font_string>(layer::artwork, "$parentFontString");
    if (!p_font)
        return;

    p_font->set_special();
    p_font->notify_loaded();
    set_font_string(p_font);
}

void edit_box::create_highlight_() {
    if (p_highlight_ || is_virtual())
        return;

    auto p_highlight = create_layered_region<texture>(layer::highlight, "$parentHighlight");
    if (!p_highlight)
        return;

    p_highlight->set_special();

    p_highlight->set_point(anchor_point::top, vector2f(0.0f, text_insets_.top));
    p_highlight->set_point(anchor_point::bottom, vector2f(0.0f, -text_insets_.bottom));

    p_highlight->set_solid_color(highlight_color_);

    p_highlight->notify_loaded();
    p_highlight_ = p_highlight;
}

void edit_box::create_carret_() {
    if (!p_font_string_ || !p_font_string_->get_text_object() || is_virtual())
        return;

    if (!p_carret_) {
        auto p_carret = create_layered_region<texture>(layer::highlight, "$parentCarret");
        if (!p_carret)
            return;

        p_carret->set_special();

        p_carret->set_point(
            anchor_point::center, anchor_point::left, vector2f(text_insets_.left - 1.0f, 0.0f));

        p_carret->notify_loaded();
        p_carret_ = p_carret;
    }

    quad quad = p_font_string_->get_text_object()->create_letter_quad(U'|');
    for (std::size_t i = 0; i < 4; ++i)
        quad.v[i].col = p_font_string_->get_text_color();

    p_carret_->set_quad(quad);

    update_carret_position_();
}

void edit_box::check_text_() {
    if (unicode_text_.size() > ui_max_letters_)
        unicode_text_.resize(ui_max_letters_);

    // TODO: use localizer's locale for these checks
    // https://github.com/cschreib/lxgui/issues/88
    if (is_numeric_only_ && !utils::is_number(unicode_text_)) {
        unicode_text_.clear();
        return;
    }

    if (is_integer_only_ && !utils::is_integer(unicode_text_)) {
        unicode_text_.clear();
        return;
    }

    if (is_positive_only_) {
        double d_value = 0.0;
        if (!utils::from_string(unicode_text_, d_value) || d_value < 0.0) {
            unicode_text_.clear();
            return;
        }
    }
}

void edit_box::update_displayed_text_() {
    if (!p_font_string_ || !p_font_string_->get_text_object())
        return;

    if (is_password_mode_)
        displayed_text_ = utils::ustring(unicode_text_.size(), U'*');
    else
        displayed_text_ = unicode_text_;

    if (!is_multi_line_) {
        text* p_text_object = p_font_string_->get_text_object();

        if (!std::isinf(p_text_object->get_box_width())) {
            displayed_text_.erase(0, ui_display_pos_);

            while (!displayed_text_.empty() && p_text_object->get_string_width(displayed_text_) >
                                                   p_text_object->get_box_width()) {
                displayed_text_.erase(displayed_text_.size() - 1, 1);
            }
        }
    } else {
        // TODO: implement for multiline edit box
        // https://github.com/cschreib/lxgui/issues/39
    }
}

void edit_box::update_font_string_() {
    if (!p_font_string_)
        return;

    p_font_string_->set_text(displayed_text_);

    if (is_text_selected_)
        highlight_text(ui_selection_start_pos_, ui_selection_end_pos_, true);
}

void edit_box::update_carret_position_() {
    if (!p_font_string_ || !p_font_string_->get_text_object() || !p_carret_)
        return;

    if (unicode_text_.empty()) {
        anchor_point point;
        float        offset = 0.0f;
        switch (p_font_string_->get_alignment_x()) {
        case alignment_x::left:
            point  = anchor_point::left;
            offset = text_insets_.left - 1;
            break;
        case alignment_x::center: point = anchor_point::center; break;
        case alignment_x::right:
            point  = anchor_point::right;
            offset = -text_insets_.right - 1;
            break;
        default: point = anchor_point::left; break;
        }

        p_carret_->set_point(anchor_point::center, point, vector2f(offset, 0.0f));
    } else {
        text*                    p_text = p_font_string_->get_text_object();
        utils::ustring::iterator iter_display_carret;

        if (!is_multi_line_) {
            std::size_t ui_global_pos = iter_carret_pos_ - unicode_text_.begin();

            if (ui_display_pos_ > ui_global_pos) {
                // The carret has been positioned before the start of the displayed string
                float          box_width            = p_text->get_box_width();
                float          left_string_max_size = box_width * 0.25f;
                float          left_string_size     = 0.0f;
                utils::ustring left_string;

                utils::ustring::iterator iter = iter_carret_pos_;
                while ((iter != unicode_text_.begin()) &&
                       (left_string_size < left_string_max_size)) {
                    --iter;
                    left_string.insert(left_string.begin(), *iter);
                    left_string_size = p_text->get_string_width(left_string);
                }

                ui_display_pos_ = iter - unicode_text_.begin();
                update_displayed_text_();
                update_font_string_();
            }

            std::size_t ui_carret_pos = ui_global_pos - ui_display_pos_;
            if (ui_carret_pos > displayed_text_.size()) {
                // The carret has been positioned after the end of the displayed string
                float          box_width            = p_text->get_box_width();
                float          left_string_max_size = box_width * 0.75f;
                float          left_string_size     = 0.0f;
                utils::ustring left_string;

                utils::ustring::iterator iter = iter_carret_pos_;
                while ((iter_carret_pos_ != unicode_text_.begin()) &&
                       (left_string_size < left_string_max_size)) {
                    --iter;
                    left_string.insert(left_string.begin(), *iter);
                    left_string_size = p_text->get_string_width(left_string);
                }

                ui_display_pos_ = iter - unicode_text_.begin();
                update_displayed_text_();
                update_font_string_();

                ui_carret_pos = ui_global_pos - ui_display_pos_;
            }

            iter_display_carret = displayed_text_.begin() + ui_carret_pos;
        } else {
            iter_display_carret = displayed_text_.begin() +
                                  (iter_carret_pos_ - unicode_text_.begin()) - ui_display_pos_;
        }

        float y_offset = static_cast<float>((p_text->get_num_lines() - 1)) *
                         (p_text->get_line_height() * p_text->get_line_spacing());

        std::size_t ui_index = iter_display_carret - displayed_text_.begin();

        float x_offset = text_insets_.left;
        if (ui_index < displayed_text_.size()) {
            if (ui_index < p_text->get_num_letters())
                x_offset += p_text->get_letter_quad(ui_index)[0].pos.x;
        } else {
            ui_index = displayed_text_.size() - 1;
            if (ui_index < p_text->get_num_letters())
                x_offset += p_text->get_letter_quad(ui_index)[2].pos.x;
        }

        p_carret_->set_point(
            anchor_point::center, anchor_point::left, vector2f(x_offset, y_offset));
    }

    carret_timer_.zero();
    if (has_focus())
        p_carret_->show();
    else
        p_carret_->hide();
}

bool edit_box::add_char_(char32_t c) {
    if (is_text_selected_)
        remove_char_();

    if (get_num_letters() >= ui_max_letters_)
        return false;

    // TODO: use localizer for these checks, if possible
    // https://github.com/cschreib/lxgui/issues/88
    if (is_numeric_only_) {
        if (c == U'.') {
            if (is_integer_only_)
                return false;

            if (unicode_text_.find(U'.') != utils::ustring::npos)
                return false;
        } else if (c == U'+' || c == U'-') {
            if (is_positive_only_)
                return false;

            if (iter_carret_pos_ != unicode_text_.begin() ||
                unicode_text_.find(U'+') != utils::ustring::npos ||
                unicode_text_.find(U'-') != utils::ustring::npos)
                return false;
        } else if (!utils::is_number(c))
            return false;
    }

    iter_carret_pos_ = unicode_text_.insert(iter_carret_pos_, c) + 1;

    update_displayed_text_();
    update_font_string_();
    update_carret_position_();

    if (p_carret_)
        p_carret_->show();

    carret_timer_.zero();

    return true;
}

bool edit_box::remove_char_() {
    if (is_text_selected_) {
        if (ui_selection_start_pos_ != ui_selection_end_pos_) {
            std::size_t ui_left  = std::min(ui_selection_start_pos_, ui_selection_end_pos_);
            std::size_t ui_right = std::max(ui_selection_start_pos_, ui_selection_end_pos_);

            unicode_text_.erase(ui_left, ui_right - ui_left);

            iter_carret_pos_ = unicode_text_.begin() + ui_left;
        }

        unlight_text();
    } else {
        if (iter_carret_pos_ == unicode_text_.end())
            return false;

        iter_carret_pos_ = unicode_text_.erase(iter_carret_pos_);
    }

    update_displayed_text_();
    update_font_string_();
    update_carret_position_();

    if (p_carret_)
        p_carret_->show();

    carret_timer_.zero();

    return true;
}

std::size_t edit_box::get_letter_id_at_(const vector2f& position) const {
    if (!p_font_string_ || !p_font_string_->get_text_object())
        return std::numeric_limits<std::size_t>::max();

    if (displayed_text_.empty())
        return ui_display_pos_;

    const text* p_text = p_font_string_->get_text_object();

    float local_x = position.x - border_list_.left - text_insets_.left;
    // float local_y = position.y - border_list_.top  - text_insets_.top;

    if (!is_multi_line_) {
        if (position.x < border_list_.left + text_insets_.left)
            return ui_display_pos_;
        else if (position.x > border_list_.right - text_insets_.right)
            return displayed_text_.size() + ui_display_pos_;

        std::size_t ui_num_letters =
            std::min<std::size_t>(p_text->get_num_letters(), displayed_text_.size());

        for (std::size_t ui_index = 0u; ui_index < ui_num_letters; ++ui_index) {
            const auto& quad = p_text->get_letter_quad(ui_index);
            if (local_x < 0.5f * (quad[0].pos.x + quad[2].pos.x))
                return ui_index + ui_display_pos_;
        }

        return displayed_text_.size() + ui_display_pos_;
    } else {
        // TODO: Implement for multi line edit_box
        // https://github.com/cschreib/lxgui/issues/39
        return ui_display_pos_;
    }
}

bool edit_box::move_carret_at_(const vector2f& position) {
    std::size_t ui_pos = get_letter_id_at_(position);
    if (ui_pos != std::numeric_limits<std::size_t>::max()) {
        iter_carret_pos_ = unicode_text_.begin() + ui_pos;
        update_carret_position_();
        return true;
    } else
        return false;
}

bool edit_box::move_carret_horizontally_(bool forward) {
    if (forward) {
        if (iter_carret_pos_ != unicode_text_.end()) {
            ++iter_carret_pos_;
            update_displayed_text_();
            update_carret_position_();

            if (p_carret_)
                p_carret_->show();

            carret_timer_.zero();

            return true;
        } else
            return false;
    } else {
        if (iter_carret_pos_ != unicode_text_.begin()) {
            --iter_carret_pos_;
            update_displayed_text_();
            update_carret_position_();

            if (p_carret_)
                p_carret_->show();

            carret_timer_.zero();

            return true;
        } else
            return false;
    }
}

bool edit_box::move_carret_vertically_(bool down) {
    if (is_multi_line_) {
        // TODO: Implement for multi line edit_box
        // https://github.com/cschreib/lxgui/issues/39
        return false;
    } else {
        utils::ustring::iterator iter_old = iter_carret_pos_;

        if (down)
            iter_carret_pos_ = unicode_text_.end();
        else
            iter_carret_pos_ = unicode_text_.begin();

        if (iter_old != iter_carret_pos_) {
            update_displayed_text_();
            update_carret_position_();

            if (p_carret_)
                p_carret_->show();

            carret_timer_.zero();

            return true;
        } else
            return false;
    }
}

void edit_box::process_key_(key key_id, bool shift_is_pressed, bool ctrl_is_pressed) {
    alive_checker checker(*this);

    if (key_id == key::k_return || key_id == key::k_numpadenter) {
        if (is_multi_line_) {
            if (add_char_(U'\n')) {
                event_data key_event;
                key_event.add(std::string("\n"));
                fire_script("OnChar", key_event);
                if (!checker.is_alive())
                    return;

                fire_script("OnTextChanged");
                if (!checker.is_alive())
                    return;
            }
        }
    } else if (key_id == key::k_end) {
        std::size_t ui_previous_carret_pos = get_cursor_position();
        set_cursor_position(get_num_letters());

        if (shift_is_pressed) {
            if (is_text_selected_)
                highlight_text(ui_selection_start_pos_, iter_carret_pos_ - unicode_text_.begin());
            else
                highlight_text(ui_previous_carret_pos, iter_carret_pos_ - unicode_text_.begin());
        } else
            unlight_text();

        return;
    } else if (key_id == key::k_home) {
        std::size_t ui_previous_carret_pos = get_cursor_position();
        set_cursor_position(0u);

        if (shift_is_pressed) {
            if (is_text_selected_)
                highlight_text(ui_selection_start_pos_, iter_carret_pos_ - unicode_text_.begin());
            else
                highlight_text(ui_previous_carret_pos, iter_carret_pos_ - unicode_text_.begin());
        } else
            unlight_text();

        return;
    } else if (key_id == key::k_back || key_id == key::k_delete) {
        if (is_text_selected_ || key_id == key::k_delete || move_carret_horizontally_(false)) {
            remove_char_();
            fire_script("OnTextChanged");
            if (!checker.is_alive())
                return;
        }
    } else if (
        key_id == key::k_left || key_id == key::k_right ||
        (is_multi_line_ && (key_id == key::k_up || key_id == key::k_down))) {
        if (!are_arrows_ignored_) {
            std::size_t ui_previous_carret_pos = iter_carret_pos_ - unicode_text_.begin();

            if (key_id == key::k_left || key_id == key::k_right) {
                if (is_text_selected_ && !shift_is_pressed) {
                    std::size_t ui_offset = 0;
                    if (key_id == key::k_left)
                        ui_offset = std::min(ui_selection_start_pos_, ui_selection_end_pos_);
                    else
                        ui_offset = std::max(ui_selection_start_pos_, ui_selection_end_pos_);

                    iter_carret_pos_ = unicode_text_.begin() + ui_offset;
                    update_carret_position_();
                } else
                    move_carret_horizontally_(key_id == key::k_right);
            } else {
                if (is_multi_line_)
                    move_carret_vertically_(key_id == key::k_down);
            }

            if (shift_is_pressed) {
                if (is_text_selected_) {
                    std::size_t ui_new_end_pos = iter_carret_pos_ - unicode_text_.begin();
                    if (ui_new_end_pos != ui_selection_start_pos_)
                        highlight_text(ui_selection_start_pos_, ui_new_end_pos);
                    else
                        unlight_text();
                } else
                    highlight_text(
                        ui_previous_carret_pos, iter_carret_pos_ - unicode_text_.begin());
            } else
                unlight_text();
        }
    } else if (
        !is_multi_line_ && (key_id == key::k_up || key_id == key::k_down) &&
        !history_line_list_.empty()) {
        if (key_id == key::k_up) {
            if (ui_current_history_line_ != 0u) {
                if (ui_current_history_line_ == std::numeric_limits<std::size_t>::max())
                    ui_current_history_line_ = history_line_list_.size() - 1;
                else
                    --ui_current_history_line_;

                set_text(history_line_list_[ui_current_history_line_]);
                if (!checker.is_alive())
                    return;
            }
        } else {
            if (ui_current_history_line_ != std::numeric_limits<std::size_t>::max()) {
                if (ui_current_history_line_ + 1 == history_line_list_.size()) {
                    ui_current_history_line_ = std::numeric_limits<std::size_t>::max();
                    set_text(U"");
                    if (!checker.is_alive())
                        return;
                } else {
                    ++ui_current_history_line_;
                    set_text(history_line_list_[ui_current_history_line_]);
                    if (!checker.is_alive())
                        return;
                }
            }
        }
    } else if (key_id == key::k_c && ctrl_is_pressed) {
        if (ui_selection_end_pos_ != ui_selection_start_pos_) {
            std::size_t    ui_min_pos = std::min(ui_selection_start_pos_, ui_selection_end_pos_);
            std::size_t    ui_max_pos = std::max(ui_selection_start_pos_, ui_selection_end_pos_);
            utils::ustring selected   = unicode_text_.substr(ui_min_pos, ui_max_pos - ui_min_pos);
            get_manager().get_window().set_clipboard_content(selected);
        }
    } else if (key_id == key::k_v && ctrl_is_pressed) {
        for (char32_t c : get_manager().get_window().get_clipboard_content()) {
            if (!add_char_(c))
                break;
            if (!checker.is_alive())
                return;
        }
    }
}

periodic_timer::periodic_timer(double d_duration, start_type type, bool ticks_now) :
    d_elapsed_(ticks_now ? d_duration : 0.0), d_duration_(d_duration), type_(type) {
    if (type == start_type::now)
        start();
}

double periodic_timer::get_elapsed() const {
    return d_elapsed_;
}

double periodic_timer::get_period() const {
    return d_duration_;
}

bool periodic_timer::is_paused() const {
    return paused_;
}

bool periodic_timer::ticks() {
    if (type_ == start_type::first_tick && first_tick_) {
        start();
        first_tick_ = false;
    }

    if (d_elapsed_ >= d_duration_) {
        if (!paused_)
            zero();

        return true;
    } else
        return false;
}

void periodic_timer::stop() {
    d_elapsed_ = 0.0;
    paused_    = true;
}

void periodic_timer::pause() {
    paused_ = true;
}

void periodic_timer::start() {
    paused_ = false;
}

void periodic_timer::zero() {
    d_elapsed_ = 0.0;
}

void periodic_timer::update(double d_delta) {
    d_elapsed_ += d_delta;
}

} // namespace lxgui::gui
