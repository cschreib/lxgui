#include "lxgui/gui_button.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"
#include "lxgui/gui_alive_checker.hpp"

namespace lxgui {
namespace gui
{
button::button(utils::control_block& mBlock, manager& mManager) : frame(mBlock, mManager)
{
    lType_.push_back(CLASS_NAME);
}

std::string button::serialize(const std::string& sTab) const
{
    return base::serialize(sTab);
}

void button::create_glue()
{
    create_glue_(this);
}

bool button::can_use_script(const std::string& sScriptName) const
{
    if (base::can_use_script(sScriptName))
        return true;
    else if ((sScriptName == "OnClick") ||
        (sScriptName == "OnDoubleClick") ||
        (sScriptName == "OnEnable") ||
        (sScriptName == "OnDisable"))
        return true;
    else
        return false;
}

void button::on_script(const std::string& sScriptName, const event_data& mData)
{
    if (!is_loaded())
        return;

    if (sScriptName == "OnLoad")
        enable_mouse(true);

    alive_checker mChecker(*this);
    base::on_script(sScriptName, mData);
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

    base::on_event(mEvent);
    if (!mChecker.is_alive())
        return;

    if (!get_manager().is_input_enabled())
        return;

    if (mEvent.get_name() == "MOUSE_DOUBLE_CLICKED" && bMouseInFrame_)
    {
        on_script("OnDoubleClicked");
        if (!mChecker.is_alive())
            return;
    }
}

void button::copy_from(const uiobject& mObj)
{
    base::copy_from(mObj);

    const button* pButton = down_cast<button>(&mObj);
    if (!pButton)
        return;

    this->set_text(pButton->get_text());

    if (const texture* pOtherTexture = pButton->get_normal_texture().get())
    {
        uiobject_core_attributes mAttr;
        mAttr.sName = pOtherTexture->get_name();
        mAttr.lInheritance = {pButton->get_normal_texture()};

        auto pTexture = this->create_region<texture>(
            pOtherTexture->get_draw_layer(), std::move(mAttr));

        if (pTexture)
        {
            pTexture->set_special();
            pTexture->notify_loaded();
            this->set_normal_texture(pTexture);
        }
    }

    if (const texture* pOtherTexture = pButton->get_pushed_texture().get())
    {
        uiobject_core_attributes mAttr;
        mAttr.sName = pOtherTexture->get_name();
        mAttr.lInheritance = {pButton->get_pushed_texture()};

        auto pTexture = this->create_region<texture>(
            pOtherTexture->get_draw_layer(), std::move(mAttr));

        if (pTexture)
        {
            pTexture->set_special();
            pTexture->notify_loaded();
            this->set_pushed_texture(pTexture);
        }
    }

    if (const texture* pOtherTexture = pButton->get_highlight_texture().get())
    {
        uiobject_core_attributes mAttr;
        mAttr.sName = pOtherTexture->get_name();
        mAttr.lInheritance = {pButton->get_highlight_texture()};

        auto pTexture = this->create_region<texture>(
            pOtherTexture->get_draw_layer(), std::move(mAttr));

        if (pTexture)
        {
            pTexture->set_special();
            pTexture->notify_loaded();
            this->set_highlight_texture(pTexture);
        }
    }

    if (const texture* pOtherTexture = pButton->get_disabled_texture().get())
    {
        uiobject_core_attributes mAttr;
        mAttr.sName = pOtherTexture->get_name();
        mAttr.lInheritance = {pButton->get_disabled_texture()};

        auto pTexture = this->create_region<texture>(
            pOtherTexture->get_draw_layer(), std::move(mAttr));

        if (pTexture)
        {
            pTexture->set_special();
            pTexture->notify_loaded();
            this->set_disabled_texture(pTexture);
        }
    }

    if (const font_string* pOtherText = pButton->get_normal_text().get())
    {
        uiobject_core_attributes mAttr;
        mAttr.sName = pOtherText->get_name();
        mAttr.lInheritance = {pButton->get_normal_text()};

        auto pFont = this->create_region<font_string>(
            pOtherText->get_draw_layer(), std::move(mAttr));

        if (pFont)
        {
            pFont->set_special();
            pFont->notify_loaded();
            this->set_normal_text(pFont);
        }
    }

    if (const font_string* pOtherText = pButton->get_highlight_text().get())
    {
        uiobject_core_attributes mAttr;
        mAttr.sName = pOtherText->get_name();
        mAttr.lInheritance = {pButton->get_highlight_text()};

        auto pFont = this->create_region<font_string>(
            pOtherText->get_draw_layer(), std::move(mAttr));

        if (pFont)
        {
            pFont->set_special();
            pFont->notify_loaded();
            this->set_highlight_text(pFont);
        }
    }

    if (const font_string* pOtherText = pButton->get_disabled_text().get())
    {
        uiobject_core_attributes mAttr;
        mAttr.sName = pOtherText->get_name();
        mAttr.lInheritance = {pButton->get_disabled_text()};

        auto pFont = this->create_region<font_string>(
            pOtherText->get_draw_layer(), std::move(mAttr));

        if (pFont)
        {
            pFont->set_special();
            pFont->notify_loaded();
            this->set_disabled_text(pFont);
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

void button::set_normal_texture(utils::observer_ptr<texture> pTexture)
{
    pNormalTexture_ = std::move(pTexture);
    if (!pNormalTexture_)
        return;

    pNormalTexture_->set_shown(mState_ == state::UP);
}

void button::set_pushed_texture(utils::observer_ptr<texture> pTexture)
{
    pPushedTexture_ = std::move(pTexture);
    if (!pPushedTexture_)
        return;

    pPushedTexture_->set_shown(mState_ == state::DOWN);
}

void button::set_disabled_texture(utils::observer_ptr<texture> pTexture)
{
    pDisabledTexture_ = std::move(pTexture);
    if (!pDisabledTexture_)
        return;

    pDisabledTexture_->set_shown(mState_ == state::DISABLED);
}

void button::set_highlight_texture(utils::observer_ptr<texture> pTexture)
{
    pHighlightTexture_ = std::move(pTexture);
    if (!pHighlightTexture_)
        return;

    pHighlightTexture_->set_shown(bHighlighted_);
}

void button::set_normal_text(utils::observer_ptr<font_string> pFont)
{
    if (pNormalText_ == pCurrentFontString_)
        pCurrentFontString_ = pFont;

    pNormalText_ = std::move(pFont);
    if (!pNormalText_)
        return;

    pNormalText_->set_shown(mState_ == state::UP);
    pNormalText_->set_text(sText_);
}

void button::set_highlight_text(utils::observer_ptr<font_string> pFont)
{
    if (pHighlightText_ == pCurrentFontString_)
        pCurrentFontString_ = pFont;

    pHighlightText_ = std::move(pFont);
    if (!pHighlightText_)
        return;

    pHighlightText_->set_shown(bHighlighted_);
    pHighlightText_->set_text(sText_);
}

void button::set_disabled_text(utils::observer_ptr<font_string> pFont)
{
    if (pDisabledText_ == pCurrentFontString_)
        pCurrentFontString_ = pFont;

    pDisabledText_ = std::move(pFont);
    if (!pDisabledText_)
        return;

    pDisabledText_->set_shown(mState_ == state::DISABLED);
    pDisabledText_->set_text(sText_);
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
            pHighlightText_->set_offset(mPushedTextOffset_);
        if (pNormalText_)
            pNormalText_->set_offset(mPushedTextOffset_);

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
            pHighlightText_->set_offset(vector2f(0, 0));
        if (pNormalText_)
            pNormalText_->set_offset(vector2f(0, 0));

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
