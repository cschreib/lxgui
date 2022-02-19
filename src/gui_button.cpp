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

button::button(utils::control_block& block, manager& mgr) : frame(block, mgr) {
    type_.push_back(class_name);
    enable_mouse(true);
}

std::string button::serialize(const std::string& tab) const {
    return base::serialize(tab);
}

void button::create_glue() {
    create_glue_(this);
}

bool button::can_use_script(const std::string& script_name) const {
    if (base::can_use_script(script_name))
        return true;
    else if (
        (script_name == "OnClick") || (script_name == "OnEnable") || (script_name == "OnDisable"))
        return true;
    else
        return false;
}

void button::fire_script(const std::string& script_name, const event_data& data) {
    alive_checker checker(*this);
    base::fire_script(script_name, data);
    if (!checker.is_alive())
        return;

    if (is_enabled()) {
        if (script_name == "OnEnter")
            highlight();

        if (script_name == "OnLeave") {
            unlight();

            if (state_ == state::down)
                release();
        }

        if (script_name == "OnMouseDown") {
            push();
            fire_script("OnClick");
            if (!checker.is_alive())
                return;
        }

        if (script_name == "OnMouseUp")
            release();
    }
}

void button::copy_from(const region& obj) {
    base::copy_from(obj);

    const button* p_button = down_cast<button>(&obj);
    if (!p_button)
        return;

    this->set_text(p_button->get_text());

    if (const texture* p_other_texture = p_button->get_normal_texture().get()) {
        region_core_attributes attr;
        attr.name        = p_other_texture->get_name();
        attr.inheritance = {p_button->get_normal_texture()};

        auto p_texture = this->create_layered_region<texture>(
            p_other_texture->get_draw_layer(), std::move(attr));

        if (p_texture) {
            p_texture->set_special();
            p_texture->notify_loaded();
            this->set_normal_texture(p_texture);
        }
    }

    if (const texture* p_other_texture = p_button->get_pushed_texture().get()) {
        region_core_attributes attr;
        attr.name        = p_other_texture->get_name();
        attr.inheritance = {p_button->get_pushed_texture()};

        auto p_texture = this->create_layered_region<texture>(
            p_other_texture->get_draw_layer(), std::move(attr));

        if (p_texture) {
            p_texture->set_special();
            p_texture->notify_loaded();
            this->set_pushed_texture(p_texture);
        }
    }

    if (const texture* p_other_texture = p_button->get_highlight_texture().get()) {
        region_core_attributes attr;
        attr.name        = p_other_texture->get_name();
        attr.inheritance = {p_button->get_highlight_texture()};

        auto p_texture = this->create_layered_region<texture>(
            p_other_texture->get_draw_layer(), std::move(attr));

        if (p_texture) {
            p_texture->set_special();
            p_texture->notify_loaded();
            this->set_highlight_texture(p_texture);
        }
    }

    if (const texture* p_other_texture = p_button->get_disabled_texture().get()) {
        region_core_attributes attr;
        attr.name        = p_other_texture->get_name();
        attr.inheritance = {p_button->get_disabled_texture()};

        auto p_texture = this->create_layered_region<texture>(
            p_other_texture->get_draw_layer(), std::move(attr));

        if (p_texture) {
            p_texture->set_special();
            p_texture->notify_loaded();
            this->set_disabled_texture(p_texture);
        }
    }

    if (const font_string* p_other_text = p_button->get_normal_text().get()) {
        region_core_attributes attr;
        attr.name        = p_other_text->get_name();
        attr.inheritance = {p_button->get_normal_text()};

        auto p_font = this->create_layered_region<font_string>(
            p_other_text->get_draw_layer(), std::move(attr));

        if (p_font) {
            p_font->set_special();
            p_font->notify_loaded();
            this->set_normal_text(p_font);
        }
    }

    if (const font_string* p_other_text = p_button->get_highlight_text().get()) {
        region_core_attributes attr;
        attr.name        = p_other_text->get_name();
        attr.inheritance = {p_button->get_highlight_text()};

        auto p_font = this->create_layered_region<font_string>(
            p_other_text->get_draw_layer(), std::move(attr));

        if (p_font) {
            p_font->set_special();
            p_font->notify_loaded();
            this->set_highlight_text(p_font);
        }
    }

    if (const font_string* p_other_text = p_button->get_disabled_text().get()) {
        region_core_attributes attr;
        attr.name        = p_other_text->get_name();
        attr.inheritance = {p_button->get_disabled_text()};

        auto p_font = this->create_layered_region<font_string>(
            p_other_text->get_draw_layer(), std::move(attr));

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

void button::set_text(const utils::ustring& content) {
    content_ = content;

    if (p_normal_text_)
        p_normal_text_->set_text(content);

    if (p_highlight_text_)
        p_highlight_text_->set_text(content);

    if (p_disabled_text_)
        p_disabled_text_->set_text(content);
}

const utils::ustring& button::get_text() const {
    return content_;
}

void button::set_normal_texture(utils::observer_ptr<texture> p_texture) {
    p_normal_texture_ = std::move(p_texture);
    if (!p_normal_texture_)
        return;

    p_normal_texture_->set_shown(state_ == state::up);
}

void button::set_pushed_texture(utils::observer_ptr<texture> p_texture) {
    p_pushed_texture_ = std::move(p_texture);
    if (!p_pushed_texture_)
        return;

    p_pushed_texture_->set_shown(state_ == state::down);
}

void button::set_disabled_texture(utils::observer_ptr<texture> p_texture) {
    p_disabled_texture_ = std::move(p_texture);
    if (!p_disabled_texture_)
        return;

    p_disabled_texture_->set_shown(state_ == state::disabled);
}

void button::set_highlight_texture(utils::observer_ptr<texture> p_texture) {
    p_highlight_texture_ = std::move(p_texture);
    if (!p_highlight_texture_)
        return;

    p_highlight_texture_->set_shown(is_highlighted_);
}

void button::set_normal_text(utils::observer_ptr<font_string> p_font) {
    if (p_normal_text_ == p_current_font_string_)
        p_current_font_string_ = p_font;

    p_normal_text_ = std::move(p_font);
    if (!p_normal_text_)
        return;

    p_normal_text_->set_shown(state_ == state::up);
    p_normal_text_->set_text(content_);
}

void button::set_highlight_text(utils::observer_ptr<font_string> p_font) {
    if (p_highlight_text_ == p_current_font_string_)
        p_current_font_string_ = p_font;

    p_highlight_text_ = std::move(p_font);
    if (!p_highlight_text_)
        return;

    p_highlight_text_->set_shown(is_highlighted_);
    p_highlight_text_->set_text(content_);
}

void button::set_disabled_text(utils::observer_ptr<font_string> p_font) {
    if (p_disabled_text_ == p_current_font_string_)
        p_current_font_string_ = p_font;

    p_disabled_text_ = std::move(p_font);
    if (!p_disabled_text_)
        return;

    p_disabled_text_->set_shown(state_ == state::disabled);
    p_disabled_text_->set_text(content_);
}

void button::disable() {
    if (is_enabled()) {
        state_ = state::disabled;
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

        alive_checker checker(*this);
        fire_script("OnDisable");
        if (!checker.is_alive())
            return;
    }
}

void button::enable() {
    if (!is_enabled()) {
        state_ = state::up;
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

        alive_checker checker(*this);
        fire_script("OnEnable");
        if (!checker.is_alive())
            return;
    }
}

bool button::is_enabled() const {
    return (state_ != state::disabled);
}

void button::push() {
    if (is_enabled()) {
        if (p_pushed_texture_) {
            p_pushed_texture_->show();
            if (p_normal_texture_)
                p_normal_texture_->hide();
        }

        if (p_highlight_text_)
            p_highlight_text_->set_offset(pushed_text_offset_);
        if (p_normal_text_)
            p_normal_text_->set_offset(pushed_text_offset_);

        state_ = state::down;
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

        state_ = state::up;
    }
}

void button::highlight() {
    if (!is_highlighted_) {
        if (p_highlight_texture_) {
            p_highlight_texture_->show();
        }

        if (p_highlight_text_) {
            if (p_current_font_string_)
                p_current_font_string_->hide();
            p_current_font_string_ = p_highlight_text_;
            p_current_font_string_->show();
        }

        is_highlighted_ = true;
    }
}

void button::unlight() {
    if (!is_highlight_locked_ && is_highlighted_) {
        if (p_highlight_texture_) {
            p_highlight_texture_->hide();
        }

        if (p_highlight_text_) {
            if (p_current_font_string_)
                p_current_font_string_->hide();

            switch (state_) {
            case state::up: p_current_font_string_ = p_normal_text_; break;
            case state::down: p_current_font_string_ = p_normal_text_; break;
            case state::disabled: p_current_font_string_ = p_disabled_text_; break;
            }

            if (p_current_font_string_)
                p_current_font_string_->show();
        }

        is_highlighted_ = false;
    }
}

button::state button::get_button_state() const {
    return state_;
}

void button::lock_highlight() {
    highlight();
    is_highlight_locked_ = true;
}

void button::unlock_highlight() {
    if (!is_mouse_in_frame_)
        unlight();

    is_highlight_locked_ = false;
}

void button::set_pushed_text_offset(const vector2f& offset) {
    pushed_text_offset_ = offset;
    notify_renderer_need_redraw();
}

const vector2f& button::get_pushed_text_offset() const {
    return pushed_text_offset_;
}

} // namespace lxgui::gui
