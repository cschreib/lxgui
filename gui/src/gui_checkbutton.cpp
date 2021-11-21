#include "lxgui/gui_checkbutton.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"
#include <lxgui/luapp_state.hpp>

namespace lxgui {
namespace gui
{
check_button::check_button(manager& mManager) : button(mManager)
{
    lType_.push_back(CLASS_NAME);
}

std::string check_button::serialize(const std::string& sTab) const
{
    return button::serialize(sTab);
}

void check_button::copy_from(const uiobject& mObj)
{
    button::copy_from(mObj);

    const check_button* pButton = down_cast<check_button>(&mObj);
    if (!pButton)
        return;

    if (const texture* pCheckedTexture = pButton->get_checked_texture().get())
    {
        auto pTexture = this->create_checked_texture_();
        if (this->is_virtual())
            pTexture->set_virtual();
        pTexture->set_name(pCheckedTexture->get_name());
        if (!get_manager().add_uiobject(pTexture))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to add \""+pCheckedTexture->get_name()+"\" to \""+sName_+"\", "
                "but its name was already taken : \""+pTexture->get_name()+"\". Skipped." << std::endl;
        }
        else
        {
            if (!is_virtual())
                pTexture->create_glue();
            pTexture->set_draw_layer(pCheckedTexture->get_draw_layer());
            pTexture->copy_from(*pCheckedTexture);
            pTexture->notify_loaded();

            this->set_checked_texture(pTexture);
            this->add_region(std::move(pTexture));
        }
    }

    if (const texture* pDisabledTexture = pButton->get_disabled_checked_texture().get())
    {
        auto pTexture = this->create_disabled_checked_texture_();
        if (this->is_virtual())
            pTexture->set_virtual();
        pTexture->set_name(pDisabledTexture->get_name());
        if (!get_manager().add_uiobject(pTexture))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to add \""+pDisabledTexture->get_name()+"\" to \""+sName_+"\", "
                "but its name was already taken : \""+pTexture->get_name()+"\". Skipped." << std::endl;
        }
        else
        {
            if (!is_virtual())
                pTexture->create_glue();
            pTexture->set_draw_layer(pDisabledTexture->get_draw_layer());
            pTexture->copy_from(*pDisabledTexture);
            pTexture->notify_loaded();

            this->set_disabled_checked_texture(pTexture);
            this->add_region(std::move(pTexture));
        }
    }
}

void check_button::check()
{
    if (!bChecked_)
    {
        if (mState_ == state::DISABLED)
        {
            if (pDisabledCheckedTexture_)
                pDisabledCheckedTexture_->show();
            else if (pCheckedTexture_)
                pCheckedTexture_->show();
        }
        else
        {
            if (pCheckedTexture_)
                pCheckedTexture_->show();
        }

        bChecked_ = true;
    }
}

void check_button::uncheck()
{
    if (bChecked_)
    {
        if (pDisabledCheckedTexture_)
            pDisabledCheckedTexture_->hide();

        if (pCheckedTexture_)
            pCheckedTexture_->hide();

        bChecked_ = false;
    }
}

void check_button::disable()
{
    button::disable();

    if (is_enabled() && is_checked() && pDisabledCheckedTexture_)
    {
        if (pCheckedTexture_)
            pCheckedTexture_->hide();

        pDisabledCheckedTexture_->show();
    }
}

void check_button::enable()
{
    button::enable();

    if (!is_enabled() && is_checked() && pDisabledCheckedTexture_)
    {
        if (pCheckedTexture_)
            pCheckedTexture_->show();

        pDisabledCheckedTexture_->hide();
    }
}

void check_button::release()
{
    button::release();

    if (is_checked())
        uncheck();
    else
        check();
}

bool check_button::is_checked() const
{
    return bChecked_;
}

void check_button::set_checked_texture(utils::observer_ptr<texture> pTexture)
{
    pCheckedTexture_ = std::move(pTexture);
}

void check_button::set_disabled_checked_texture(utils::observer_ptr<texture> pTexture)
{
    pDisabledCheckedTexture_ = std::move(pTexture);
}

void check_button::create_glue()
{
    create_glue_(this);
}

utils::owner_ptr<texture> check_button::create_checked_texture_()
{
    auto pCheckedTexture = utils::make_owned<texture>(get_manager());
    pCheckedTexture->set_special();
    pCheckedTexture->set_parent(observer_from(this));
    pCheckedTexture->set_draw_layer(layer_type::ARTWORK);

    return pCheckedTexture;
}

utils::owner_ptr<texture> check_button::create_disabled_checked_texture_()
{
    auto pDisabledCheckedTexture = utils::make_owned<texture>(get_manager());
    pDisabledCheckedTexture->set_special();
    pDisabledCheckedTexture->set_parent(observer_from(this));
    pDisabledCheckedTexture->set_draw_layer(layer_type::ARTWORK);

    return pDisabledCheckedTexture;
}
}
}
