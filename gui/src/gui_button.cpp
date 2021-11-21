#include "lxgui/gui_button.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"
#include "lxgui/gui_alive_checker.hpp"
#include <lxgui/luapp_state.hpp>

namespace lxgui {
namespace gui
{
button::button(manager& mManager) : frame(mManager)
{
    lType_.push_back(CLASS_NAME);
}

std::string button::serialize(const std::string& sTab) const
{
    return frame::serialize(sTab);
}

void button::create_glue()
{
    create_glue_(this);
}

bool button::can_use_script(const std::string& sScriptName) const
{
    if (frame::can_use_script(sScriptName))
        return true;
    else if ((sScriptName == "OnClick") ||
        (sScriptName == "OnDoubleClick") ||
        (sScriptName == "OnEnable") ||
        (sScriptName == "OnDisable"))
        return true;
    else
        return false;
}

void button::on_script(const std::string& sScriptName, event* pEvent)
{
    if (sScriptName == "OnLoad")
        enable_mouse(true);

    alive_checker mChecker(*this);
    frame::on_script(sScriptName, pEvent);
    if (!mChecker.is_alive())
        return;

    if (is_enabled())
    {
        if (sScriptName == "OnEnter")
            highlight();

        if (sScriptName == "OnLeave")
        {
            unlight();

            if (mState_ == state::DOWN)
                release();
        }

        if (sScriptName == "OnMouseDown")
            push();

        if (sScriptName == "OnMouseUp")
        {
            release();
            on_script("OnClick");
            if (!mChecker.is_alive())
                return;
        }
    }
}

void button::on_event(const event& mEvent)
{
    alive_checker mChecker(*this);

    frame::on_event(mEvent);
    if (!mChecker.is_alive())
        return;

    if (!get_manager().is_input_enabled())
        return;

    if (mEvent.get_name() == "MOUSE_DOUBLE_CLICKED")
    {
        update_mouse_in_frame_();
        if (bMouseInFrame_)
        {
            on_script("OnDoubleClicked");
            if (!mChecker.is_alive())
                return;
        }
    }
}

void button::copy_from(const uiobject& mObj)
{
    frame::copy_from(mObj);

    const button* pButton = down_cast<button>(&mObj);
    if (!pButton)
        return;

    this->set_text(pButton->get_text());

    if (const texture* pOtherTexture = pButton->get_normal_texture().get())
    {
        auto pTexture = this->create_normal_texture_();
        if (this->is_virtual())
            pTexture->set_virtual();
        pTexture->set_name(pOtherTexture->get_name());
        if (!get_manager().add_uiobject(pTexture))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to add \""+pOtherTexture->get_name()+"\" to \""+sName_+"\", "
                "but its name was already taken : \""+pTexture->get_name()+"\". Skipped." << std::endl;
        }
        else
        {
            if (!is_virtual())
                pTexture->create_glue();
            pTexture->set_draw_layer(pOtherTexture->get_draw_layer());
            pTexture->copy_from(*pOtherTexture);
            pTexture->notify_loaded();
            this->set_normal_texture(pTexture);

            this->add_region(std::move(pTexture));
        }
    }

    if (const texture* pOtherTexture = pButton->get_pushed_texture().get())
    {
        auto pTexture = this->create_pushed_texture_();
        if (this->is_virtual())
            pTexture->set_virtual();
        pTexture->set_name(pOtherTexture->get_name());
        if (!get_manager().add_uiobject(pTexture))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to add \""+pOtherTexture->get_name()+"\" to \""+sName_+"\", "
                "but its name was already taken : \""+pTexture->get_name()+"\". Skipped." << std::endl;
        }
        else
        {
            if (!is_virtual())
                pTexture->create_glue();
            pTexture->set_draw_layer(pOtherTexture->get_draw_layer());
            pTexture->copy_from(*pOtherTexture);
            pTexture->notify_loaded();

            this->set_pushed_texture(pTexture);
            this->add_region(std::move(pTexture));
        }
    }

