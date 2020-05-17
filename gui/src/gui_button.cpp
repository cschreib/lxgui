#include "lxgui/gui_button.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"

namespace lxgui {
namespace gui
{
button::button(manager* pManager) : frame(pManager),
    mState_(state::UP), bHighlighted_(false), bLockHighlight_(false),
    pNormalTexture_(nullptr), pPushedTexture_(nullptr), pDisabledTexture_(nullptr),
    pHighlightTexture_(nullptr), pNormalText_(nullptr), pHighlightText_(nullptr),
    pDisabledText_(nullptr), pCurrentFontString_(nullptr),
    mPushedTextOffset_(0, 0)
{
    enable_mouse(true);
    lType_.push_back(CLASS_NAME);
}

button::~button()
{
}

std::string button::serialize(const std::string& sTab) const
{
    return frame::serialize(sTab);
}

void button::create_glue()
{
    if (lGlue_) return;

    lua::state* pLua = pManager_->get_lua();

    if (bVirtual_)
    {
        pLua->push_number(uiID_);
        lGlue_ = pLua->push_new<lua_virtual_glue>();
    }
    else
    {
        pLua->push_string(sName_);
        lGlue_ = pLua->push_new<lua_button>();
    }

    pLua->set_global(sLuaName_);
    pLua->pop();
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

void button::on(const std::string& sScriptName, event* pEvent)
{
    frame::on(sScriptName, pEvent);

    if (is_enabled())
    {
        if (sScriptName == "Enter")
            highlight();

        if (sScriptName == "Leave")
        {
            unlight();

            if (mState_ == state::DOWN)
                release();
        }

        if (sScriptName == "MouseDown")
            push();

        if (sScriptName == "MouseUp")
        {
            release();
            on("Click");
        }
    }
}

void button::on_event(const event& mEvent)
{
    frame::on_event(mEvent);

    if (!pManager_->is_input_enabled())
        return;

    if (mEvent.get_name() == "MOUSE_DOUBLE_CLICKED" && bMouseInFrame_)
        on("DoubleClicked");
}

void button::copy_from(uiobject* pObj)
{
    frame::copy_from(pObj);

    button* pButton = dynamic_cast<button*>(pObj);

    if (pButton)
    {
        this->set_text(pButton->get_text());

        if (pButton->get_normal_texture())
        {
            this->create_normal_texture_();
            if (this->is_virtual())
                pNormalTexture_->set_virtual();
            pNormalTexture_->set_name(pButton->get_normal_texture()->get_name());
            if (!pManager_->add_uiobject(pNormalTexture_))
            {
                gui::out << gui::warning << "gui::" << lType_.back() << " : "
                    "Trying to add \""+pButton->get_normal_texture()->get_name()+"\" to \""+sName_+"\",\n"
                    "but its name was already taken : \""+pNormalTexture_->get_name()+"\". Skipped." << std::endl;
                delete pNormalTexture_; pNormalTexture_ = nullptr;
            }
            else
            {
                if (!is_virtual())
                    pNormalTexture_->create_glue();
                pNormalTexture_->set_draw_layer(pButton->get_normal_texture()->get_draw_layer());
                this->add_region(pNormalTexture_);
                pNormalTexture_->copy_from(pButton->get_normal_texture());
                pNormalTexture_->notify_loaded();
            }
        }
        if (pButton->get_pushed_texture())
        {
            this->create_pushed_texture_();
            if (this->is_virtual())
                pPushedTexture_->set_virtual();
            pPushedTexture_->set_name(pButton->get_pushed_texture()->get_name());
            if (!pManager_->add_uiobject(pPushedTexture_))
            {
                gui::out << gui::warning << "gui::" << lType_.back() << " : "
                    "Trying to add \""+pButton->get_pushed_texture()->get_name()+"\" to \""+sName_+"\",\n"
                    "but its name was already taken : \""+pPushedTexture_->get_name()+"\". Skipped." << std::endl;
                delete pPushedTexture_; pPushedTexture_ = nullptr;
            }
            else
            {
                if (!is_virtual())
                    pPushedTexture_->create_glue();
                pPushedTexture_->set_draw_layer(pButton->get_pushed_texture()->get_draw_layer());
                this->add_region(pPushedTexture_);
                pPushedTexture_->copy_from(pButton->get_pushed_texture());
                pPushedTexture_->notify_loaded();
            }
        }
        if (pButton->get_highlight_texture())
        {
            this->create_highlight_texture_();
            if (this->is_virtual())
                pHighlightTexture_->set_virtual();
            pHighlightTexture_->set_name(pButton->get_highlight_texture()->get_name());
            if (!pManager_->add_uiobject(pHighlightTexture_))
            {
                gui::out << gui::warning << "gui::" << lType_.back() << " : "
                    "Trying to add \""+pButton->get_highlight_texture()->get_name()+"\" to \""+sName_+"\",\n"
                    "but its name was already taken : \""+pHighlightTexture_->get_name()+"\". Skipped." << std::endl;
                delete pHighlightTexture_; pHighlightTexture_ = nullptr;
            }
            else
            {
                if (!is_virtual())
                    pHighlightTexture_->create_glue();
                pHighlightTexture_->set_draw_layer(pButton->get_highlight_texture()->get_draw_layer());
                this->add_region(pHighlightTexture_);
                pHighlightTexture_->copy_from(pButton->get_highlight_texture());
                pHighlightTexture_->notify_loaded();
            }
        }
        if (pButton->get_disabled_texture())
        {
            this->create_disabled_texture_();
            if (this->is_virtual())
                pDisabledTexture_->set_virtual();
            pDisabledTexture_->set_name(pButton->get_disabled_texture()->get_name());
            if (!pManager_->add_uiobject(pDisabledTexture_))
            {
                gui::out << gui::warning << "gui::" << lType_.back() << " : "
                    "Trying to add \""+pButton->get_disabled_texture()->get_name()+"\" to \""+sName_+"\",\n"
                    "but its name was already taken : \""+pDisabledTexture_->get_name()+"\". Skipped." << std::endl;
                delete pDisabledTexture_; pDisabledTexture_ = nullptr;
            }
            else
            {
                if (!is_virtual())
                    pDisabledTexture_->create_glue();
                pDisabledTexture_->set_draw_layer(pButton->get_disabled_texture()->get_draw_layer());
                this->add_region(pDisabledTexture_);
                pDisabledTexture_->copy_from(pButton->get_disabled_texture());
                pDisabledTexture_->notify_loaded();
            }
        }

        if (pButton->get_normal_text())
        {
            this->create_normal_text_();
            if (this->is_virtual())
                pNormalText_->set_virtual();
            pNormalText_->set_name(pButton->get_normal_text()->get_name());
            if (!pManager_->add_uiobject(pNormalText_))
            {
                gui::out << gui::warning << "gui::" << lType_.back() << " : "
                    "Trying to add \""+pButton->get_normal_text()->get_name()+"\" to \""+sName_+"\",\n"
                    "but its name was already taken : \""+pNormalText_->get_name()+"\". Skipped." << std::endl;
                delete pNormalText_; pNormalText_ = nullptr;
            }
            else
            {
                if (!is_virtual())
                    pNormalText_->create_glue();
                pNormalText_->set_draw_layer(pButton->get_normal_text()->get_draw_layer());
                this->add_region(pNormalText_);
                pNormalText_->copy_from(pButton->get_normal_text());
                pNormalText_->notify_loaded();
            }
        }
        if (pButton->get_highlight_text())
        {
            this->create_highlight_text_();
            if (this->is_virtual())
                pHighlightText_->set_virtual();
            pHighlightText_->set_name(pButton->get_highlight_text()->get_name());
            if (!pManager_->add_uiobject(pHighlightText_))
            {
                gui::out << gui::warning << "gui::" << lType_.back() << " : "
                    "Trying to add \""+pButton->get_highlight_text()->get_name()+"\" to \""+sName_+"\",\n"
                    "but its name was already taken : \""+pHighlightText_->get_name()+"\". Skipped." << std::endl;
                delete pHighlightText_; pHighlightText_ = nullptr;
            }
            else
            {
                if (!is_virtual())
                    pHighlightText_->create_glue();
                pHighlightText_->set_draw_layer(pButton->get_highlight_text()->get_draw_layer());
                this->add_region(pHighlightText_);
                pHighlightText_->copy_from(pButton->get_highlight_text());
                pHighlightText_->notify_loaded();
            }
        }
        if (pButton->get_disabled_text())
        {
            this->create_disabled_text_();
                if (this->is_virtual())
                    pDisabledText_->set_virtual();
            pDisabledText_->set_name(pButton->get_disabled_text()->get_name());
            if (!pManager_->add_uiobject(pDisabledText_))
            {
                gui::out << gui::warning << "gui::" << lType_.back() << " : "
                    "Trying to add \""+pButton->get_disabled_text()->get_name()+"\" to \""+sName_+"\",\n"
                    "but its name was already taken : \""+pDisabledText_->get_name()+"\". Skipped." << std::endl;
                delete pDisabledText_; pDisabledText_ = nullptr;
            }
            else
            {
                if (!is_virtual())
                    pDisabledText_->create_glue();
                pDisabledText_->set_draw_layer(pButton->get_disabled_text()->get_draw_layer());
                this->add_region(pDisabledText_);
                pDisabledText_->copy_from(pButton->get_disabled_text());
                pDisabledText_->notify_loaded();
            }
        }

        this->set_pushed_text_offset(pButton->get_pushed_text_offset());

        if (!pButton->is_enabled())
            this->disable();
    }
}

void button::set_text(const std::string& sText)
{
    sText_ = sText;

    if (pNormalText_)
        pNormalText_->set_text(sText);

    if (pHighlightText_)
        pHighlightText_->set_text(sText);

    if (pDisabledText_)
        pDisabledText_->set_text(sText);
}

const std::string& button::get_text() const
{
    return sText_;
}

texture* button::create_normal_texture_()
{
    if (!pNormalTexture_)
    {
        pNormalTexture_ = new texture(pManager_);
        pNormalTexture_->set_special();
        pNormalTexture_->set_parent(this);
        pNormalTexture_->set_draw_layer("BORDER");
    }

    return pNormalTexture_;
}

texture* button::create_pushed_texture_()
{
    if (!pPushedTexture_)
    {
        pPushedTexture_ = new texture(pManager_);
        pPushedTexture_->set_special();
        pPushedTexture_->set_parent(this);
        pPushedTexture_->set_draw_layer("BORDER");
    }

    return pPushedTexture_;
}

texture* button::create_disabled_texture_()
{
    if (!pDisabledTexture_)
    {
        pDisabledTexture_ = new texture(pManager_);
        pDisabledTexture_->set_special();
        pDisabledTexture_->set_parent(this);
        pDisabledTexture_->set_draw_layer("BORDER");
    }

    return pDisabledTexture_;
}

texture* button::create_highlight_texture_()
{
    if (!pHighlightTexture_)
    {
        pHighlightTexture_ = new texture(pManager_);
        pHighlightTexture_->set_special();
        pHighlightTexture_->set_parent(this);
        pHighlightTexture_->set_draw_layer("HIGHLIGHT");
    }

    return pHighlightTexture_;
}

font_string* button::create_normal_text_()
{
    if (!pNormalText_)
    {
        pNormalText_ = new font_string(pManager_);
        pNormalText_->set_special();
        pNormalText_->set_parent(this);
        pNormalText_->set_draw_layer("ARTWORK");
        pCurrentFontString_ = pNormalText_;
    }

    return pNormalText_;
}

font_string* button::create_highlight_text_()
{
    if (!pHighlightText_)
    {
        pHighlightText_ = new font_string(pManager_);
        pHighlightText_->set_special();
        pHighlightText_->set_parent(this);
        pHighlightText_->set_draw_layer("ARTWORK");
    }

    return pHighlightText_;
}

font_string* button::create_disabled_text_()
{
    if (!pDisabledText_)
    {
        pDisabledText_ = new font_string(pManager_);
        pDisabledText_->set_special();
        pDisabledText_->set_parent(this);
        pDisabledText_->set_draw_layer("BORDER");
    }

    return pDisabledText_;
}

texture* button::get_normal_texture()
{
    return pNormalTexture_;
}

texture* button::get_pushed_texture()
{
    return pPushedTexture_;
}

texture* button::get_disabled_texture()
{
    return pDisabledTexture_;
}

texture* button::get_highlight_texture()
{
    return pHighlightTexture_;
}

font_string* button::get_normal_text()
{
    return pNormalText_;
}

font_string* button::get_highlight_text()
{
    return pHighlightText_;
}

font_string* button::get_disabled_text()
{
    return pDisabledText_;
}

void button::set_normal_texture(texture* pTexture)
{
    pNormalTexture_ = pTexture;
}

void button::set_pushed_texture(texture* pTexture)
{
    pPushedTexture_ = pTexture;
}

void button::set_disabled_texture(texture* pTexture)
{
    pDisabledTexture_ = pTexture;
}

void button::set_highlight_texture(texture* pTexture)
{
    pHighlightTexture_ = pTexture;
}

void button::set_normal_text(font_string* pFont)
{
    if (pNormalText_ == pCurrentFontString_)
        pCurrentFontString_ = pFont;

    pNormalText_ = pFont;
}

void button::set_highlight_text(font_string* pFont)
{
    pHighlightText_ = pFont;
}

void button::set_disabled_text(font_string* pFont)
{
    pDisabledText_ = pFont;
}

font_string* button::get_CurrentFontString()
{
    return pCurrentFontString_;
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

        on("Disable");
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

        on("Enable");
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
    if (!bMouseInFrame_)
        unlight();

    bLockHighlight_ = false;
}

void button::set_pushed_text_offset(const vector2i& mOffset)
{
    mPushedTextOffset_ = mOffset;
    notify_renderer_need_redraw();
}

const vector2i& button::get_pushed_text_offset() const
{
    return mPushedTextOffset_;
}
}
}
