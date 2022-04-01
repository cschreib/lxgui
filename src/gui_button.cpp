#include "lxgui/gui_button.hpp"

#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_font_string.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"

namespace lxgui::gui {

button::button(utils::control_block& block, manager& mgr, const frame_core_attributes& attr) :
    frame(block, mgr, attr) {

    initialize_(*this, attr);

    enable_mouse(true);
}

std::string button::serialize(const std::string& tab) const {
    return base::serialize(tab);
}

bool button::can_use_script(const std::string& script_name) const {
    return base::can_use_script(script_name) || script_name == "OnClick" ||
           script_name == "OnEnable" || script_name == "OnDisable";
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

    const button* button_obj = down_cast<button>(&obj);
    if (!button_obj)
        return;

    this->set_text(button_obj->get_text());

    if (const texture* other_texture = button_obj->get_normal_texture().get()) {
        region_core_attributes attr;
        attr.name        = other_texture->get_raw_name();
        attr.inheritance = {button_obj->get_normal_texture()};

        auto tex =
            this->create_layered_region<texture>(other_texture->get_draw_layer(), std::move(attr));

        if (tex) {
            tex->set_manually_inherited(true);
            tex->notify_loaded();
            this->set_normal_texture(tex);
        }
    }

    if (const texture* other_texture = button_obj->get_pushed_texture().get()) {
        region_core_attributes attr;
        attr.name        = other_texture->get_raw_name();
        attr.inheritance = {button_obj->get_pushed_texture()};

        auto tex =
            this->create_layered_region<texture>(other_texture->get_draw_layer(), std::move(attr));

        if (tex) {
            tex->set_manually_inherited(true);
            tex->notify_loaded();
            this->set_pushed_texture(tex);
        }
    }

    if (const texture* other_texture = button_obj->get_highlight_texture().get()) {
        region_core_attributes attr;
        attr.name        = other_texture->get_raw_name();
        attr.inheritance = {button_obj->get_highlight_texture()};

        auto tex =
            this->create_layered_region<texture>(other_texture->get_draw_layer(), std::move(attr));

        if (tex) {
            tex->set_manually_inherited(true);
            tex->notify_loaded();
            this->set_highlight_texture(tex);
        }
    }

    if (const texture* other_texture = button_obj->get_disabled_texture().get()) {
        region_core_attributes attr;
        attr.name        = other_texture->get_raw_name();
        attr.inheritance = {button_obj->get_disabled_texture()};

        auto tex =
            this->create_layered_region<texture>(other_texture->get_draw_layer(), std::move(attr));

        if (tex) {
            tex->set_manually_inherited(true);
            tex->notify_loaded();
            this->set_disabled_texture(tex);
        }
    }

    if (const font_string* other_text = button_obj->get_normal_text().get()) {
        region_core_attributes attr;
        attr.name        = other_text->get_raw_name();
        attr.inheritance = {button_obj->get_normal_text()};

        auto fstr =
            this->create_layered_region<font_string>(other_text->get_draw_layer(), std::move(attr));

        if (fstr) {
            fstr->set_manually_inherited(true);
            fstr->notify_loaded();
            this->set_normal_text(fstr);
        }
    }

    if (const font_string* other_text = button_obj->get_highlight_text().get()) {
        region_core_attributes attr;
        attr.name        = other_text->get_raw_name();
        attr.inheritance = {button_obj->get_highlight_text()};

        auto fstr =
            this->create_layered_region<font_string>(other_text->get_draw_layer(), std::move(attr));

        if (fstr) {
            fstr->set_manually_inherited(true);
            fstr->notify_loaded();
            this->set_highlight_text(fstr);
        }
    }

    if (const font_string* other_text = button_obj->get_disabled_text().get()) {
        region_core_attributes attr;
        attr.name        = other_text->get_raw_name();
        attr.inheritance = {button_obj->get_disabled_text()};

        auto fstr =
            this->create_layered_region<font_string>(other_text->get_draw_layer(), std::move(attr));

        if (fstr) {
            fstr->set_manually_inherited(true);
            fstr->notify_loaded();
            this->set_disabled_text(fstr);
        }
    }

    this->set_pushed_text_offset(button_obj->get_pushed_text_offset());

    if (!button_obj->is_enabled())
        this->disable();
}

void button::set_text(const utils::ustring& content) {
    content_ = content;

    if (normal_text_)
        normal_text_->set_text(content);

    if (highlight_text_)
        highlight_text_->set_text(content);

    if (disabled_text_)
        disabled_text_->set_text(content);
}

const utils::ustring& button::get_text() const {
    return content_;
}

void button::set_normal_texture(utils::observer_ptr<texture> tex) {
    normal_texture_ = std::move(tex);
    if (!normal_texture_)
        return;

    normal_texture_->set_shown(state_ == state::up);
}

void button::set_pushed_texture(utils::observer_ptr<texture> tex) {
    pushed_texture_ = std::move(tex);
    if (!pushed_texture_)
        return;

    pushed_texture_->set_shown(state_ == state::down);
}

void button::set_disabled_texture(utils::observer_ptr<texture> tex) {
    disabled_texture_ = std::move(tex);
    if (!disabled_texture_)
        return;

    disabled_texture_->set_shown(state_ == state::disabled);
}

void button::set_highlight_texture(utils::observer_ptr<texture> tex) {
    highlight_texture_ = std::move(tex);
    if (!highlight_texture_)
        return;

    highlight_texture_->set_shown(is_highlighted_);
}

void button::set_normal_text(utils::observer_ptr<font_string> fstr) {
    if (normal_text_ == current_font_string_)
        current_font_string_ = fstr;

    normal_text_ = std::move(fstr);
    if (!normal_text_)
        return;

    normal_text_->set_shown(state_ == state::up);
    normal_text_->set_text(content_);
}

void button::set_highlight_text(utils::observer_ptr<font_string> fstr) {
    if (highlight_text_ == current_font_string_)
        current_font_string_ = fstr;

    highlight_text_ = std::move(fstr);
    if (!highlight_text_)
        return;

    highlight_text_->set_shown(is_highlighted_);
    highlight_text_->set_text(content_);
}

void button::set_disabled_text(utils::observer_ptr<font_string> fstr) {
    if (disabled_text_ == current_font_string_)
        current_font_string_ = fstr;

    disabled_text_ = std::move(fstr);
    if (!disabled_text_)
        return;

    disabled_text_->set_shown(state_ == state::disabled);
    disabled_text_->set_text(content_);
}

void button::disable() {
    if (!is_enabled())
        return;

    state_ = state::disabled;
    if (disabled_texture_) {
        if (normal_texture_)
            normal_texture_->hide();
        if (pushed_texture_)
            pushed_texture_->hide();

        disabled_texture_->show();
    } else {
        if (normal_texture_)
            normal_texture_->show();
        if (pushed_texture_)
            pushed_texture_->hide();
    }

    if (disabled_text_) {
        if (normal_text_)
            normal_text_->hide();

        disabled_text_->show();
        current_font_string_ = disabled_text_;
    } else {
        if (normal_text_)
            normal_text_->show();

        current_font_string_ = normal_text_;
    }

    unlight();

    fire_script("OnDisable");
}

void button::enable() {
    if (is_enabled())
        return;

    state_ = state::up;
    if (disabled_texture_) {
        if (normal_texture_)
            normal_texture_->show();
        if (pushed_texture_)
            pushed_texture_->hide();

        disabled_texture_->hide();
    } else {
        if (normal_texture_)
            normal_texture_->show();
        if (pushed_texture_)
            pushed_texture_->hide();
    }

    if (normal_text_)
        normal_text_->show();

    current_font_string_ = normal_text_;

    if (disabled_text_)
        disabled_text_->hide();

    fire_script("OnEnable");
}

bool button::is_enabled() const {
    return state_ != state::disabled;
}

void button::push() {
    if (!is_enabled())
        return;

    if (pushed_texture_) {
        pushed_texture_->show();
        if (normal_texture_)
            normal_texture_->hide();
    }

    if (highlight_text_)
        highlight_text_->set_offset(pushed_text_offset_);
    if (normal_text_)
        normal_text_->set_offset(pushed_text_offset_);

    state_ = state::down;
}

void button::release() {
    if (!is_enabled())
        return;

    if (pushed_texture_) {
        pushed_texture_->hide();
        if (normal_texture_)
            normal_texture_->show();
    }

    if (highlight_text_)
        highlight_text_->set_offset(vector2f(0, 0));
    if (normal_text_)
        normal_text_->set_offset(vector2f(0, 0));

    state_ = state::up;
}

void button::highlight() {
    if (is_highlighted_)
        return;

    if (highlight_texture_) {
        highlight_texture_->show();
    }

    if (highlight_text_) {
        if (current_font_string_)
            current_font_string_->hide();
        current_font_string_ = highlight_text_;
        current_font_string_->show();
    }

    is_highlighted_ = true;
}

void button::unlight() {
    if (is_highlight_locked_ || !is_highlighted_)
        return;

    if (highlight_texture_) {
        highlight_texture_->hide();
    }

    if (highlight_text_) {
        if (current_font_string_)
            current_font_string_->hide();

        switch (state_) {
        case state::up: current_font_string_ = normal_text_; break;
        case state::down: current_font_string_ = normal_text_; break;
        case state::disabled: current_font_string_ = disabled_text_; break;
        }

        if (current_font_string_)
            current_font_string_->show();
    }

    is_highlighted_ = false;
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

const std::vector<std::string>& button::get_type_list_() const {
    return get_type_list_impl_<button>();
}

} // namespace lxgui::gui
