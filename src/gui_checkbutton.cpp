#include "lxgui/gui_checkbutton.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"

namespace lxgui {
namespace gui
{
check_button::check_button(utils::control_block& mBlock, manager& mManager) : button(mBlock, mManager)
{
    lType_.push_back(CLASS_NAME);
}

std::string check_button::serialize(const std::string& sTab) const
{
    return base::serialize(sTab);
}

void check_button::copy_from(const region& mObj)
{
    base::copy_from(mObj);

    const check_button* pButton = down_cast<check_button>(&mObj);
    if (!pButton)
        return;

    if (const texture* pCheckedTexture = pButton->get_checked_texture().get())
    {
        region_core_attributes mAttr;
        mAttr.sName = pCheckedTexture->get_name();
        mAttr.lInheritance = {pButton->get_checked_texture()};

        auto pTexture = this->create_region<texture>(
            pCheckedTexture->get_draw_layer(), std::move(mAttr));

        if (pTexture)
        {
            pTexture->set_special();
            pTexture->notify_loaded();
            this->set_checked_texture(pTexture);
        }
    }

    if (const texture* pDisabledTexture = pButton->get_disabled_checked_texture().get())
    {
        region_core_attributes mAttr;
        mAttr.sName = pDisabledTexture->get_name();
        mAttr.lInheritance = {pButton->get_disabled_checked_texture()};

        auto pTexture = this->create_region<texture>(
            pDisabledTexture->get_draw_layer(), std::move(mAttr));

        if (pTexture)
        {
            pTexture->set_special();
            pTexture->notify_loaded();
            this->set_disabled_checked_texture(pTexture);
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
    base::disable();

    if (is_enabled() && is_checked() && pDisabledCheckedTexture_)
    {
        if (pCheckedTexture_)
            pCheckedTexture_->hide();

        pDisabledCheckedTexture_->show();
    }
}

void check_button::enable()
{
    base::enable();

    if (!is_enabled() && is_checked() && pDisabledCheckedTexture_)
    {
        if (pCheckedTexture_)
            pCheckedTexture_->show();

        pDisabledCheckedTexture_->hide();
    }
}

void check_button::release()
{
    base::release();

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
    if (!pCheckedTexture_)
        return;

    pCheckedTexture_->set_shown(is_checked() &&
        (mState_ != state::DISABLED || !pDisabledCheckedTexture_));
}

void check_button::set_disabled_checked_texture(utils::observer_ptr<texture> pTexture)
{
    pDisabledCheckedTexture_ = std::move(pTexture);
    if (!pDisabledCheckedTexture_)
        return;

    if (pCheckedTexture_)
    {
        pCheckedTexture_->set_shown(is_checked() &&
            (mState_ != state::DISABLED || !pDisabledCheckedTexture_));
    }

    pDisabledCheckedTexture_->set_shown(is_checked() && mState_ == state::DISABLED);
}

void check_button::create_glue()
{
    create_glue_(this);
}

}
}