    if (const texture* pOtherTexture = pButton->get_highlight_texture().get())
    {
        auto pTexture = this->create_highlight_texture_();
        if (this->is_virtual())
            pTexture->set_virtual();
        pTexture->set_name(pOtherTexture->get_name());
        if (!get_manager().add_uiobject(pTexture))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to add \""+pOtherTexture->get_name()+"\" to \""+sName_+"\", "
                "but its name was already taken : \""+pTexture->get_name()+"\". Skipped." << std::endl;
        }
        else
        {
            if (!is_virtual())
                pTexture->create_glue();
            pTexture->set_draw_layer(pOtherTexture->get_draw_layer());
            pTexture->copy_from(*pOtherTexture);
            pTexture->notify_loaded();

            this->set_highlight_texture(pTexture);
            this->add_region(std::move(pTexture));
        }
    }

    if (const texture* pOtherTexture = pButton->get_disabled_texture().get())
    {
        auto pTexture = this->create_disabled_texture_();
        if (this->is_virtual())
            pTexture->set_virtual();
        pTexture->set_name(pOtherTexture->get_name());
        if (!get_manager().add_uiobject(pTexture))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to add \""+pOtherTexture->get_name()+"\" to \""+sName_+"\", "
                "but its name was already taken : \""+pTexture->get_name()+"\". Skipped." << std::endl;
        }
        else
        {
            if (!is_virtual())
                pTexture->create_glue();
            pTexture->set_draw_layer(pOtherTexture->get_draw_layer());
            pTexture->copy_from(*pOtherTexture);
            pTexture->notify_loaded();

            this->set_disabled_texture(pTexture);
            this->add_region(std::move(pTexture));
        }
    }

    if (const font_string* pOtherText = pButton->get_normal_text().get())
    {
        auto pText = this->create_normal_text_();
        if (this->is_virtual())
            pText->set_virtual();
        pText->set_name(pOtherText->get_name());
        if (!get_manager().add_uiobject(pText))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to add \""+pOtherText->get_name()+"\" to \""+sName_+"\", "
                "but its name was already taken : \""+pText->get_name()+"\". Skipped." << std::endl;
        }
        else
        {
            if (!is_virtual())
                pText->create_glue();
            pText->set_draw_layer(pOtherText->get_draw_layer());
            pText->copy_from(*pOtherText);
            pText->notify_loaded();

            this->set_normal_text(pText);
            this->add_region(std::move(pText));
        }
    }

    if (const font_string* pOtherText = pButton->get_highlight_text().get())
    {
        auto pText = this->create_highlight_text_();
        if (this->is_virtual())
            pText->set_virtual();
        pText->set_name(pOtherText->get_name());
        if (!get_manager().add_uiobject(pText))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to add \""+pOtherText->get_name()+"\" to \""+sName_+"\", "
                "but its name was already taken : \""+pText->get_name()+"\". Skipped." << std::endl;
        }
        else
        {
            if (!is_virtual())
                pText->create_glue();
            pText->set_draw_layer(pOtherText->get_draw_layer());
            pText->copy_from(*pOtherText);
            pText->notify_loaded();

            this->set_highlight_text(pText);
            this->add_region(std::move(pText));
        }
    }

    if (const font_string* pOtherText = pButton->get_disabled_text().get())
    {
        auto pText = this->create_disabled_text_();
        if (this->is_virtual())
            pText->set_virtual();
        pText->set_name(pOtherText->get_name());
        if (!get_manager().add_uiobject(pText))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to add \""+pOtherText->get_name()+"\" to \""+sName_+"\", "
                "but its name was already taken : \""+pText->get_name()+"\". Skipped." << std::endl;
        }
        else
        {
            if (!is_virtual())
                pText->create_glue();
            pText->set_draw_layer(pOtherText->get_draw_layer());
            pText->copy_from(*pOtherText);
            pText->notify_loaded();

            this->set_disabled_text(pText);
            this->add_region(std::move(pText));
        }
    }

    this->set_pushed_text_offset(pButton->get_pushed_text_offset());

    if (!pButton->is_enabled())
        this->disable();
}

