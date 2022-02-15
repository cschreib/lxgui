#include "lxgui/gui_checkbutton.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_texture.hpp"

namespace lxgui::gui {

check_button::check_button(utils::control_block& m_block, manager& m_manager) :
    button(m_block, m_manager) {
    type_.push_back(class_name);
}

std::string check_button::serialize(const std::string& tab) const {
    return base::serialize(tab);
}

void check_button::copy_from(const region& m_obj) {
    base::copy_from(m_obj);

    const check_button* p_button = down_cast<check_button>(&m_obj);
    if (!p_button)
        return;

    if (const texture* p_checked_texture = p_button->get_checked_texture().get()) {
        region_core_attributes m_attr;
        m_attr.name        = p_checked_texture->get_name();
        m_attr.inheritance = {p_button->get_checked_texture()};

        auto p_texture = this->create_layered_region<texture>(
            p_checked_texture->get_draw_layer(), std::move(m_attr));

        if (p_texture) {
            p_texture->set_special();
            p_texture->notify_loaded();
            this->set_checked_texture(p_texture);
        }
    }

    if (const texture* p_disabled_texture = p_button->get_disabled_checked_texture().get()) {
        region_core_attributes m_attr;
        m_attr.name        = p_disabled_texture->get_name();
        m_attr.inheritance = {p_button->get_disabled_checked_texture()};

        auto p_texture = this->create_layered_region<texture>(
            p_disabled_texture->get_draw_layer(), std::move(m_attr));

        if (p_texture) {
            p_texture->set_special();
            p_texture->notify_loaded();
            this->set_disabled_checked_texture(p_texture);
        }
    }
}

void check_button::check() {
    if (!b_checked_) {
        if (m_state_ == state::disabled) {
            if (p_disabled_checked_texture_)
                p_disabled_checked_texture_->show();
            else if (p_checked_texture_)
                p_checked_texture_->show();
        } else {
            if (p_checked_texture_)
                p_checked_texture_->show();
        }

        b_checked_ = true;
    }
}

void check_button::uncheck() {
    if (b_checked_) {
        if (p_disabled_checked_texture_)
            p_disabled_checked_texture_->hide();

        if (p_checked_texture_)
            p_checked_texture_->hide();

        b_checked_ = false;
    }
}

void check_button::disable() {
    base::disable();

    if (is_enabled() && is_checked() && p_disabled_checked_texture_) {
        if (p_checked_texture_)
            p_checked_texture_->hide();

        p_disabled_checked_texture_->show();
    }
}

void check_button::enable() {
    base::enable();

    if (!is_enabled() && is_checked() && p_disabled_checked_texture_) {
        if (p_checked_texture_)
            p_checked_texture_->show();

        p_disabled_checked_texture_->hide();
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
    return b_checked_;
}

void check_button::set_checked_texture(utils::observer_ptr<texture> p_texture) {
    p_checked_texture_ = std::move(p_texture);
    if (!p_checked_texture_)
        return;

    p_checked_texture_->set_shown(
        is_checked() && (m_state_ != state::disabled || !p_disabled_checked_texture_));
}

void check_button::set_disabled_checked_texture(utils::observer_ptr<texture> p_texture) {
    p_disabled_checked_texture_ = std::move(p_texture);
    if (!p_disabled_checked_texture_)
        return;

    if (p_checked_texture_) {
        p_checked_texture_->set_shown(
            is_checked() && (m_state_ != state::disabled || !p_disabled_checked_texture_));
    }

    p_disabled_checked_texture_->set_shown(is_checked() && m_state_ == state::disabled);
}

void check_button::create_glue() {
    create_glue_(this);
}

} // namespace lxgui::gui
