#include "lxgui/gui_check_button.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"

namespace lxgui::gui {

check_button::check_button(utils::control_block& block, manager& mgr) : button(block, mgr) {
    type_.push_back(class_name);
}

std::string check_button::serialize(const std::string& tab) const {
    return base::serialize(tab);
}

void check_button::copy_from(const region& obj) {
    base::copy_from(obj);

    const check_button* button_obj = down_cast<check_button>(&obj);
    if (!button_obj)
        return;

    if (const texture* checked_texture = button_obj->get_checked_texture().get()) {
        region_core_attributes attr;
        attr.name        = checked_texture->get_name();
        attr.inheritance = {button_obj->get_checked_texture()};

        auto tex = this->create_layered_region<texture>(
            checked_texture->get_draw_layer(), std::move(attr));

        if (tex) {
            tex->set_special();
            tex->notify_loaded();
            this->set_checked_texture(tex);
        }
    }

    if (const texture* disabled_texture = button_obj->get_disabled_checked_texture().get()) {
        region_core_attributes attr;
        attr.name        = disabled_texture->get_name();
        attr.inheritance = {button_obj->get_disabled_checked_texture()};

        auto tex = this->create_layered_region<texture>(
            disabled_texture->get_draw_layer(), std::move(attr));

        if (tex) {
            tex->set_special();
            tex->notify_loaded();
            this->set_disabled_checked_texture(tex);
        }
    }
}

void check_button::check() {
    if (!is_checked_) {
        if (state_ == state::disabled) {
            if (disabled_checked_texture_)
                disabled_checked_texture_->show();
            else if (checked_texture_)
                checked_texture_->show();
        } else {
            if (checked_texture_)
                checked_texture_->show();
        }

        is_checked_ = true;
    }
}

void check_button::uncheck() {
    if (is_checked_) {
        if (disabled_checked_texture_)
            disabled_checked_texture_->hide();

        if (checked_texture_)
            checked_texture_->hide();

        is_checked_ = false;
    }
}

void check_button::disable() {
    base::disable();

    if (is_enabled() && is_checked() && disabled_checked_texture_) {
        if (checked_texture_)
            checked_texture_->hide();

        disabled_checked_texture_->show();
    }
}

void check_button::enable() {
    base::enable();

    if (!is_enabled() && is_checked() && disabled_checked_texture_) {
        if (checked_texture_)
            checked_texture_->show();

        disabled_checked_texture_->hide();
    }
}

void check_button::release() {
    base::release();

    if (is_checked())
        uncheck();
    else
        check();
}

bool check_button::is_checked() const {
    return is_checked_;
}

void check_button::set_checked_texture(utils::observer_ptr<texture> tex) {
    checked_texture_ = std::move(tex);
    if (!checked_texture_)
        return;

    checked_texture_->set_shown(
        is_checked() && (state_ != state::disabled || !disabled_checked_texture_));
}

void check_button::set_disabled_checked_texture(utils::observer_ptr<texture> tex) {
    disabled_checked_texture_ = std::move(tex);
    if (!disabled_checked_texture_)
        return;

    if (checked_texture_) {
        checked_texture_->set_shown(
            is_checked() && (state_ != state::disabled || !disabled_checked_texture_));
    }

    disabled_checked_texture_->set_shown(is_checked() && state_ == state::disabled);
}

void check_button::create_glue() {
    create_glue_(this);
}

} // namespace lxgui::gui
