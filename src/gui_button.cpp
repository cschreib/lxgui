#include "lxgui/gui_button.hpp"

#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"

namespace lxgui::gui {

button::button(utils::control_block& m_block, manager& m_manager) : frame(m_block, m_manager) {
    l_type_.push_back(class_name);
    enable_mouse(true);
}

std::string button::serialize(const std::string& s_tab) const {
    return base::serialize(s_tab);
}

void button::create_glue() {
    create_glue_(this);
}

bool button::can_use_script(const std::string& s_script_name) const {
    if (base::can_use_script(s_script_name))
        return true;
    else if (
        (s_script_name == "OnClick") || (s_script_name == "OnEnable") || (s_script_name == "OnDisable"))
        return true;
    else
        return false;
}

void button::fire_script(const std::string& s_script_name, const event_data& m_data) {
    alive_checker m_checker(*this);
    base::fire_script(s_script_name, m_data);
    if (!m_checker.is_alive())
        return;

    if (is_enabled()) {
        if (s_script_name == "OnEnter")
            highlight();

        if (s_script_name == "OnLeave") {
            unlight();

            if (m_state_ == state::down)
                release();
        }

        if (s_script_name == "OnMouseDown") {
            push();
            fire_script("OnClick");
            if (!m_checker.is_alive())
                return;
        }

        if (s_script_name == "OnMouseUp")
            release();
    }
}

void button::copy_from(const region& m_obj) {
    base::copy_from(m_obj);

    const button* p_button = down_cast<button>(&m_obj);
    if (!p_button)
        return;

    this->set_text(p_button->get_text());

    if (const texture* p_other_texture = p_button->get_normal_texture().get()) {
        region_core_attributes m_attr;
        m_attr.s_name        = p_other_texture->get_name();
        m_attr.l_inheritance = {p_button->get_normal_texture()};

        auto p_texture =
            this->create_layered_region<texture>(p_other_texture->get_draw_layer(), std::move(m_attr));

        if (p_texture) {
            p_texture->set_special();
            p_texture->notify_loaded();
            this->set_normal_texture(p_texture);
        }
    }

    if (const texture* p_other_texture = p_button->get_pushed_texture().get()) {
        region_core_attributes m_attr;
        m_attr.s_name        = p_other_texture->get_name();
        m_attr.l_inheritance = {p_button->get_pushed_texture()};

        auto p_texture =
            this->create_layered_region<texture>(p_other_texture->get_draw_layer(), std::move(m_attr));

        if (p_texture) {
            p_texture->set_special();
            p_texture->notify_loaded();
            this->set_pushed_texture(p_texture);
        }
    }

    if (const texture* p_other_texture = p_button->get_highlight_texture().get()) {
        region_core_attributes m_attr;
        m_attr.s_name        = p_other_texture->get_name();
        m_attr.l_inheritance = {p_button->get_highlight_texture()};

        auto p_texture =
            this->create_layered_region<texture>(p_other_texture->get_draw_layer(), std::move(m_attr));

        if (p_texture) {
            p_texture->set_special();
            p_texture->notify_loaded();
            this->set_highlight_texture(p_texture);
        }
    }

    if (const texture* p_other_texture = p_button->get_disabled_texture().get()) {
        region_core_attributes m_attr;
        m_attr.s_name        = p_other_texture->get_name();
        m_attr.l_inheritance = {p_button->get_disabled_texture()};

        auto p_texture =
            this->create_layered_region<texture>(p_other_texture->get_draw_layer(), std::move(m_attr));

        if (p_texture) {
            p_texture->set_special();
            p_texture->notify_loaded();
            this->set_disabled_texture(p_texture);
        }
    }

    if (const font_string* p_other_text = p_button->get_normal_text().get()) {
        region_core_attributes m_attr;
        m_attr.s_name        = p_other_text->get_name();
        m_attr.l_inheritance = {p_button->get_normal_text()};

        auto p_font = this->create_layered_region<font_string>(
            p_other_text->get_draw_layer(), std::move(m_attr));

        if (p_font) {
            p_font->set_special();
            p_font->notify_loaded();
            this->set_normal_text(p_font);
        }
    }

    if (const font_string* p_other_text = p_button->get_highlight_text().get()) {
        region_core_attributes m_attr;
        m_attr.s_name        = p_other_text->get_name();
        m_attr.l_inheritance = {p_button->get_highlight_text()};

        auto p_font = this->create_layered_region<font_string>(
            p_other_text->get_draw_layer(), std::move(m_attr));

        if (p_font) {
            p_font->set_special();
            p_font->notify_loaded();
            this->set_highlight_text(p_font);
        }
    }

    if (const font_string* p_other_text = p_button->get_disabled_text().get()) {
        region_core_attributes m_attr;
        m_attr.s_name        = p_other_text->get_name();
        m_attr.l_inheritance = {p_button->get_disabled_text()};

        auto p_font = this->create_layered_region<font_string>(
            p_other_text->get_draw_layer(), std::move(m_attr));

        if (p_font) {
            p_font->set_special();
            p_font->notify_loaded();
            this->set_disabled_text(p_font);
        }
    }

    this->set_pushed_text_offset(p_button->get_pushed_text_offset());

    if (!p_button->is_enabled())
        this->disable();
}

void button::set_text(const utils::ustring& s_text) {
    s_text_ = s_text;

    if (p_normal_text_)
        p_normal_text_->set_text(s_text);

    if (p_highlight_text_)
        p_highlight_text_->set_text(s_text);

    if (p_disabled_text_)
        p_disabled_text_->set_text(s_text);
}

const utils::ustring& button::get_text() const {
    return s_text_;
}

void button::set_normal_texture(utils::observer_ptr<texture> p_texture) {
    p_normal_texture_ = std::move(p_texture);
    if (!p_normal_texture_)
        return;

    p_normal_texture_->set_shown(m_state_ == state::up);
}

void button::set_pushed_texture(utils::observer_ptr<texture> p_texture) {
    p_pushed_texture_ = std::move(p_texture);
    if (!p_pushed_texture_)
        return;

    p_pushed_texture_->set_shown(m_state_ == state::down);
}

void button::set_disabled_texture(utils::observer_ptr<texture> p_texture) {
    p_disabled_texture_ = std::move(p_texture);
    if (!p_disabled_texture_)
        return;

    p_disabled_texture_->set_shown(m_state_ == state::disabled);
}

void button::set_highlight_texture(utils::observer_ptr<texture> p_texture) {
    p_highlight_texture_ = std::move(p_texture);
    if (!p_highlight_texture_)
        return;

    p_highlight_texture_->set_shown(b_highlighted_);
}

void button::set_normal_text(utils::observer_ptr<font_string> p_font) {
    if (p_normal_text_ == p_current_font_string_)
        p_current_font_string_ = p_font;

    p_normal_text_ = std::move(p_font);
    if (!p_normal_text_)
        return;

    p_normal_text_->set_shown(m_state_ == state::up);
    p_normal_text_->set_text(s_text_);
}

void button::set_highlight_text(utils::observer_ptr<font_string> p_font) {
    if (p_highlight_text_ == p_current_font_string_)
        p_current_font_string_ = p_font;

    p_highlight_text_ = std::move(p_font);
    if (!p_highlight_text_)
        return;

    p_highlight_text_->set_shown(b_highlighted_);
    p_highlight_text_->set_text(s_text_);
}

void button::set_disabled_text(utils::observer_ptr<font_string> p_font) {
    if (p_disabled_text_ == p_current_font_string_)
        p_current_font_string_ = p_font;

    p_disabled_text_ = std::move(p_font);
    if (!p_disabled_text_)
        return;

    p_disabled_text_->set_shown(m_state_ == state::disabled);
    p_disabled_text_->set_text(s_text_);
}

void button::disable() {
    if (is_enabled()) {
        m_state_ = state::disabled;
        if (p_disabled_texture_) {
            if (p_normal_texture_)
                p_normal_texture_->hide();
            if (p_pushed_texture_)
                p_pushed_texture_->hide();

            p_disabled_texture_->show();
        } else {
            if (p_normal_texture_)
                p_normal_texture_->show();
            if (p_pushed_texture_)
                p_pushed_texture_->hide();
        }

        if (p_disabled_text_) {
            if (p_normal_text_)
                p_normal_text_->hide();

            p_disabled_text_->show();
            p_current_font_string_ = p_disabled_text_;
        } else {
            if (p_normal_text_)
                p_normal_text_->show();

            p_current_font_string_ = p_normal_text_;
        }

        unlight();

        alive_checker m_checker(*this);
        fire_script("OnDisable");
        if (!m_checker.is_alive())
            return;
    }
}

void button::enable() {
    if (!is_enabled()) {
        m_state_ = state::up;
        if (p_disabled_texture_) {
            if (p_normal_texture_)
                p_normal_texture_->show();
            if (p_pushed_texture_)
                p_pushed_texture_->hide();

            p_disabled_texture_->hide();
        } else {
            if (p_normal_texture_)
                p_normal_texture_->show();
            if (p_pushed_texture_)
                p_pushed_texture_->hide();
        }

        if (p_normal_text_)
            p_normal_text_->show();

        p_current_font_string_ = p_normal_text_;

        if (p_disabled_text_)
            p_disabled_text_->hide();

        alive_checker m_checker(*this);
        fire_script("OnEnable");
        if (!m_checker.is_alive())
            return;
    }
}

bool button::is_enabled() const {
    return (m_state_ != state::disabled);
}

void button::push() {
    if (is_enabled()) {
        if (p_pushed_texture_) {
            p_pushed_texture_->show();
            if (p_normal_texture_)
                p_normal_texture_->hide();
        }

        if (p_highlight_text_)
            p_highlight_text_->set_offset(m_pushed_text_offset_);
        if (p_normal_text_)
            p_normal_text_->set_offset(m_pushed_text_offset_);

        m_state_ = state::down;
    }
}

void button::release() {
    if (is_enabled()) {
        if (p_pushed_texture_) {
            p_pushed_texture_->hide();
            if (p_normal_texture_)
                p_normal_texture_->show();
        }

        if (p_highlight_text_)
            p_highlight_text_->set_offset(vector2f(0, 0));
        if (p_normal_text_)
            p_normal_text_->set_offset(vector2f(0, 0));

        m_state_ = state::up;
    }
}

void button::highlight() {
    if (!b_highlighted_) {
        if (p_highlight_texture_) {
            p_highlight_texture_->show();
        }

        if (p_highlight_text_) {
            if (p_current_font_string_)
                p_current_font_string_->hide();
            p_current_font_string_ = p_highlight_text_;
            p_current_font_string_->show();
        }

        b_highlighted_ = true;
    }
}

void button::unlight() {
    if (!b_lock_highlight_ && b_highlighted_) {
        if (p_highlight_texture_) {
            p_highlight_texture_->hide();
        }

        if (p_highlight_text_) {
            if (p_current_font_string_)
                p_current_font_string_->hide();

            switch (m_state_) {
            case state::up: p_current_font_string_ = p_normal_text_; break;
            case state::down: p_current_font_string_ = p_normal_text_; break;
            case state::disabled: p_current_font_string_ = p_disabled_text_; break;
            }

            if (p_current_font_string_)
                p_current_font_string_->show();
        }

        b_highlighted_ = false;
    }
}

button::state button::get_button_state() const {
    return m_state_;
}

void button::lock_highlight() {
    highlight();
    b_lock_highlight_ = true;
}

void button::unlock_highlight() {
    if (!b_mouse_in_frame_)
        unlight();

    b_lock_highlight_ = false;
}

void button::set_pushed_text_offset(const vector2f& m_offset) {
    m_pushed_text_offset_ = m_offset;
    notify_renderer_need_redraw();
}

const vector2f& button::get_pushed_text_offset() const {
    return m_pushed_text_offset_;
}

} // namespace lxgui::gui
