#include "lxgui/gui_editbox.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_sprite.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/input.hpp"

using namespace input;

namespace gui
{
#ifdef NO_CPP11_CONSTEXPR
const char* edit_box::CLASS_NAME = "EditBox";
#endif

edit_box::edit_box(manager* pManager) : focus_frame(pManager),
    uiDisplayPos_(0), uiNumLetters_(0), uiMaxLetters_(-1), bNumericOnly_(false),
    bPositiveOnly_(false), bIntegerOnly_(false), bPasswordMode_(false),
    bMultiLine_(false), bArrowsIgnored_(false),
    pHighlight_(nullptr), mHighlightColor_(1.0f, 1.0f, 1.0f, 0.35f),
    uiSelectionStartPos_(0), uiSelectionEndPos_(0), bSelectedText_(false),
    pCarret_(nullptr), dBlinkSpeed_(0.5),
    mCarretTimer_(dBlinkSpeed_, periodic_timer::START_FIRST_TICK, false),
    uiMaxHistoryLines_(uint(-1)), pFontString_(nullptr),
    lTextInsets_(quad2i::ZERO), uiLastKeyPressed_(0u), dKeyRepeatSpeed_(0.03),
    mKeyRepeatTimer_(dKeyRepeatSpeed_, periodic_timer::START_FIRST_TICK, true)
{
    lType_.push_back(CLASS_NAME);

    iterCarretPos_ = sUnicodeText_.begin();

    std::vector<std::string> lRegs; lRegs.push_back("LeftButton");
    register_for_drag(lRegs);
}

edit_box::~edit_box()
{
    set_focus(false);
}

bool edit_box::can_use_script(const std::string& sScriptName) const
{
    if (frame::can_use_script(sScriptName))
        return true;
    else if ((sScriptName == "OnChar") ||
        (sScriptName == "OnCursorChanged") ||
        (sScriptName == "OnEditFocusGained") ||
        (sScriptName == "OnEditFocusLost") ||
        (sScriptName == "OnEnterPressed") ||
        (sScriptName == "OnEscapePressed") ||
        (sScriptName == "OnSpacePressed") ||
        (sScriptName == "OnTabPressed") ||
        (sScriptName == "OnTextChanged") ||
        (sScriptName == "OnTextSet"))
        return true;
    else
        return false;
}

void edit_box::copy_from(uiobject* pObj)
{
    focus_frame::copy_from(pObj);

    edit_box* pEditBox = dynamic_cast<edit_box*>(pObj);

    if (pEditBox)
    {
        this->set_max_letters(pEditBox->get_max_letters());
        this->set_blink_speed(pEditBox->get_blink_speed());
        this->set_numeric_only(pEditBox->is_numeric_only());
        this->set_positive_only(pEditBox->is_positive_only());
        this->set_integer_only(pEditBox->is_integer_only());
        this->enable_password_mode(pEditBox->is_password_mode_enabled());
        this->set_multi_line(pEditBox->is_multi_line());
        this->set_max_history_lines(pEditBox->get_max_history_lines());
        this->set_text_insets(pEditBox->get_text_insets());

        font_string* pFS = pEditBox->get_font_string();
        if (pFS)
        {
            this->create_font_string_();

            if (this->is_virtual())
                pFontString_->set_virtual();

            pFontString_->set_name(pFS->get_name());
            if (!pManager_->add_uiobject(pFontString_))
            {
                gui::out << gui::warning << "gui::" << lType_.back() << " : "
                    "Trying to add \""+pFS->get_name()+"\" to \""+sName_+"\",\n"
                    "but its name was already taken : \""+pFontString_->get_name()+"\". Skipped." << std::endl;
                delete pFontString_; pFontString_ = nullptr;
            }
            else
            {
                if (!is_virtual())
                    pFontString_->create_glue();

                this->add_region(pFontString_);
                pFontString_->copy_from(pFS);

                if (!is_virtual())
                    pFontString_->enable_formatting(false);

                pFontString_->notify_loaded();
            }
        }
    }
}

void edit_box::update(float fDelta)
{
    frame::update(fDelta);

    if (bMouseDragged_)
    {
        uint uiPos = get_letter_id_at_(iMousePosX_, iMousePosY_);
        if (uiPos != uiSelectionEndPos_)
        {
            if (uiPos != uint(-1))
            {
                highlight_text(uiSelectionStartPos_, uiPos);
                iterCarretPos_ = sUnicodeText_.begin() + uiPos;
                update_carret_position_();
            }
            else
            {
                uint uiTemp = uiSelectionStartPos_;
                unlight_text();
                uiSelectionStartPos_ = uiTemp;
                iterCarretPos_ = sUnicodeText_.begin() + uiSelectionStartPos_;
                update_carret_position_();
            }
        }
    }

    if (bFocus_)
    {
        mCarretTimer_.update(fDelta);

        if (mCarretTimer_.ticks())
        {
            if (!pCarret_)
                create_carret_();

            if (pCarret_)
            {
                if (pCarret_->is_shown())
                    pCarret_->hide();
                else
                    pCarret_->show();
            }
        }
    }

    if (bFocus_ && uiLastKeyPressed_ != 0u &&
        pManager_->get_input_manager()->key_is_down_long((key::code)uiLastKeyPressed_, true))
    {
        if (mKeyRepeatTimer_.is_paused())
            mKeyRepeatTimer_.start();

        mKeyRepeatTimer_.update(fDelta);

        if (mKeyRepeatTimer_.ticks())
            process_key_(uiLastKeyPressed_);
    }
}

void edit_box::on_event(const event& mEvent)
{
    frame::on_event(mEvent);

    if (!pManager_->is_input_enabled())
        return;

    if (mEvent.get_name() == "TEXT_ENTERED" && bFocus_)
    {
        char32_t c = mEvent.get<char32_t>(0);
        if (add_char_(c))
        {
            event mKeyEvent;
            mKeyEvent.add(utils::unicode_to_UTF8(utils::ustring(1, c)));
            on("Char", &mKeyEvent);
        }
        return;
    }

    if (mEvent.get_name() == "MOUSE_PRESSED" && bMouseInFrame_)
    {
        set_focus(true);
        unlight_text();

        move_carret_at_(iMousePosX_, iMousePosY_);
        return;
    }

    if (mEvent.get_name() == "KEY_PRESSED" && bFocus_)
    {
        uint uiChar = mEvent[0].get<uint>();
        if (uiChar == key::K_RETURN || uiChar == key::K_NUMPADENTER)
            on("EnterPressed");
        else if (uiChar == key::K_END)
        {
            uint uiPreviousCarretPos = iterCarretPos_ - sUnicodeText_.begin();

            iterCarretPos_ = sUnicodeText_.end();
            update_carret_position_();

            if (pManager_->get_input_manager()->shift_is_pressed())
            {
                if (bSelectedText_)
                    highlight_text(uiSelectionStartPos_, iterCarretPos_ - sUnicodeText_.begin());
                else
                    highlight_text(uiPreviousCarretPos, iterCarretPos_ - sUnicodeText_.begin());
            }
            else
                unlight_text();

            return;
        }
        else if (uiChar == key::K_HOME)
        {
            uint uiPreviousCarretPos = iterCarretPos_ - sUnicodeText_.begin();

            iterCarretPos_ = sUnicodeText_.begin();
            update_carret_position_();

            if (pManager_->get_input_manager()->shift_is_pressed())
            {
                if (bSelectedText_)
                    highlight_text(uiSelectionStartPos_, iterCarretPos_ - sUnicodeText_.begin());
                else
                    highlight_text(uiPreviousCarretPos, iterCarretPos_ - sUnicodeText_.begin());
            }
            else
                unlight_text();

            return;
        }
        else if (uiChar == key::K_TAB)
            on("TabPressed");
        else if (uiChar == key::K_SPACE)
            on("SpacePressed");

        uiLastKeyPressed_ = uiChar;

        process_key_(uiChar);
    }
    else if (mEvent.get_name() == "KEY_RELEASED")
    {
        uint uiChar = mEvent[0].get<uint>();

        if (uiChar == key::K_ESCAPE)
        {
            on("EscapePressed");
            return;
        }

        if (uiChar == uiLastKeyPressed_)
        {
            uiLastKeyPressed_ = 0u;
            mKeyRepeatTimer_.stop();
        }
    }
}

void edit_box::enable_keyboard(bool bIsKeyboardEnabled)
{
    if (!bVirtual_)
    {
        if (bIsKeyboardEnabled && !bIsKeyboardEnabled_)
            event_receiver::register_event("TEXT_ENTERED");
        else if (!bIsKeyboardEnabled && bIsKeyboardEnabled_)
            event_receiver::unregister_event("TEXT_ENTERED");
    }

    frame::enable_keyboard(bIsKeyboardEnabled);
}

void edit_box::on(const std::string& sScriptName, event* pEvent)
{
    if (bFocus_ && (sScriptName == "KeyUp" || sScriptName == "KeyDown"))
        return;

    if (lDefinedScriptList_.find(sScriptName) != lDefinedScriptList_.end())
    {
        utils::wptr<lua::state> pLua = pManager_->get_lua();

        if (sScriptName == "Char")
        {
            // set_ key name
            if (pEvent)
            {
                pLua->push_string(pEvent->get<std::string>(0));
                pLua->set_global("arg1");
            }
        }
    }

    frame::on(sScriptName, pEvent);

    if (sScriptName == "SizeChanged")
    {
        update_displayed_text_();
        update_font_string_();
        update_carret_position_();
    }

    if (sScriptName == "DragStart")
        uiSelectionEndPos_ = uiSelectionStartPos_ = get_letter_id_at_(iMousePosX_, iMousePosY_);
}

void edit_box::create_glue()
{
    if (lGlue_) return;

    if (bVirtual_)
    {
        utils::wptr<lua::state> pLua = pManager_->get_lua();
        pLua->push_number(uiID_);
        lGlue_ = pLua->push_new<lua_virtual_glue>();
        pLua->set_global(sLuaName_);
        pLua->pop();
    }
    else
    {
        utils::wptr<lua::state> pLua = pManager_->get_lua();
        pLua->push_string(sLuaName_);
        lGlue_ = pLua->push_new<lua_edit_box>();
        pLua->set_global(sLuaName_);
        pLua->pop();
    }
}

void edit_box::set_text(const std::string& sText)
{
    if (sText != sText_)
    {
        unlight_text();
        sText_ = sText;
        check_text_();
        update_displayed_text_();
        iterCarretPos_ = sUnicodeText_.end();
        update_font_string_();
        update_carret_position_();
        on("TextSet");
        on("TextChanged");
    }
}

const std::string& edit_box::get_text() const
{
    return sText_;
}

void edit_box::unlight_text()
{
    uiSelectionStartPos_ = uiSelectionEndPos_ = 0u;
    bSelectedText_ = false;

    if (pHighlight_)
        pHighlight_->hide();
}

void edit_box::highlight_text(uint uiStart, uint uiEnd, bool bForceUpdate)
{
    if (!pHighlight_)
        create_highlight_();

    if (!pHighlight_)
        return;

    uint uiLeft  = std::min(uiStart, uiEnd);
    uint uiRight = std::max(uiStart, uiEnd);

    if (uiLeft == uiRight || uiRight < uiDisplayPos_ || uiLeft >= uiDisplayPos_ + sDisplayedText_.size())
        pHighlight_->hide();
    else
        pHighlight_->show();

    if (uiSelectionStartPos_ != uiStart || uiSelectionEndPos_ != uiEnd || bForceUpdate)
    {
        if (uiLeft != uiRight && uiRight >= uiDisplayPos_ &&
            uiLeft < uiDisplayPos_ + sDisplayedText_.size() &&
            pFontString_ && pFontString_->get_text_object())
        {
            bSelectedText_ = true;

            int iLeftPos = 0;
            int iRightPos = pFontString_->get_right() - pFontString_->get_left();

            utils::wptr<text> pText = pFontString_->get_text_object();
            const std::vector<text::letter>& lLetters = pText->get_letter_cache();

            std::vector<text::letter>::const_iterator iter;
            foreach (iter, lLetters)
            {
                uint uiPos = iter - lLetters.begin() + uiDisplayPos_;

                if (uiPos == uiLeft)
                    iLeftPos = int(iter->mQuad.left) + lTextInsets_.left;

                if (uiPos == uiRight - 1)
                {
                    iRightPos = int(iter->mQuad.right) + lTextInsets_.left;
                    break;
                }
            }

            pHighlight_->set_abs_point(ANCHOR_LEFT,  sName_, ANCHOR_LEFT, iLeftPos,  0);
            pHighlight_->set_abs_point(ANCHOR_RIGHT, sName_, ANCHOR_LEFT, iRightPos, 0);
        }
    }

    uiSelectionStartPos_ = uiStart;
    uiSelectionEndPos_   = uiEnd;
}

void edit_box::set_highlight_color(const color& mColor)
{
    if (mHighlightColor_ != mColor)
    {
        mHighlightColor_ = mColor;

        if (!pHighlight_)
            create_highlight_();

        if (!pHighlight_)
            return;

        pHighlight_->set_color(mHighlightColor_);
    }
}

void edit_box::insert_after_cursor(const std::string& sText)
{
    if (!sText.empty())
    {
        if (bNumericOnly_ && !utils::is_number(sText))
            return;

        utils::ustring sUStr = utils::UTF8_to_unicode(sText);
        if (sUnicodeText_.size() + sUStr.size() <= uiMaxLetters_)
        {
            unlight_text();
            sUnicodeText_.insert(iterCarretPos_, sUStr.begin(), sUStr.end());
            iterCarretPos_ += sUStr.size();
            sText_ = utils::unicode_to_UTF8(sUnicodeText_);
            uiNumLetters_ = sUnicodeText_.size();

            update_displayed_text_();
            update_font_string_();
            update_carret_position_();
        }
    }
}

void edit_box::set_max_letters(uint uiMaxLetters)
{
    if (uiMaxLetters == 0)
    {
        uiMaxLetters_ = uint(-1);
        return;
    }

    if (uiMaxLetters_ != uiMaxLetters)
    {
        uiMaxLetters_ = uiMaxLetters;

        uint uiCarretPos = iterCarretPos_ - sUnicodeText_.begin();

        check_text_();

        if (uiCarretPos > uiMaxLetters_)
        {
            iterCarretPos_ = sUnicodeText_.end();
            update_displayed_text_();
            update_font_string_();
            update_carret_position_();
        }
        else
            iterCarretPos_ = sUnicodeText_.begin() + uiCarretPos;
    }
}

uint edit_box::get_max_letters() const
{
    return uiMaxLetters_;
}

uint edit_box::get_num_letters() const
{
    return uiNumLetters_;
}

void edit_box::set_blink_speed(const double& dBlinkSpeed)
{
    if (dBlinkSpeed_ != dBlinkSpeed)
    {
        dBlinkSpeed_ = dBlinkSpeed;
        mCarretTimer_ = periodic_timer(dBlinkSpeed_, periodic_timer::START_FIRST_TICK, false);
    }
}

const double& edit_box::get_blink_speed() const
{
    return dBlinkSpeed_;
}

void edit_box::set_numeric_only(bool bNumericOnly)
{
    if (bNumericOnly_ != bNumericOnly)
    {
        bNumericOnly_ = bNumericOnly;

        if (bNumericOnly_)
        {
            check_text_();
            iterCarretPos_ = sUnicodeText_.end();
            update_displayed_text_();
            update_carret_position_();
        }
    }
}

void edit_box::set_positive_only(bool bPositiveOnly)
{
    if (bPositiveOnly_ != bPositiveOnly)
    {
        bPositiveOnly_ = bPositiveOnly;

        if (bNumericOnly_ && bPositiveOnly_)
        {
            check_text_();
            iterCarretPos_ = sUnicodeText_.end();
            update_displayed_text_();
            update_carret_position_();
        }
    }
}

void edit_box::set_integer_only(bool bIntegerOnly)
{
    if (bIntegerOnly_ != bIntegerOnly)
    {
        bIntegerOnly_ = bIntegerOnly;

        if (bIntegerOnly_ && bPositiveOnly_)
        {
            check_text_();
            iterCarretPos_ = sUnicodeText_.end();
            update_displayed_text_();
            update_carret_position_();
        }
    }
}

bool edit_box::is_numeric_only() const
{
    return bNumericOnly_;
}

bool edit_box::is_positive_only() const
{
    return bPositiveOnly_;
}

bool edit_box::is_integer_only() const
{
    return bIntegerOnly_;
}

void edit_box::enable_password_mode(bool bEnable)
{
    if (bPasswordMode_ != bEnable)
    {
        bPasswordMode_ = bEnable;

        update_displayed_text_();
        update_font_string_();
        update_carret_position_();
    }
}

bool edit_box::is_password_mode_enabled() const
{
    return bPasswordMode_;
}

void edit_box::set_multi_line(bool bMultiLine)
{
    if (bMultiLine_ != bMultiLine)
    {
        bMultiLine_ = bMultiLine;

        if (pFontString_)
            pFontString_->set_word_wrap(bMultiLine_, bMultiLine_);

        check_text_();
        iterCarretPos_ = sUnicodeText_.end();
        update_displayed_text_();
        update_carret_position_();
    }
}

bool edit_box::is_multi_line() const
{
    return bMultiLine_;
}

void edit_box::set_max_history_lines(uint uiMaxHistoryLines)
{
    if (uiMaxHistoryLines == 0)
    {
        uiMaxHistoryLines_ = uint(-1);
        return;
    }

    if (uiMaxHistoryLines_ != uiMaxHistoryLines)
    {
        uiMaxHistoryLines_ = uiMaxHistoryLines;

        while (lHistoryLineList_.size() > uiMaxHistoryLines_)
            lHistoryLineList_.pop_front();
    }
}

uint edit_box::get_max_history_lines() const
{
    return uiMaxHistoryLines_;
}

void edit_box::add_history_line(const std::string& sHistoryLine)
{
    lHistoryLineList_.push_back(sHistoryLine);

    while (lHistoryLineList_.size() > uiMaxHistoryLines_)
        lHistoryLineList_.pop_front();
}

const std::deque<std::string>& edit_box::get_history_lines() const
{
    return lHistoryLineList_;
}

void edit_box::set_arrows_ignored(bool bArrowsIgnored)
{
    bArrowsIgnored_ = bArrowsIgnored;
}

void edit_box::set_text_insets(int iLeft, int iRight, int iTop, int iBottom)
{
    set_text_insets(quad2i(iLeft, iRight, iTop, iBottom));
}

void edit_box::set_text_insets(const quad2i& lInsets)
{
    lTextInsets_ = lInsets;

    if (pFontString_)
    {
        pFontString_->clear_all_points();
        pFontString_->set_abs_point(
            ANCHOR_TOPLEFT, sName_, ANCHOR_TOPLEFT, lTextInsets_.top_left()
        );
        pFontString_->set_abs_point(
            ANCHOR_BOTTOMRIGHT, sName_, ANCHOR_BOTTOMRIGHT, lTextInsets_.bottom_right()
        );

        update_displayed_text_();
        update_font_string_();
        update_carret_position_();
    }
}

const quad2i& edit_box::get_text_insets() const
{
    return lTextInsets_;
}

void edit_box::notify_focus(bool bFocus)
{
    if (bFocus_ != bFocus)
    {
        bFocus_ = bFocus;
        if (bFocus_)
        {
            if (!pCarret_)
                create_carret_();

            if (pCarret_)
                pCarret_->show();

            mCarretTimer_.zero();

            lQueuedEventList_.push_back("EditFocusGained");
        }
        else
        {
            if (pCarret_)
                pCarret_->hide();

            lQueuedEventList_.push_back("EditFocusLost");
        }
    }
}

void edit_box::notify_invisible_(bool bTriggerEvents)
{
    set_focus(false);
    frame::notify_invisible_(bTriggerEvents);
}

font_string* edit_box::get_font_string()
{
    return pFontString_;
}

void edit_box::set_font_string(font_string* pFont)
{
    pFontString_ = pFont;
    if (pFontString_)
    {
        pFontString_->set_special();
        pFontString_->set_parent(this);
        pFontString_->set_word_wrap(bMultiLine_, bMultiLine_);

        pFontString_->set_abs_dimensions(0u, 0u);
        pFontString_->clear_all_points();
        pFontString_->set_abs_point(
            ANCHOR_TOPLEFT,     "$parent", ANCHOR_TOPLEFT,      lTextInsets_.top_left()
        );
        pFontString_->set_abs_point(
            ANCHOR_BOTTOMRIGHT, "$parent", ANCHOR_BOTTOMRIGHT, -lTextInsets_.bottom_right()
        );

        pFontString_->enable_formatting(false);
    }
}

font_string* edit_box::create_font_string_()
{
    font_string* pFont = new font_string(pManager_);
    pFont->set_draw_layer(LAYER_ARTWORK);
    set_font_string(pFont);

    return pFontString_;
}

void edit_box::create_highlight_()
{
    pHighlight_ = new texture(pManager_);
    pHighlight_->set_special();
    pHighlight_->set_parent(this);
    pHighlight_->set_draw_layer(LAYER_HIGHLIGHT);
    pHighlight_->set_name("$parentHighlight");

    if (!pManager_->add_uiobject(pHighlight_))
    {
        gui::out << gui::warning << "gui::" << lType_.back() << " : "
            "Trying to create highlight texture for \""+sName_+"\",\n"
            "but its name was already taken : \""+pHighlight_->get_name()+"\". Skipped." << std::endl;
        delete pHighlight_; pHighlight_ = nullptr;
        return;
    }

    pHighlight_->create_glue();
    add_region(pHighlight_);

    pHighlight_->set_abs_point(
        ANCHOR_TOP,    sName_, ANCHOR_TOP,    0,  lTextInsets_.top
    );
    pHighlight_->set_abs_point(
        ANCHOR_BOTTOM, sName_, ANCHOR_BOTTOM, 0, -lTextInsets_.bottom
    );

    pHighlight_->set_color(mHighlightColor_);
    pHighlight_->notify_loaded();
}

void edit_box::create_carret_()
{
    if (pFontString_ && pFontString_->get_text_object())
    {
        pCarret_ = new texture(pManager_);
        pCarret_->set_special();
        pCarret_->set_parent(this);
        pCarret_->set_draw_layer(LAYER_HIGHLIGHT);
        pCarret_->set_name("$parentCarret");

        if (!pManager_->add_uiobject(pCarret_))
        {
            gui::out << gui::warning << "gui::" << lType_.back() << " : "
                "Trying to create carret texture for \""+sName_+"\",\n"
                "but its name was already taken : \""+pCarret_->get_name()+"\". Skipped." << std::endl;
            delete pCarret_; pCarret_ = nullptr;
            return;
        }

        pCarret_->create_glue();
        add_region(pCarret_);

        utils::refptr<sprite> pSprite = pFontString_->get_text_object()->create_sprite(TO_U('|'));
        pSprite->set_color(pFontString_->get_text_color());

        pCarret_->set_sprite(pSprite);
        pCarret_->set_abs_point(ANCHOR_CENTER, sName_, ANCHOR_LEFT, lTextInsets_.left - 1, 0);

        pCarret_->notify_loaded();
    }
}

void edit_box::check_text_()
{
    sUnicodeText_ = utils::UTF8_to_unicode(sText_);

    if (!utils::is_number(sText_) && bNumericOnly_)
        sUnicodeText_.clear();
    else
    {
        if (sUnicodeText_.size() > uiMaxLetters_)
            sUnicodeText_.resize(uiMaxLetters_);
    }

    uiNumLetters_ = sUnicodeText_.size();

    sText_ = utils::unicode_to_UTF8(sUnicodeText_);
}

void edit_box::update_displayed_text_()
{
    if (pFontString_ && pFontString_->get_text_object())
    {
        if (bPasswordMode_)
            sDisplayedText_ = utils::ustring(utils::UTF8_to_unicode('*'), sUnicodeText_.size());
        else
            sDisplayedText_ = sUnicodeText_;

        if (!bMultiLine_)
        {
            utils::wptr<text> pTextObject = pFontString_->get_text_object();

            if (!math::isinf(pTextObject->get_box_width()))
            {
                sDisplayedText_.erase(0, uiDisplayPos_);

                while (pTextObject->get_string_width(sDisplayedText_) > pTextObject->get_box_width())
                    sDisplayedText_.erase(sDisplayedText_.size()-2, 1);
            }
        }
    }
}

void edit_box::update_font_string_()
{
    if (!pFontString_)
        return;

    pFontString_->set_text(utils::unicode_to_UTF8(sDisplayedText_));

    if (bSelectedText_)
        highlight_text(uiSelectionStartPos_, uiSelectionEndPos_, true);
}

void edit_box::update_carret_position_()
{
    if (pFontString_ && pFontString_->get_text_object() && pCarret_)
    {
        if (sDisplayedText_.empty())
        {
            anchor_point mPoint;
            int iOffset = 0;
            switch (pFontString_->get_justify_h())
            {
                case text::ALIGN_LEFT :
                    mPoint = ANCHOR_LEFT;
                    iOffset = lTextInsets_.left - 1;
                    break;
                case text::ALIGN_CENTER :
                    mPoint = ANCHOR_CENTER;
                    break;
                case text::ALIGN_RIGHT :
                    mPoint = ANCHOR_RIGHT;
                    iOffset = -lTextInsets_.right - 1;
                    break;
                default : mPoint = ANCHOR_LEFT; break;
            }
            pCarret_->set_abs_point(
                ANCHOR_CENTER, sName_, mPoint, iOffset, 0
            );
            return;
        }

        utils::wptr<text> pText = pFontString_->get_text_object();
        utils::ustring::iterator iterDisplayCarret;

        if (!bMultiLine_)
        {
            uint uiGlobalPos = iterCarretPos_ - sUnicodeText_.begin();

            if (uiDisplayPos_ > uiGlobalPos)
            {
                // The carret has been positioned before the start of the displayed string
                float fBoxWidth = pText->get_box_width();
                float fLeftStringMaxSize = fBoxWidth*0.25f;
                float fLeftStringSize = 0.0f;
                utils::ustring sLeftString;

                utils::ustring::iterator iter = iterCarretPos_;
                while ((iter != sUnicodeText_.begin()) && (fLeftStringSize < fLeftStringMaxSize))
                {
                    --iter;
                    sLeftString.insert(sLeftString.begin(), *iter);
                    fLeftStringSize = pText->get_string_width(sLeftString);
                }

                uiDisplayPos_ = iter - sUnicodeText_.begin();
                update_displayed_text_();
                update_font_string_();
            }

            uint uiCarretPos = uiGlobalPos - uiDisplayPos_;
            if (uiCarretPos > sDisplayedText_.size())
            {
                // The carret has been positioned after the end of the displayed string
                float fBoxWidth = pText->get_box_width();
                float fLeftStringMaxSize = fBoxWidth*0.75f;
                float fLeftStringSize = 0.0f;
                utils::ustring sLeftString;

                utils::ustring::iterator iter = iterCarretPos_;
                while ((iterCarretPos_ != sUnicodeText_.begin()) && (fLeftStringSize < fLeftStringMaxSize))
                {
                    --iter;
                    sLeftString.insert(sLeftString.begin(), *iter);
                    fLeftStringSize = pText->get_string_width(sLeftString);
                }

                uiDisplayPos_ = iter - sUnicodeText_.begin();
                update_displayed_text_();
                update_font_string_();

                uiCarretPos = uiGlobalPos - uiDisplayPos_;
            }

            iterDisplayCarret = sDisplayedText_.begin() + uiCarretPos;
        }
        else
            iterDisplayCarret = sDisplayedText_.begin() + (iterCarretPos_ - sUnicodeText_.begin()) - uiDisplayPos_;

        const std::vector<text::letter>& lLetters = pText->get_letter_cache();
        std::vector<text::letter>::const_iterator iterLetter = lLetters.begin();

        float fYOffset = 0.0f;

        utils::ustring::const_iterator iter;
        for (iter = sDisplayedText_.begin(); iter != iterDisplayCarret; ++iter)
        {
            float fLastY = iterLetter->mQuad.top;
            ++iterLetter;
            if (iterLetter != lLetters.end() && fLastY != iterLetter->mQuad.top)
                fYOffset += iterLetter->mQuad.top - fLastY;
        }

        if (iterLetter == lLetters.begin())
        {
            pCarret_->set_abs_point(
                ANCHOR_CENTER, sName_, ANCHOR_LEFT,
                lTextInsets_.left + int(iterLetter->mQuad.left) - 1, int(fYOffset)
            );
        }
        else
        {
            --iterLetter;
            pCarret_->set_abs_point(
                ANCHOR_CENTER, sName_, ANCHOR_LEFT,
                lTextInsets_.left + int(iterLetter->mQuad.right) - 1, int(fYOffset)
            );
        }

        mCarretTimer_.zero();
        pCarret_->show();
    }
}

bool edit_box::add_char_(char32_t sUnicode)
{
    if (bSelectedText_)
        remove_char_();

    if (uiNumLetters_ + 1 >= uiMaxLetters_)
        return false;

    if (bNumericOnly_)
    {
        if (sUnicode == TO_U('.'))
        {
            if (bIntegerOnly_)
                return false;

            if (sUnicodeText_.find(TO_U('.')))
                return false;
        }
        else if (sUnicode == TO_U('+') || sUnicode == TO_U('-'))
        {
            if (bPositiveOnly_)
                return false;

            if (iterCarretPos_ != sUnicodeText_.begin() ||
                sUnicodeText_.find(TO_U('+')) || sUnicodeText_.find(TO_U('-')))
                return false;
        }
        else if (sUnicode < TO_U('0') || sUnicode > TO_U('9'))
            return false;
    }

    iterCarretPos_ = sUnicodeText_.insert(iterCarretPos_, sUnicode) + 1;
    ++uiNumLetters_;

    sText_ = utils::unicode_to_UTF8(sUnicodeText_);

    update_displayed_text_();
    update_font_string_();
    update_carret_position_();

    if (pCarret_)
        pCarret_->show();

    mCarretTimer_.zero();

    on("TextChanged");

    return true;
}

bool edit_box::remove_char_()
{
    if (bSelectedText_)
    {
        if (uiSelectionStartPos_ != uiSelectionEndPos_)
        {
            uint uiLeft = std::min(uiSelectionStartPos_, uiSelectionEndPos_);
            uint uiRight = std::max(uiSelectionStartPos_, uiSelectionEndPos_);

            sUnicodeText_.erase(uiLeft, uiRight - uiLeft);
            sText_ = utils::unicode_to_UTF8(sUnicodeText_);
            uiNumLetters_ = sUnicodeText_.size();

            iterCarretPos_ = sUnicodeText_.begin() + uiLeft;
            on("TextChanged");
        }

        unlight_text();
    }
    else
    {
        if (iterCarretPos_ == sUnicodeText_.end())
            return false;

        iterCarretPos_ = sUnicodeText_.erase(iterCarretPos_);
        sText_ = utils::unicode_to_UTF8(sUnicodeText_);
        --uiNumLetters_;
        on("TextChanged");
    }

    update_displayed_text_();
    update_font_string_();
    update_carret_position_();

    if (pCarret_)
        pCarret_->show();

    mCarretTimer_.zero();

    return true;
}

uint edit_box::get_letter_id_at_(int iX, int iY)
{
    if (pFontString_ && pFontString_->get_text_object())
    {
        utils::wptr<text> pText = pFontString_->get_text_object();
        const std::vector<text::letter>& lLetters = pText->get_letter_cache();

        if (lLetters.empty())
            return uiDisplayPos_;

        float fX = float(iX - lBorderList_.left - lTextInsets_.left);
        //float fY = float(iY - lBorderList_.top  - lTextInsets_.top);

        if (!bMultiLine_)
        {
            if (iX < lBorderList_.left + lTextInsets_.left)
                return uiDisplayPos_;
            else if (iX > lBorderList_.right - lTextInsets_.right)
                return lLetters.size() + uiDisplayPos_;

            std::vector<text::letter>::const_iterator iter;
            foreach (iter, lLetters)
            {
                if (fX < iter->mQuad.center().x)
                    return (iter - lLetters.begin()) + uiDisplayPos_;
            }

            return lLetters.size() + uiDisplayPos_;
        }
        else
        {
            // TODO : Implement for multi line edit_box
            return uiDisplayPos_;
        }

    }

    return uint(-1);
}

bool edit_box::move_carret_at_(int iX, int iY)
{
    uint uiPos = get_letter_id_at_(iX, iY);
    if (uiPos != uint(-1))
    {
        iterCarretPos_ = sUnicodeText_.begin() + uiPos;
        update_carret_position_();
        return true;
    }
    else
        return false;
}

bool edit_box::move_carret_horizontally_(bool bForward)
{
    if (bForward)
    {
        if (iterCarretPos_ != sUnicodeText_.end())
        {
            ++iterCarretPos_;
            update_carret_position_();

            if (pCarret_)
                pCarret_->show();

            mCarretTimer_.zero();

            return true;
        }
        else
            return false;
    }
    else
    {
        if (iterCarretPos_ != sUnicodeText_.begin())
        {
            --iterCarretPos_;
            update_carret_position_();

            if (pCarret_)
                pCarret_->show();

            mCarretTimer_.zero();

            return true;
        }
        else
            return false;
    }
}

bool edit_box::move_carret_vertically_(bool bDown)
{
    if (bMultiLine_)
    {
        // TODO : Implement for multi line edit_box
        return false;
    }
    else
    {
        utils::ustring::iterator iterOld = iterCarretPos_;

        if (bDown)
            iterCarretPos_ = sUnicodeText_.end();
        else
            iterCarretPos_ = sUnicodeText_.begin();

        if (iterOld != iterCarretPos_)
        {
            update_carret_position_();

            if (pCarret_)
                pCarret_->show();

            mCarretTimer_.zero();

            return true;
        }
        else
            return false;
    }
}

void edit_box::process_key_(uint uiKey)
{
    if (uiKey == key::K_RETURN || uiKey == key::K_NUMPADENTER)
    {
        if (bMultiLine_)
        {
            if (add_char_(TO_U('\n')))
            {
                event mKeyEvent;
                mKeyEvent.add(std::string("\n"));
                on("OnChar", &mKeyEvent);
            }
        }
    }
    else if (uiKey == key::K_BACK)
    {
        if (!bSelectedText_)
        {
            if (move_carret_horizontally_(false))
                remove_char_();
        }
        else
            remove_char_();
    }
    else if (uiKey == key::K_DELETE)
        remove_char_();
    else if (uiKey == key::K_LEFT || uiKey == key::K_RIGHT || uiKey == key::K_UP || uiKey == key::K_DOWN)
    {
        if (!bArrowsIgnored_)
        {
            uint uiPreviousCarretPos = iterCarretPos_ - sUnicodeText_.begin();

            if (uiKey == key::K_LEFT)
                move_carret_horizontally_(false);
            else if (uiKey == key::K_RIGHT)
                move_carret_horizontally_(true);
            else if (uiKey == key::K_UP)
                move_carret_vertically_(false);
            else if (uiKey == key::K_DOWN)
                move_carret_vertically_(true);

            if (pManager_->get_input_manager()->shift_is_pressed())
            {
                if (bSelectedText_)
                    highlight_text(uiSelectionStartPos_, iterCarretPos_ - sUnicodeText_.begin());
                else
                    highlight_text(uiPreviousCarretPos, iterCarretPos_ - sUnicodeText_.begin());
            }
            else
                unlight_text();
        }
    }
}

periodic_timer::periodic_timer(const double& dDuration, start_type mType, bool bTickFirst) :
    dElapsed_(bTickFirst ? dDuration : 0.0), dDuration_(dDuration), bPaused_(true),
    bFirstTick_(true), mType_(mType)
{
    if (mType == START_NOW)
        start();
}

const double& periodic_timer::get_elapsed()
{
    return dElapsed_;
}

const double& periodic_timer::get_period() const
{
    return dDuration_;
}

bool periodic_timer::is_paused() const
{
    return bPaused_;
}

bool periodic_timer::ticks()
{
    if (mType_ == START_FIRST_TICK && bFirstTick_)
    {
        start();
        bFirstTick_ = false;
    }

    if (dElapsed_ >= dDuration_)
    {
        if (!bPaused_)
            zero();

        return true;
    }
    else
        return false;
}

void periodic_timer::stop()
{
    dElapsed_ = 0.0;
    bPaused_ = true;
}

void periodic_timer::pause()
{
    bPaused_ = true;
}

void periodic_timer::start()
{
    bPaused_ = false;
}

void periodic_timer::zero()
{
    dElapsed_ = 0.0;
}

void periodic_timer::update(double dDelta)
{
    dElapsed_ += dDelta;
}
}