void button::set_text(const utils::ustring& sText)
{
    sText_ = sText;

    if (pNormalText_)
        pNormalText_->set_text(sText);

    if (pHighlightText_)
        pHighlightText_->set_text(sText);

    if (pDisabledText_)
        pDisabledText_->set_text(sText);
}

const utils::ustring& button::get_text() const
{
    return sText_;
}

utils::owner_ptr<texture> button::create_normal_texture_()
{
    auto pNormalTexture = utils::make_owned<texture>(get_manager());
    pNormalTexture->set_special();
    pNormalTexture->set_parent(observer_from(this));
    pNormalTexture->set_draw_layer(layer_type::BORDER);

    return pNormalTexture;
}

utils::owner_ptr<texture> button::create_pushed_texture_()
{
    auto pPushedTexture = utils::make_owned<texture>(get_manager());
    pPushedTexture->set_special();
    pPushedTexture->set_parent(observer_from(this));
    pPushedTexture->set_draw_layer(layer_type::BORDER);

    return pPushedTexture;
}

utils::owner_ptr<texture> button::create_disabled_texture_()
{
    auto pDisabledTexture = utils::make_owned<texture>(get_manager());
    pDisabledTexture->set_special();
    pDisabledTexture->set_parent(observer_from(this));
    pDisabledTexture->set_draw_layer(layer_type::BORDER);

    return pDisabledTexture;
}

utils::owner_ptr<texture> button::create_highlight_texture_()
{
    auto pHighlightTexture = utils::make_owned<texture>(get_manager());
    pHighlightTexture->set_special();
    pHighlightTexture->set_parent(observer_from(this));
    pHighlightTexture->set_draw_layer(layer_type::HIGHLIGHT);

    return pHighlightTexture;
}

utils::owner_ptr<font_string> button::create_normal_text_()
{
    auto pNormalText = utils::make_owned<font_string>(get_manager());
    pNormalText->set_special();
    pNormalText->set_parent(observer_from(this));
    pNormalText->set_draw_layer(layer_type::ARTWORK);

    return pNormalText;
}

utils::owner_ptr<font_string> button::create_highlight_text_()
{
    auto pHighlightText = utils::make_owned<font_string>(get_manager());
    pHighlightText->set_special();
    pHighlightText->set_parent(observer_from(this));
    pHighlightText->set_draw_layer(layer_type::ARTWORK);

    return pHighlightText;
}

utils::owner_ptr<font_string> button::create_disabled_text_()
{
    auto pDisabledText = utils::make_owned<font_string>(get_manager());
    pDisabledText->set_special();
    pDisabledText->set_parent(observer_from(this));
    pDisabledText->set_draw_layer(layer_type::BORDER);

    return pDisabledText;
}

void button::set_normal_texture(utils::observer_ptr<texture> pTexture)
{
    pNormalTexture_ = pTexture;
}

void button::set_pushed_texture(utils::observer_ptr<texture> pTexture)
{
    pPushedTexture_ = pTexture;
}

void button::set_disabled_texture(utils::observer_ptr<texture> pTexture)
{
    pDisabledTexture_ = pTexture;
}

void button::set_highlight_texture(utils::observer_ptr<texture> pTexture)
{
    pHighlightTexture_ = pTexture;
}

void button::set_normal_text(utils::observer_ptr<font_string> pFont)
{
    if (pNormalText_ == pCurrentFontString_)
        pCurrentFontString_ = pFont;

    pNormalText_ = pFont;
    pNormalText_->set_text(sText_);
}

void button::set_highlight_text(utils::observer_ptr<font_string> pFont)
{
    if (pHighlightText_ == pCurrentFontString_)
        pCurrentFontString_ = pFont;

    pHighlightText_ = pFont;
    pNormalText_->set_text(sText_);
}

void button::set_disabled_text(utils::observer_ptr<font_string> pFont)
{
    if (pDisabledText_ == pCurrentFontString_)
        pCurrentFontString_ = pFont;

    pDisabledText_ = pFont;
    pNormalText_->set_text(sText_);
}

