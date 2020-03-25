#include "lxgui/gui_checkbutton.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"

namespace gui
{
check_button::check_button(manager* pManager) : button(pManager),
    bChecked_(false), pCheckedTexture_(nullptr), pDisabledCheckedTexture_(nullptr)
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

    check_button* pButton = dynamic_cast<check_button*>(pObj);

    if (pButton)
    {
        if (pButton->get_checked_texture())
        {
            this->create_checked_texture_();
            if (this->is_virtual())
                pCheckedTexture_->set_virtual();
            pCheckedTexture_->set_name(pButton->get_checked_texture()->get_name());
            if (!pManager_->add_uiobject(pCheckedTexture_))
            {
                gui::out << gui::warning << "gui::" << lType_.back() << " : "
                    "Trying to add \""+pButton->get_checked_texture()->get_name()+"\" to \""+sName_+"\",\n"
                    "but its name was already taken : \""+pCheckedTexture_->get_name()+"\". Skipped." << std::endl;
                delete pCheckedTexture_; pCheckedTexture_ = nullptr;
            }
            else
            {
                if (!is_virtual())
                    pCheckedTexture_->create_glue();
                pCheckedTexture_->set_draw_layer(pButton->get_checked_texture()->get_draw_layer());
                this->add_region(pCheckedTexture_);
                pCheckedTexture_->copy_from(pButton->get_checked_texture());
                pCheckedTexture_->notify_loaded();
            }
        }
        if (pButton->get_disabled_checked_texture())
        {
            this->create_disabled_checked_texture_();
            if (this->is_virtual())
                pDisabledCheckedTexture_->set_virtual();
            pDisabledCheckedTexture_->set_name(pButton->get_disabled_checked_texture()->get_name());
            if (!pManager_->add_uiobject(pDisabledCheckedTexture_))
            {
                gui::out << gui::warning << "gui::" << lType_.back() << " : "
                    "Trying to add \""+pButton->get_disabled_checked_texture()->get_name()+"\" to \""+sName_+"\",\n"
                    "but its name was already taken : \""+pDisabledCheckedTexture_->get_name()+"\". Skipped." << std::endl;
                delete pDisabledTexture_; pDisabledTexture_ = nullptr;
            }
            else
            {
                if (!is_virtual())
                    pDisabledCheckedTexture_->create_glue();
                pDisabledCheckedTexture_->set_draw_layer(pButton->get_disabled_checked_texture()->get_draw_layer());
                this->add_region(pDisabledCheckedTexture_);
                pDisabledCheckedTexture_->copy_from(pButton->get_disabled_checked_texture());
                pDisabledCheckedTexture_->notify_loaded();
            }
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

texture* check_button::create_checked_texture_()
{
    if (!pCheckedTexture_)
    {
        pCheckedTexture_ = new texture(pManager_);
        pCheckedTexture_->set_special();
        pCheckedTexture_->set_parent(this);
        pCheckedTexture_->set_draw_layer("ARTWORK");
    }

    return pCheckedTexture_;
}

texture* check_button::create_disabled_checked_texture_()
{
    if (!pDisabledCheckedTexture_)
    {
        pDisabledCheckedTexture_ = new texture(pManager_);
        pDisabledCheckedTexture_->set_special();
        pDisabledCheckedTexture_->set_parent(this);
        pDisabledCheckedTexture_->set_draw_layer("ARTWORK");
    }

    return pDisabledCheckedTexture_;
}
}
