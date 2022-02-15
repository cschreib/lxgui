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

edit_box::edit_box(utils::control_block& m_block, manager& m_manager) :
    frame(m_block, m_manager),
    m_carret_timer_(d_blink_speed_, periodic_timer::start_type::first_tick, false) {
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

void edit_box::copy_from(const region& m_obj) {
    base::copy_from(m_obj);

    const edit_box* p_edit_box = down_cast<edit_box>(&m_obj);
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
        region_core_attributes m_attr;
        m_attr.name        = p_fs->get_name();
        m_attr.inheritance = {p_edit_box->get_font_string()};

        auto p_font =
            this->create_layered_region<font_string>(p_fs->get_draw_layer(), std::move(m_attr));

        if (p_font) {
            p_font->set_special();
            p_font->notify_loaded();
            this->set_font_string(p_font);
        }
    }
}

void edit_box::update(float f_delta) {
    alive_checker m_checker(*this);

    base::update(f_delta);
    if (!m_checker.is_alive())
        return;

    if (b_focus_) {
        m_carret_timer_.update(f_delta);

        if (m_carret_timer_.ticks()) {
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
        if (!m_checker.is_alive())
            return;
    }
}

void edit_box::fire_script(const std::string& script_name, const event_data& m_data) {
    alive_checker m_checker(*this);

    // Do not fire OnKeyUp/OnKeyDown events when typing
    bool b_bypass_event = false;
    if (has_focus() && (script_name == "OnKeyUp" || script_name == "OnKeyDown"))
        b_bypass_event = true;
    if (!has_focus() && (script_name == "OnChar"))
        b_bypass_event = true;

    if (!b_bypass_event) {
        base::fire_script(script_name, m_data);
        if (!m_checker.is_alive())
            return;
    }

    if (script_name == "OnKeyDown" && has_focus()) {
        key  m_key              = m_data.get<key>(0);
        bool b_shift_is_pressed = m_data.get<bool>(2);
        bool b_ctrl_is_pressed  = m_data.get<bool>(3);

        if (m_key == key::k_return || m_key == key::k_numpadenter) {
            fire_script("OnEnterPressed");
            if (!m_checker.is_alive())
                return;
        } else if (m_key == key::k_tab) {
            fire_script("OnTabPressed");
            if (!m_checker.is_alive())
                return;
        } else if (m_key == key::k_up) {
            fire_script("OnUpPressed");
            if (!m_checker.is_alive())
                return;
        } else if (m_key == key::k_down) {
            fire_script("OnDownPressed");
            if (!m_checker.is_alive())
                return;
        } else if (m_key == key::k_space) {
            fire_script("OnSpacePressed");
            if (!m_checker.is_alive())
                return;
        } else if (m_key == key::k_escape) {
            fire_script("OnEscapePressed");
            if (!m_checker.is_alive())
                return;
        }

        process_key_(m_key, b_shift_is_pressed, b_ctrl_is_pressed);

        if (!m_checker.is_alive())
            return;
    } else if (script_name == "OnChar" && has_focus()) {
        std::uint32_t c = m_data.get<std::uint32_t>(1);
        if (add_char_(c)) {
            fire_script("OnTextChanged");
            if (!m_checker.is_alive())
                return;
        }
    } else if (script_name == "OnSizeChanged") {
        update_displayed_text_();
        update_font_string_();
        update_carret_position_();
    } else if (script_name == "OnDragStart") {
        ui_selection_end_pos_ = ui_selection_start_pos_ =
            get_letter_id_at_(vector2f(m_data.get<float>(1), m_data.get<float>(2)));
    } else if (script_name == "OnDragMove") {
        std::size_t ui_pos =
            get_letter_id_at_(vector2f(m_data.get<float>(0), m_data.get<float>(1)));
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
        if (!m_checker.is_alive())
            return;

        unlight_text();

        move_carret_at_({m_data.get<float>(1), m_data.get<float>(2)});
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

    alive_checker m_checker(*this);

    fire_script("OnTextSet");
    if (!m_checker.is_alive())
        return;

    fire_script("OnTextChanged");
    if (!m_checker.is_alive())
        return;
}

const utils::ustring& edit_box::get_text() const {
    return unicode_text_;
}

void edit_box::unlight_text() {
    ui_selection_start_pos_ = ui_selection_end_pos_ = 0u;
    b_selected_text_                                = false;

    if (p_highlight_)
        p_highlight_->hide();
}

void edit_box::highlight_text(std::size_t ui_start, std::size_t ui_end, bool b_force_update) {
    if (!p_highlight_)
        create_highlight_();

    if (!p_highlight_)
        return;

    std::size_t ui_left  = std::min(ui_start, ui_end);
    std::size_t ui_right = std::max(ui_start, ui_end);

    if (ui_selection_start_pos_ != ui_start || ui_selection_end_pos_ != ui_end || b_force_update) {
        if (ui_left != ui_right) {
            b_selected_text_ = true;

            if (ui_right >= ui_display_pos_ && ui_left < ui_display_pos_ + displayed_text_.size() &&
                p_font_string_ && p_font_string_->get_text_object()) {
                text* p_text = p_font_string_->get_text_object();

                if (ui_left < ui_display_pos_)
                    ui_left = 0;
                else
                    ui_left = ui_left - ui_display_pos_;

                float f_left_pos = text_insets_.left;
                if (ui_left < p_text->get_num_letters())
                    f_left_pos += p_text->get_letter_quad(ui_left)[0].pos.x;

                ui_right          = ui_right - ui_display_pos_;
                float f_right_pos = text_insets_.left;
                if (ui_right < displayed_text_.size()) {
                    if (ui_right < p_text->get_num_letters())
                        f_right_pos += p_text->get_letter_quad(ui_right)[0].pos.x;
                } else {
                    ui_right = displayed_text_.size() - 1;
                    if (ui_right < p_text->get_num_letters())
                        f_right_pos += p_text->get_letter_quad(ui_right)[2].pos.x;
                }

                p_highlight_->set_point(anchor_point::left, name_, vector2f(f_left_pos, 0));
                p_highlight_->set_point(
                    anchor_point::right, name_, anchor_point::left, vector2f(f_right_pos, 0));

                p_highlight_->show();
            } else
                p_highlight_->hide();
        } else {
            b_selected_text_ = false;
            p_highlight_->hide();
        }
    }

    ui_selection_start_pos_ = ui_start;
    ui_selection_end_pos_   = ui_end;
}

void edit_box::set_highlight_color(const color& m_color) {
    if (m_highlight_color_ == m_color)
        return;

    m_highlight_color_ = m_color;

    if (!p_highlight_)
        create_highlight_();

    if (!p_highlight_)
        return;

    p_highlight_->set_solid_color(m_highlight_color_);
}

void edit_box::insert_after_cursor(const utils::ustring& content) {
    if (content.empty())
        return;

    if (b_numeric_only_ && !utils::is_number(content))
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

    d_blink_speed_  = d_blink_speed;
    m_carret_timer_ = periodic_timer(d_blink_speed_, periodic_timer::start_type::first_tick, false);
}

double edit_box::get_blink_speed() const {
    return d_blink_speed_;
}

void edit_box::set_numeric_only(bool b_numeric_only) {
    if (b_numeric_only_ == b_numeric_only)
        return;

    b_numeric_only_ = b_numeric_only;

    if (b_numeric_only_) {
        check_text_();
        iter_carret_pos_ = unicode_text_.end();
        update_displayed_text_();
        update_carret_position_();
    }
}

void edit_box::set_positive_only(bool b_positive_only) {
    if (b_positive_only_ == b_positive_only)
        return;

    b_positive_only_ = b_positive_only;

    if (b_numeric_only_ && b_positive_only_) {
        check_text_();
        iter_carret_pos_ = unicode_text_.end();
        update_displayed_text_();
        update_carret_position_();
    }
}

void edit_box::set_integer_only(bool b_integer_only) {
    if (b_integer_only_ == b_integer_only)
        return;

    b_integer_only_ = b_integer_only;

    if (b_numeric_only_ && b_integer_only_) {
        check_text_();
        iter_carret_pos_ = unicode_text_.end();
        update_displayed_text_();
        update_carret_position_();
    }
}

bool edit_box::is_numeric_only() const {
    return b_numeric_only_;
}

bool edit_box::is_positive_only() const {
    return b_positive_only_;
}

bool edit_box::is_integer_only() const {
    return b_integer_only_;
}

void edit_box::enable_password_mode(bool b_enable) {
    if (b_password_mode_ == b_enable)
        return;

    b_password_mode_ = b_enable;

    update_displayed_text_();
    update_font_string_();
    update_carret_position_();
}

bool edit_box::is_password_mode_enabled() const {
    return b_password_mode_;
}

void edit_box::set_multi_line(bool b_multi_line) {
    if (b_multi_line_ == b_multi_line)
        return;

    b_multi_line_ = b_multi_line;

    if (p_font_string_)
        p_font_string_->set_word_wrap(b_multi_line_, b_multi_line_);

    check_text_();
    iter_carret_pos_ = unicode_text_.end();
    update_displayed_text_();
    update_carret_position_();
    clear_history();
}

bool edit_box::is_multi_line() const {
    return b_multi_line_;
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
    if (b_multi_line_)
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

void edit_box::set_arrows_ignored(bool b_arrows_ignored) {
    b_arrows_ignored_ = b_arrows_ignored;
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

void edit_box::notify_focus(bool b_focus) {
    if (b_focus_ == b_focus)
        return;

    if (b_focus) {
        if (!p_carret_)
            create_carret_();

        if (p_carret_)
            p_carret_->show();

        m_carret_timer_.zero();
    } else {
        if (p_carret_)
            p_carret_->hide();

        unlight_text();
    }

    base::notify_focus(b_focus);
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

    p_font_string_->set_word_wrap(b_multi_line_, b_multi_line_);

    p_font_string_->set_dimensions(vector2f(0, 0));
    p_font_string_->clear_all_points();

    p_font_string_->set_point(anchor_point::top_left, text_insets_.top_left());
    p_font_string_->set_point(anchor_point::bottom_right, -text_insets_.bottom_right());

    p_font_string_->enable_formatting(false);

    create_carret_();
}

void edit_box::set_font(const std::string& font_name, float f_height) {
    create_font_string_();

    p_font_string_->set_font(font_name, f_height);

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

    p_highlight->set_solid_color(m_highlight_color_);

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

    quad m_quad = p_font_string_->get_text_object()->create_letter_quad(U'|');
    for (std::size_t i = 0; i < 4; ++i)
        m_quad.v[i].col = p_font_string_->get_text_color();

    p_carret_->set_quad(m_quad);

    update_carret_position_();
}

void edit_box::check_text_() {
    if (unicode_text_.size() > ui_max_letters_)
        unicode_text_.resize(ui_max_letters_);

    // TODO: use localizer's locale for these checks
    // https://github.com/cschreib/lxgui/issues/88
    if (b_numeric_only_ && !utils::is_number(unicode_text_)) {
        unicode_text_.clear();
        return;
    }

    if (b_integer_only_ && !utils::is_integer(unicode_text_)) {
        unicode_text_.clear();
        return;
    }

    if (b_positive_only_) {
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

    if (b_password_mode_)
        displayed_text_ = utils::ustring(unicode_text_.size(), U'*');
    else
        displayed_text_ = unicode_text_;

    if (!b_multi_line_) {
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

    if (b_selected_text_)
        highlight_text(ui_selection_start_pos_, ui_selection_end_pos_, true);
}

void edit_box::update_carret_position_() {
    if (!p_font_string_ || !p_font_string_->get_text_object() || !p_carret_)
        return;

    if (unicode_text_.empty()) {
        anchor_point m_point;
        float        f_offset = 0.0f;
        switch (p_font_string_->get_alignment_x()) {
        case alignment_x::left:
            m_point  = anchor_point::left;
            f_offset = text_insets_.left - 1;
            break;
        case alignment_x::center: m_point = anchor_point::center; break;
        case alignment_x::right:
            m_point  = anchor_point::right;
            f_offset = -text_insets_.right - 1;
            break;
        default: m_point = anchor_point::left; break;
        }

        p_carret_->set_point(anchor_point::center, m_point, vector2f(f_offset, 0.0f));
    } else {
        text*                    p_text = p_font_string_->get_text_object();
        utils::ustring::iterator iter_display_carret;

        if (!b_multi_line_) {
            std::size_t ui_global_pos = iter_carret_pos_ - unicode_text_.begin();

            if (ui_display_pos_ > ui_global_pos) {
                // The carret has been positioned before the start of the displayed string
                float          f_box_width            = p_text->get_box_width();
                float          f_left_string_max_size = f_box_width * 0.25f;
                float          f_left_string_size     = 0.0f;
                utils::ustring left_string;

                utils::ustring::iterator iter = iter_carret_pos_;
                while ((iter != unicode_text_.begin()) &&
                       (f_left_string_size < f_left_string_max_size)) {
                    --iter;
                    left_string.insert(left_string.begin(), *iter);
                    f_left_string_size = p_text->get_string_width(left_string);
                }

                ui_display_pos_ = iter - unicode_text_.begin();
                update_displayed_text_();
                update_font_string_();
            }

            std::size_t ui_carret_pos = ui_global_pos - ui_display_pos_;
            if (ui_carret_pos > displayed_text_.size()) {
                // The carret has been positioned after the end of the displayed string
                float          f_box_width            = p_text->get_box_width();
                float          f_left_string_max_size = f_box_width * 0.75f;
                float          f_left_string_size     = 0.0f;
                utils::ustring left_string;

                utils::ustring::iterator iter = iter_carret_pos_;
                while ((iter_carret_pos_ != unicode_text_.begin()) &&
                       (f_left_string_size < f_left_string_max_size)) {
                    --iter;
                    left_string.insert(left_string.begin(), *iter);
                    f_left_string_size = p_text->get_string_width(left_string);
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

        float f_y_offset = static_cast<float>((p_text->get_num_lines() - 1)) *
                           (p_text->get_line_height() * p_text->get_line_spacing());

        std::size_t ui_index = iter_display_carret - displayed_text_.begin();

        float f_x_offset = text_insets_.left;
        if (ui_index < displayed_text_.size()) {
            if (ui_index < p_text->get_num_letters())
                f_x_offset += p_text->get_letter_quad(ui_index)[0].pos.x;
        } else {
            ui_index = displayed_text_.size() - 1;
            if (ui_index < p_text->get_num_letters())
                f_x_offset += p_text->get_letter_quad(ui_index)[2].pos.x;
        }

        p_carret_->set_point(
            anchor_point::center, anchor_point::left, vector2f(f_x_offset, f_y_offset));
    }

    m_carret_timer_.zero();
    if (b_focus_)
        p_carret_->show();
    else
        p_carret_->hide();
}

bool edit_box::add_char_(char32_t c) {
    if (b_selected_text_)
        remove_char_();

    if (get_num_letters() >= ui_max_letters_)
        return false;

    // TODO: use localizer for these checks, if possible
    // https://github.com/cschreib/lxgui/issues/88
    if (b_numeric_only_) {
        if (c == U'.') {
            if (b_integer_only_)
                return false;

            if (unicode_text_.find(U'.') != utils::ustring::npos)
                return false;
        } else if (c == U'+' || c == U'-') {
            if (b_positive_only_)
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

    m_carret_timer_.zero();

    return true;
}

bool edit_box::remove_char_() {
    if (b_selected_text_) {
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

    m_carret_timer_.zero();

    return true;
}

std::size_t edit_box::get_letter_id_at_(const vector2f& m_position) const {
    if (!p_font_string_ || !p_font_string_->get_text_object())
        return std::numeric_limits<std::size_t>::max();

    if (displayed_text_.empty())
        return ui_display_pos_;

    const text* p_text = p_font_string_->get_text_object();

    float f_local_x = m_position.x - border_list_.left - text_insets_.left;
    // float fLocalY = mPosition.y - lBorderList_.top  - lTextInsets_.top;

    if (!b_multi_line_) {
        if (m_position.x < border_list_.left + text_insets_.left)
            return ui_display_pos_;
        else if (m_position.x > border_list_.right - text_insets_.right)
            return displayed_text_.size() + ui_display_pos_;

        std::size_t ui_num_letters =
            std::min<std::size_t>(p_text->get_num_letters(), displayed_text_.size());

        for (std::size_t ui_index = 0u; ui_index < ui_num_letters; ++ui_index) {
            const auto& m_quad = p_text->get_letter_quad(ui_index);
            if (f_local_x < 0.5f * (m_quad[0].pos.x + m_quad[2].pos.x))
                return ui_index + ui_display_pos_;
        }

        return displayed_text_.size() + ui_display_pos_;
    } else {
        // TODO: Implement for multi line edit_box
        // https://github.com/cschreib/lxgui/issues/39
        return ui_display_pos_;
    }
}

bool edit_box::move_carret_at_(const vector2f& m_position) {
    std::size_t ui_pos = get_letter_id_at_(m_position);
    if (ui_pos != std::numeric_limits<std::size_t>::max()) {
        iter_carret_pos_ = unicode_text_.begin() + ui_pos;
        update_carret_position_();
        return true;
    } else
        return false;
}

bool edit_box::move_carret_horizontally_(bool b_forward) {
    if (b_forward) {
        if (iter_carret_pos_ != unicode_text_.end()) {
            ++iter_carret_pos_;
            update_displayed_text_();
            update_carret_position_();

            if (p_carret_)
                p_carret_->show();

            m_carret_timer_.zero();

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

            m_carret_timer_.zero();

            return true;
        } else
            return false;
    }
}

bool edit_box::move_carret_vertically_(bool b_down) {
    if (b_multi_line_) {
        // TODO: Implement for multi line edit_box
        // https://github.com/cschreib/lxgui/issues/39
        return false;
    } else {
        utils::ustring::iterator iter_old = iter_carret_pos_;

        if (b_down)
            iter_carret_pos_ = unicode_text_.end();
        else
            iter_carret_pos_ = unicode_text_.begin();

        if (iter_old != iter_carret_pos_) {
            update_displayed_text_();
            update_carret_position_();

            if (p_carret_)
                p_carret_->show();

            m_carret_timer_.zero();

            return true;
        } else
            return false;
    }
}

void edit_box::process_key_(key m_key, bool b_shift_is_pressed, bool b_ctrl_is_pressed) {
    alive_checker m_checker(*this);

    if (m_key == key::k_return || m_key == key::k_numpadenter) {
        if (b_multi_line_) {
            if (add_char_(U'\n')) {
                event_data m_key_event;
                m_key_event.add(std::string("\n"));
                fire_script("OnChar", m_key_event);
                if (!m_checker.is_alive())
                    return;

                fire_script("OnTextChanged");
                if (!m_checker.is_alive())
                    return;
            }
        }
    } else if (m_key == key::k_end) {
        std::size_t ui_previous_carret_pos = get_cursor_position();
        set_cursor_position(get_num_letters());

        if (b_shift_is_pressed) {
            if (b_selected_text_)
                highlight_text(ui_selection_start_pos_, iter_carret_pos_ - unicode_text_.begin());
            else
                highlight_text(ui_previous_carret_pos, iter_carret_pos_ - unicode_text_.begin());
        } else
            unlight_text();

        return;
    } else if (m_key == key::k_home) {
        std::size_t ui_previous_carret_pos = get_cursor_position();
        set_cursor_position(0u);

        if (b_shift_is_pressed) {
            if (b_selected_text_)
                highlight_text(ui_selection_start_pos_, iter_carret_pos_ - unicode_text_.begin());
            else
                highlight_text(ui_previous_carret_pos, iter_carret_pos_ - unicode_text_.begin());
        } else
            unlight_text();

        return;
    } else if (m_key == key::k_back || m_key == key::k_delete) {
        if (b_selected_text_ || m_key == key::k_delete || move_carret_horizontally_(false)) {
            remove_char_();
            fire_script("OnTextChanged");
            if (!m_checker.is_alive())
                return;
        }
    } else if (
        m_key == key::k_left || m_key == key::k_right ||
        (b_multi_line_ && (m_key == key::k_up || m_key == key::k_down))) {
        if (!b_arrows_ignored_) {
            std::size_t ui_previous_carret_pos = iter_carret_pos_ - unicode_text_.begin();

            if (m_key == key::k_left || m_key == key::k_right) {
                if (b_selected_text_ && !b_shift_is_pressed) {
                    std::size_t ui_offset = 0;
                    if (m_key == key::k_left)
                        ui_offset = std::min(ui_selection_start_pos_, ui_selection_end_pos_);
                    else
                        ui_offset = std::max(ui_selection_start_pos_, ui_selection_end_pos_);

                    iter_carret_pos_ = unicode_text_.begin() + ui_offset;
                    update_carret_position_();
                } else
                    move_carret_horizontally_(m_key == key::k_right);
            } else {
                if (b_multi_line_)
                    move_carret_vertically_(m_key == key::k_down);
            }

            if (b_shift_is_pressed) {
                if (b_selected_text_) {
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
        !b_multi_line_ && (m_key == key::k_up || m_key == key::k_down) &&
        !history_line_list_.empty()) {
        if (m_key == key::k_up) {
            if (ui_current_history_line_ != 0u) {
                if (ui_current_history_line_ == std::numeric_limits<std::size_t>::max())
                    ui_current_history_line_ = history_line_list_.size() - 1;
                else
                    --ui_current_history_line_;

                set_text(history_line_list_[ui_current_history_line_]);
                if (!m_checker.is_alive())
                    return;
            }
        } else {
            if (ui_current_history_line_ != std::numeric_limits<std::size_t>::max()) {
                if (ui_current_history_line_ + 1 == history_line_list_.size()) {
                    ui_current_history_line_ = std::numeric_limits<std::size_t>::max();
                    set_text(U"");
                    if (!m_checker.is_alive())
                        return;
                } else {
                    ++ui_current_history_line_;
                    set_text(history_line_list_[ui_current_history_line_]);
                    if (!m_checker.is_alive())
                        return;
                }
            }
        }
    } else if (m_key == key::k_c && b_ctrl_is_pressed) {
        if (ui_selection_end_pos_ != ui_selection_start_pos_) {
            std::size_t    ui_min_pos = std::min(ui_selection_start_pos_, ui_selection_end_pos_);
            std::size_t    ui_max_pos = std::max(ui_selection_start_pos_, ui_selection_end_pos_);
            utils::ustring selected   = unicode_text_.substr(ui_min_pos, ui_max_pos - ui_min_pos);
            get_manager().get_window().set_clipboard_content(selected);
        }
    } else if (m_key == key::k_v && b_ctrl_is_pressed) {
        for (char32_t c_char : get_manager().get_window().get_clipboard_content()) {
            if (!add_char_(c_char))
                break;
            if (!m_checker.is_alive())
                return;
        }
    }
}

periodic_timer::periodic_timer(double d_duration, start_type m_type, bool b_tick_first) :
    d_elapsed_(b_tick_first ? d_duration : 0.0), d_duration_(d_duration), m_type_(m_type) {
    if (m_type == start_type::now)
        start();
}

double periodic_timer::get_elapsed() const {
    return d_elapsed_;
}

double periodic_timer::get_period() const {
    return d_duration_;
}

bool periodic_timer::is_paused() const {
    return b_paused_;
}

bool periodic_timer::ticks() {
    if (m_type_ == start_type::first_tick && b_first_tick_) {
        start();
        b_first_tick_ = false;
    }

    if (d_elapsed_ >= d_duration_) {
        if (!b_paused_)
            zero();

        return true;
    } else
        return false;
}

void periodic_timer::stop() {
    d_elapsed_ = 0.0;
    b_paused_  = true;
}

void periodic_timer::pause() {
    b_paused_ = true;
}

void periodic_timer::start() {
    b_paused_ = false;
}

void periodic_timer::zero() {
    d_elapsed_ = 0.0;
}

void periodic_timer::update(double d_delta) {
    d_elapsed_ += d_delta;
}

} // namespace lxgui::gui