void button::disable()
{
    if (is_enabled())
    {
        mState_ = state::DISABLED;
        if (pDisabledTexture_)
        {
            if (pNormalTexture_)
                pNormalTexture_->hide();
            if (pPushedTexture_)
                pPushedTexture_->hide();

            pDisabledTexture_->show();
        }
        else
        {
            if (pNormalTexture_)
                pNormalTexture_->show();
            if (pPushedTexture_)
                pPushedTexture_->hide();
        }

        if (pDisabledText_)
        {
            if (pNormalText_)
                pNormalText_->hide();

            pDisabledText_->show();
            pCurrentFontString_ = pDisabledText_;
        }
        else
        {
            if (pNormalText_)
                pNormalText_->show();

            pCurrentFontString_ = pNormalText_;
        }

        unlight();

        alive_checker mChecker(*this);
        on_script("OnDisable");
        if (!mChecker.is_alive())
            return;
    }
}

void button::enable()
{
    if (!is_enabled())
    {
        mState_ = state::UP;
        if (pDisabledTexture_)
        {
            if (pNormalTexture_)
                pNormalTexture_->show();
            if (pPushedTexture_)
                pPushedTexture_->hide();

            pDisabledTexture_->hide();
        }
        else
        {
            if (pNormalTexture_)
                pNormalTexture_->show();
            if (pPushedTexture_)
                pPushedTexture_->hide();
        }

        if (pNormalText_)
            pNormalText_->show();

        pCurrentFontString_ = pNormalText_;

        if (pDisabledText_)
            pDisabledText_->hide();

        alive_checker mChecker(*this);
        on_script("OnEnable");
        if (!mChecker.is_alive())
            return;
    }
}

bool button::is_enabled() const
{
    return (mState_ != state::DISABLED);
}

void button::push()
{
    if (is_enabled())
    {
        if (pPushedTexture_)
        {
            pPushedTexture_->show();
            if (pNormalTexture_)
                pNormalTexture_->hide();
        }

        if (pHighlightText_)
            pHighlightText_->set_offsets(mPushedTextOffset_);
        if (pNormalText_)
            pNormalText_->set_offsets(mPushedTextOffset_);

        mState_ = state::DOWN;
    }
}

void button::release()
{
    if (is_enabled())
    {
        if (pPushedTexture_)
        {
            pPushedTexture_->hide();
            if (pNormalTexture_)
                pNormalTexture_->show();
        }

        if (pHighlightText_)
            pHighlightText_->set_offsets(0, 0);
        if (pNormalText_)
            pNormalText_->set_offsets(0, 0);

        mState_ = state::UP;
    }
}

void button::highlight()
{
    if (!bHighlighted_)
    {
        if (pHighlightTexture_)
        {
            pHighlightTexture_->show();
        }

        if (pHighlightText_)
        {
            if (pCurrentFontString_)
                pCurrentFontString_->hide();
            pCurrentFontString_ = pHighlightText_;
            pCurrentFontString_->show();
        }

        bHighlighted_ = true;
    }
}

void button::unlight()
{
    if (!bLockHighlight_ && bHighlighted_)
    {
        if (pHighlightTexture_)
        {
            pHighlightTexture_->hide();
        }

        if (pHighlightText_)
        {
            if (pCurrentFontString_)
                pCurrentFontString_->hide();

            switch (mState_)
            {
                case state::UP       : pCurrentFontString_ = pNormalText_; break;
                case state::DOWN     : pCurrentFontString_ = pNormalText_; break;
                case state::DISABLED : pCurrentFontString_ = pDisabledText_; break;
            }

            if (pCurrentFontString_)
                pCurrentFontString_->show();
        }


        bHighlighted_ = false;
    }
}

button::state button::get_button_state() const
{
    return mState_;
}

void button::lock_highlight()
{
    highlight();
    bLockHighlight_ = true;
}

void button::unlock_highlight()
{
    update_mouse_in_frame_();
    if (!bMouseInFrame_)
        unlight();

    bLockHighlight_ = false;
}

void button::set_pushed_text_offset(const vector2f& mOffset)
{
    mPushedTextOffset_ = mOffset;
    notify_renderer_need_redraw();
}

const vector2f& button::get_pushed_text_offset() const
{
    return mPushedTextOffset_;
}
}
}
