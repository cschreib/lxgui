#include "lxgui/gui_checkbutton.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"

namespace lxgui {
namespace gui
{
check_button::check_button(manager* pManager) : button(pManager)
{
    lType_.push_back(CLASS_NAME);
}

check_button::~check_button()
{
}

std::string check_button::serialize(const std::string& sTab) const
{
    return button::serialize(sTab);
}

void check_button::copy_from(uiobject* pObj)
{
    button::copy_from(pObj);

    check_button* pButton = pObj->down_cast<check_button>();
    if (!pButton)
        return;

    if (pButton->get_checked_texture())
    {
        std::unique_ptr<texture> pTexture = this->create_checked_texture_();
        if (this->is_virtual())
            pTexture->set_virtual();
        pTexture->set_name(pButton->get_checked_texture()->get_name());
        if (!pManager_->add_uiobject(pTexture.get()))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to add \""+pButton->get_checked_texture()->get_name()+"\" to \""+sName_+"\", "
                "but its name was already taken : \""+pTexture->get_name()+"\". Skipped." << std::endl;
        }
        else
        {
            if (!is_virtual())
                pTexture->create_glue();
            pTexture->set_draw_layer(pButton->get_checked_texture()->get_draw_layer());
            pTexture->copy_from(pButton->get_checked_texture());
            pTexture->notify_loaded();

            this->set_checked_texture(pTexture.get());
            this->add_region(std::move(pTexture));
        }
    }
    if (pButton->get_disabled_checked_texture())
    {
        std::unique_ptr<texture> pTexture = this->create_disabled_checked_texture_();
        if (this->is_virtual())
            pTexture->set_virtual();
        pTexture->set_name(pButton->get_disabled_checked_texture()->get_name());
        if (!pManager_->add_uiobject(pTexture.get()))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to add \""+pButton->get_disabled_checked_texture()->get_name()+"\" to \""+sName_+"\", "
                "but its name was already taken : \""+pTexture->get_name()+"\". Skipped." << std::endl;
        }
        else
        {
            if (!is_virtual())
                pTexture->create_glue();
            pTexture->set_draw_layer(pButton->get_disabled_checked_texture()->get_draw_layer());
            pTexture->copy_from(pButton->get_disabled_checked_texture());
            pTexture->notify_loaded();

            this->set_disabled_checked_texture(pTexture.get());
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

bool check_button::is_checked()
{
    return bChecked_;
}

texture* check_button::get_checked_texture()
{
    return pCheckedTexture_;
}

texture* check_button::get_disabled_checked_texture()
{
    return pDisabledCheckedTexture_;
}

void check_button::set_checked_texture(texture* pTexture)
{
    pCheckedTexture_ = pTexture;
}

void check_button::set_disabled_checked_texture(texture* pTexture)
{
    pDisabledCheckedTexture_ = pTexture;
}

void check_button::create_glue()
{
    create_glue_<lua_check_button>();
}

std::unique_ptr<texture> check_button::create_checked_texture_()
{
    std::unique_ptr<texture> pCheckedTexture(new texture(pManager_));
    pCheckedTexture->set_special();
    pCheckedTexture->set_parent(this);
    pCheckedTexture->set_draw_layer(layer_type::ARTWORK);

    return pCheckedTexture;
}

std::unique_ptr<texture> check_button::create_disabled_checked_texture_()
{
    std::unique_ptr<texture> pDisabledCheckedTexture(new texture(pManager_));
    pDisabledCheckedTexture->set_special();
    pDisabledCheckedTexture->set_parent(this);
    pDisabledCheckedTexture->set_draw_layer(layer_type::ARTWORK);

    return pDisabledCheckedTexture;
}
}
}
