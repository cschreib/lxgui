#include "lxgui/gui_editbox.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_region_tpl.hpp"

#include <lxgui/input_window.hpp>
#include <lxgui/utils_range.hpp>

#include <sol/state.hpp>

using namespace lxgui::input;

namespace lxgui {
namespace gui
{
edit_box::edit_box(utils::control_block& mBlock, manager& mManager) : frame(mBlock, mManager),
    mCarretTimer_(dBlinkSpeed_, periodic_timer::start_type::FIRST_TICK, false)
{
    lType_.push_back(CLASS_NAME);

    iterCarretPos_ = sUnicodeText_.begin();
    iterCarretPosOld_ = sUnicodeText_.begin();

    enable_mouse(true);
    register_for_drag({"LeftButton"});
}

bool edit_box::can_use_script(const std::string& sScriptName) const
{
    if (base::can_use_script(sScriptName))
        return true;
    else if ((sScriptName == "OnCursorChanged") ||
        (sScriptName == "OnEnterPressed") ||
        (sScriptName == "OnEscapePressed") ||
        (sScriptName == "OnSpacePressed") ||
        (sScriptName == "OnTabPressed") ||
        (sScriptName == "OnUpPressed") ||
        (sScriptName == "OnDownPressed") ||
        (sScriptName == "OnTextChanged") ||
        (sScriptName == "OnTextSet"))
        return true;
    else
        return false;
}

void edit_box::copy_from(const region& mObj)
{
    base::copy_from(mObj);

    const edit_box* pEditBox = down_cast<edit_box>(&mObj);
    if (!pEditBox)
        return;

    this->set_max_letters(pEditBox->get_max_letters());
    this->set_blink_speed(pEditBox->get_blink_speed());
    this->set_numeric_only(pEditBox->is_numeric_only());
    this->set_positive_only(pEditBox->is_positive_only());
    this->set_integer_only(pEditBox->is_integer_only());
    this->enable_password_mode(pEditBox->is_password_mode_enabled());
    this->set_multi_line(pEditBox->is_multi_line());
    this->set_max_history_lines(pEditBox->get_max_history_lines());
    this->set_text_insets(pEditBox->get_text_insets());

    if (const font_string* pFS = pEditBox->get_font_string().get())
    {
        region_core_attributes mAttr;
        mAttr.sName = pFS->get_name();
        mAttr.lInheritance = {pEditBox->get_font_string()};

        auto pFont = this->create_layered_region<font_string>(
            pFS->get_draw_layer(), std::move(mAttr));

        if (pFont)
        {
            pFont->set_special();
            pFont->notify_loaded();
            this->set_font_string(pFont);
        }
    }
}

void edit_box::update(float fDelta)
{
    alive_checker mChecker(*this);

    base::update(fDelta);
    if (!mChecker.is_alive())
        return;

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

    if (iterCarretPos_ != iterCarretPosOld_)
    {
        iterCarretPosOld_ = iterCarretPos_;
        fire_script("OnCursorChanged");
        if (!mChecker.is_alive())
            return;
    }
}

void edit_box::fire_script(const std::string& sScriptName, const event_data& mData)
{
    alive_checker mChecker(*this);

    // Do not fire OnKeyUp/OnKeyDown events when typing
    bool bBypassEvent = false;
    if (has_focus() && (sScriptName == "OnKeyUp" || sScriptName == "OnKeyDown"))
        bBypassEvent = true;
    if (!has_focus() && (sScriptName == "OnChar"))
        bBypassEvent = true;

    if (!bBypassEvent)
    {
        base::fire_script(sScriptName, mData);
        if (!mChecker.is_alive())
            return;
    }

    if (sScriptName == "OnKeyDown" && has_focus())
    {
        key mKey = mData.get<key>(0);
        bool bShiftIsPressed = mData.get<bool>(2);
        bool bCtrlIsPressed = mData.get<bool>(3);

        if (mKey == key::K_RETURN || mKey == key::K_NUMPADENTER)
        {
            fire_script("OnEnterPressed");
            if (!mChecker.is_alive())
                return;
        }
        else if (mKey == key::K_TAB)
        {
            fire_script("OnTabPressed");
            if (!mChecker.is_alive())
                return;
        }
        else if (mKey == key::K_UP)
        {
            fire_script("OnUpPressed");
            if (!mChecker.is_alive())
                return;
        }
        else if (mKey == key::K_DOWN)
        {
            fire_script("OnDownPressed");
            if (!mChecker.is_alive())
                return;
        }
        else if (mKey == key::K_SPACE)
        {
            fire_script("OnSpacePressed");
            if (!mChecker.is_alive())
                return;
        }
        else if (mKey == key::K_ESCAPE)
        {
            fire_script("OnEscapePressed");
            if (!mChecker.is_alive())
                return;
        }

        process_key_(mKey, bShiftIsPressed, bCtrlIsPressed);

        if (!mChecker.is_alive())
            return;
    }
    else if (sScriptName == "OnChar" && has_focus())
    {
        std::uint32_t c = mData.get<std::uint32_t>(1);
        if (add_char_(c))
        {
            fire_script("OnTextChanged");
            if (!mChecker.is_alive())
                return;
        }
    }
    else if (sScriptName == "OnSizeChanged")
    {
        update_displayed_text_();
        update_font_string_();
        update_carret_position_();
    }
    else if (sScriptName == "OnDragStart")
    {
        uiSelectionEndPos_ = uiSelectionStartPos_ = get_letter_id_at_(
            vector2f(mData.get<float>(1), mData.get<float>(2)));
    }
    else if (sScriptName == "OnDragMove")
    {
        std::size_t uiPos = get_letter_id_at_(vector2f(mData.get<float>(0), mData.get<float>(1)));
        if (uiPos != uiSelectionEndPos_)
        {
            if (uiPos != std::numeric_limits<std::size_t>::max())
            {
                highlight_text(uiSelectionStartPos_, uiPos);
                iterCarretPos_ = sUnicodeText_.begin() + uiPos;
                update_carret_position_();
            }
            else
            {
                std::size_t uiTemp = uiSelectionStartPos_;
                unlight_text();
                uiSelectionStartPos_ = uiTemp;
                iterCarretPos_ = sUnicodeText_.begin() + uiSelectionStartPos_;
                update_carret_position_();
            }
        }
    }
    else if (sScriptName == "OnMouseDown")
    {
        set_focus(true);
        if (!mChecker.is_alive())
            return;

        unlight_text();

        move_carret_at_({mData.get<float>(1), mData.get<float>(2)});
    }
}

void edit_box::create_glue()
{
    create_glue_(this);
}

void edit_box::set_text(const utils::ustring& sText)
{
    if (sText != sUnicodeText_)
    {
        unlight_text();
        sUnicodeText_ = sText;
        check_text_();
        update_displayed_text_();
        iterCarretPos_ = sUnicodeText_.end();
        update_font_string_();
        update_carret_position_();

        alive_checker mChecker(*this);

        fire_script("OnTextSet");
        if (!mChecker.is_alive())
            return;

        fire_script("OnTextChanged");
        if (!mChecker.is_alive())
            return;
    }
}

const utils::ustring& edit_box::get_text() const
{
    return sUnicodeText_;
}

void edit_box::unlight_text()
{
    uiSelectionStartPos_ = uiSelectionEndPos_ = 0u;
    bSelectedText_ = false;

    if (pHighlight_)
        pHighlight_->hide();
}

void edit_box::highlight_text(std::size_t uiStart, std::size_t uiEnd, bool bForceUpdate)
{
    if (!pHighlight_)
        create_highlight_();

    if (!pHighlight_)
        return;

    std::size_t uiLeft  = std::min(uiStart, uiEnd);
    std::size_t uiRight = std::max(uiStart, uiEnd);

    if (uiSelectionStartPos_ != uiStart || uiSelectionEndPos_ != uiEnd || bForceUpdate)
    {
        if (uiLeft != uiRight)
        {
            bSelectedText_ = true;

            if (uiRight >= uiDisplayPos_ && uiLeft < uiDisplayPos_ + sDisplayedText_.size() &&
                pFontString_ && pFontString_->get_text_object())
            {
                text* pText = pFontString_->get_text_object();

                if (uiLeft < uiDisplayPos_)
                    uiLeft = 0;
                else
                    uiLeft = uiLeft - uiDisplayPos_;

                float fLeftPos = lTextInsets_.left;
                if (uiLeft < pText->get_num_letters())
                    fLeftPos += pText->get_letter_quad(uiLeft)[0].pos.x;

                uiRight = uiRight - uiDisplayPos_;
                float fRightPos = lTextInsets_.left;
                if (uiRight < sDisplayedText_.size())
                {
                    if (uiRight < pText->get_num_letters())
                        fRightPos += pText->get_letter_quad(uiRight)[0].pos.x;
                }
                else
                {
                    uiRight = sDisplayedText_.size() - 1;
                    if (uiRight < pText->get_num_letters())
                        fRightPos += pText->get_letter_quad(uiRight)[2].pos.x;
                }

                pHighlight_->set_point(anchor_point::LEFT,  sName_, vector2f(fLeftPos,  0));
                pHighlight_->set_point(anchor_point::RIGHT, sName_,
                    anchor_point::LEFT, vector2f(fRightPos, 0));

                pHighlight_->show();
            }
            else
                pHighlight_->hide();
        }
        else
        {
            bSelectedText_ = false;
            pHighlight_->hide();
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

        pHighlight_->set_solid_color(mHighlightColor_);
    }
}

void edit_box::insert_after_cursor(const utils::ustring& sText)
{
    if (!sText.empty())
    {
        if (bNumericOnly_ && !utils::is_number(sText))
            return;

        if (sUnicodeText_.size() + sText.size() <= uiMaxLetters_)
        {
            unlight_text();
            sUnicodeText_.insert(iterCarretPos_, sText.begin(), sText.end());
            iterCarretPos_ += sText.size();

            update_displayed_text_();
            update_font_string_();
            update_carret_position_();
        }
    }
}

std::size_t edit_box::get_cursor_position() const
{
    return iterCarretPos_ - sUnicodeText_.begin();
}

void edit_box::set_cursor_position(std::size_t uiPos)
{
    if (uiPos == get_cursor_position()) return;

    iterCarretPos_ = sUnicodeText_.begin() + uiPos;
    update_carret_position_();
}

void edit_box::set_max_letters(std::size_t uiMaxLetters)
{
    if (uiMaxLetters == 0)
    {
        uiMaxLetters_ = std::numeric_limits<std::size_t>::max();
        return;
    }

    if (uiMaxLetters_ != uiMaxLetters)
    {
        uiMaxLetters_ = uiMaxLetters;

        std::size_t uiCarretPos = iterCarretPos_ - sUnicodeText_.begin();

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

std::size_t edit_box::get_max_letters() const
{
    return uiMaxLetters_;
}

std::size_t edit_box::get_num_letters() const
{
    return sUnicodeText_.size();
}

void edit_box::set_blink_speed(double dBlinkSpeed)
{
    if (dBlinkSpeed_ != dBlinkSpeed)
    {
        dBlinkSpeed_ = dBlinkSpeed;
        mCarretTimer_ = periodic_timer(dBlinkSpeed_, periodic_timer::start_type::FIRST_TICK, false);
    }
}

double edit_box::get_blink_speed() const
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

        if (bNumericOnly_ && bIntegerOnly_)
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
        clear_history();
    }
}

bool edit_box::is_multi_line() const
{
    return bMultiLine_;
}

void edit_box::set_max_history_lines(std::size_t uiMaxHistoryLines)
{
    if (uiMaxHistoryLines == 0)
    {
        uiMaxHistoryLines_ = std::numeric_limits<std::size_t>::max();
        return;
    }

    if (uiMaxHistoryLines_ != uiMaxHistoryLines)
    {
        uiMaxHistoryLines_ = uiMaxHistoryLines;

        if (lHistoryLineList_.size() > uiMaxHistoryLines_)
        {
            lHistoryLineList_.erase(
                lHistoryLineList_.begin(),
                lHistoryLineList_.begin() + (lHistoryLineList_.size() - uiMaxHistoryLines_)
            );

            uiCurrentHistoryLine_ = std::numeric_limits<std::size_t>::max();
        }
    }
}

std::size_t edit_box::get_max_history_lines() const
{
    return uiMaxHistoryLines_;
}

void edit_box::add_history_line(const utils::ustring& sHistoryLine)
{
    if (bMultiLine_) return;

    lHistoryLineList_.push_back(sHistoryLine);

    if (lHistoryLineList_.size() > uiMaxHistoryLines_)
    {
        lHistoryLineList_.erase(
            lHistoryLineList_.begin(),
            lHistoryLineList_.begin() + (lHistoryLineList_.size() - uiMaxHistoryLines_)
        );
    }

    uiCurrentHistoryLine_ = std::numeric_limits<std::size_t>::max();
}

const std::vector<utils::ustring>& edit_box::get_history_lines() const
{
    return lHistoryLineList_;
}

void edit_box::clear_history()
{
    lHistoryLineList_.clear();
    uiCurrentHistoryLine_ = std::numeric_limits<std::size_t>::max();
}

void edit_box::set_arrows_ignored(bool bArrowsIgnored)
{
    bArrowsIgnored_ = bArrowsIgnored;
}

void edit_box::set_text_insets(const bounds2f& lInsets)
{
    lTextInsets_ = lInsets;

    if (pFontString_)
    {
        pFontString_->clear_all_points();
        pFontString_->set_point(anchor_point::TOP_LEFT, lTextInsets_.top_left());
        pFontString_->set_point(anchor_point::BOTTOM_RIGHT, -lTextInsets_.bottom_right());

        update_displayed_text_();
        update_font_string_();
        update_carret_position_();
    }
}

const bounds2f& edit_box::get_text_insets() const
{
    return lTextInsets_;
}

void edit_box::notify_focus(bool bFocus)
{
    if (bFocus_ != bFocus)
    {
        if (bFocus)
        {
            if (!pCarret_)
                create_carret_();

            if (pCarret_)
                pCarret_->show();

            mCarretTimer_.zero();
        }
        else
        {
            if (pCarret_)
                pCarret_->hide();

            unlight_text();
        }
    }

    base::notify_focus(bFocus);
}

void edit_box::notify_scaling_factor_updated()
{
    base::notify_scaling_factor_updated();

    if (pFontString_)
    {
        pFontString_->notify_scaling_factor_updated();
        create_carret_();
    }
}

void edit_box::set_font_string(utils::observer_ptr<font_string> pFont)
{
    pFontString_ = std::move(pFont);
    if (!pFontString_)
        return;

    pFontString_->set_word_wrap(bMultiLine_, bMultiLine_);

    pFontString_->set_dimensions(vector2f(0, 0));
    pFontString_->clear_all_points();

    pFontString_->set_point(anchor_point::TOP_LEFT, lTextInsets_.top_left());
    pFontString_->set_point(anchor_point::BOTTOM_RIGHT, -lTextInsets_.bottom_right());

    pFontString_->enable_formatting(false);

    create_carret_();
}

void edit_box::set_font(const std::string& sFontName, float fHeight)
{
    create_font_string_();

    pFontString_->set_font(sFontName, fHeight);

    create_carret_();
}

void edit_box::create_font_string_()
{
    if (pFontString_)
        return;

    auto pFont = create_layered_region<font_string>(layer::ARTWORK, "$parentFontString");
    if (!pFont)
        return;

    pFont->set_special();
    pFont->notify_loaded();
    set_font_string(pFont);
}

void edit_box::create_highlight_()
{
    if (pHighlight_ || is_virtual())
        return;

    auto pHighlight = create_layered_region<texture>(layer::HIGHLIGHT, "$parentHighlight");
    if (!pHighlight)
        return;

    pHighlight->set_special();

    pHighlight->set_point(anchor_point::TOP, vector2f(0.0f, lTextInsets_.top));
    pHighlight->set_point(anchor_point::BOTTOM, vector2f(0.0f, -lTextInsets_.bottom));

    pHighlight->set_solid_color(mHighlightColor_);

    pHighlight->notify_loaded();
    pHighlight_ = pHighlight;
}

void edit_box::create_carret_()
{
    if (!pFontString_ || !pFontString_->get_text_object() || is_virtual())
        return;

    if (!pCarret_)
    {
        auto pCarret = create_layered_region<texture>(layer::HIGHLIGHT, "$parentCarret");
        if (!pCarret)
            return;

        pCarret->set_special();

        pCarret->set_point(anchor_point::CENTER, anchor_point::LEFT,
            vector2f(lTextInsets_.left - 1.0f, 0.0f));

        pCarret->notify_loaded();
        pCarret_ = pCarret;
    }

    quad mQuad = pFontString_->get_text_object()->create_letter_quad(U'|');
    for (std::size_t i = 0; i < 4; ++i)
        mQuad.v[i].col = pFontString_->get_text_color();

    pCarret_->set_quad(mQuad);

    update_carret_position_();
}

void edit_box::check_text_()
{
    if (sUnicodeText_.size() > uiMaxLetters_)
        sUnicodeText_.resize(uiMaxLetters_);

    // TODO: use localizer's locale for these checks
    // https://github.com/cschreib/lxgui/issues/88
    if (bNumericOnly_ && !utils::is_number(sUnicodeText_))
    {
        sUnicodeText_.clear();
        return;
    }

    if (bIntegerOnly_ && !utils::is_integer(sUnicodeText_))
    {
        sUnicodeText_.clear();
        return;
    }

    if (bPositiveOnly_)
    {
        double dValue = 0.0;
        if (!utils::from_string(sUnicodeText_, dValue) || dValue < 0.0)
        {
            sUnicodeText_.clear();
            return;
        }
    }
}

void edit_box::update_displayed_text_()
{
    if (pFontString_ && pFontString_->get_text_object())
    {
        if (bPasswordMode_)
            sDisplayedText_ = utils::ustring(sUnicodeText_.size(), U'*');
        else
            sDisplayedText_ = sUnicodeText_;

        if (!bMultiLine_)
        {
            text* pTextObject = pFontString_->get_text_object();

            if (!std::isinf(pTextObject->get_box_width()))
            {
                sDisplayedText_.erase(0, uiDisplayPos_);

                while (!sDisplayedText_.empty() &&
                    pTextObject->get_string_width(sDisplayedText_) > pTextObject->get_box_width())
                {
                    sDisplayedText_.erase(sDisplayedText_.size()-1, 1);
                }
            }
        }
        else
        {
            // TODO: implement for multiline edit box
            // https://github.com/cschreib/lxgui/issues/39
        }
    }
}

void edit_box::update_font_string_()
{
    if (!pFontString_)
        return;

    pFontString_->set_text(sDisplayedText_);

    if (bSelectedText_)
        highlight_text(uiSelectionStartPos_, uiSelectionEndPos_, true);
}

void edit_box::update_carret_position_()
{
    if (!pFontString_ || !pFontString_->get_text_object() || !pCarret_)
        return;

    if (sUnicodeText_.empty())
    {
        anchor_point mPoint;
        float fOffset = 0.0f;
        switch (pFontString_->get_alignment_x())
        {
            case alignment_x::LEFT :
                mPoint = anchor_point::LEFT;
                fOffset = lTextInsets_.left - 1;
                break;
            case alignment_x::CENTER :
                mPoint = anchor_point::CENTER;
                break;
            case alignment_x::RIGHT :
                mPoint = anchor_point::RIGHT;
                fOffset = -lTextInsets_.right - 1;
                break;
            default : mPoint = anchor_point::LEFT; break;
        }

        pCarret_->set_point(anchor_point::CENTER, mPoint, vector2f(fOffset, 0.0f));
    }
    else
    {
        text* pText = pFontString_->get_text_object();
        utils::ustring::iterator iterDisplayCarret;

        if (!bMultiLine_)
        {
            std::size_t uiGlobalPos = iterCarretPos_ - sUnicodeText_.begin();

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

            std::size_t uiCarretPos = uiGlobalPos - uiDisplayPos_;
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
        {
            iterDisplayCarret = sDisplayedText_.begin() +
                (iterCarretPos_ - sUnicodeText_.begin()) - uiDisplayPos_;
        }

        float fYOffset = static_cast<float>((pText->get_num_lines() - 1)) *
            (pText->get_line_height() * pText->get_line_spacing());

        std::size_t uiIndex = iterDisplayCarret - sDisplayedText_.begin();

        float fXOffset = lTextInsets_.left;
        if (uiIndex < sDisplayedText_.size())
        {
            if (uiIndex < pText->get_num_letters())
                fXOffset += pText->get_letter_quad(uiIndex)[0].pos.x;
        }
        else
        {
            uiIndex = sDisplayedText_.size() - 1;
            if (uiIndex < pText->get_num_letters())
                fXOffset += pText->get_letter_quad(uiIndex)[2].pos.x;
        }

        pCarret_->set_point(anchor_point::CENTER, anchor_point::LEFT, vector2f(fXOffset, fYOffset));
    }

    mCarretTimer_.zero();
    if (bFocus_)
        pCarret_->show();
    else
        pCarret_->hide();
}

bool edit_box::add_char_(char32_t sUnicode)
{
    if (bSelectedText_)
        remove_char_();

    if (get_num_letters() >= uiMaxLetters_)
        return false;

    // TODO: use localizer for these checks, if possible
    // https://github.com/cschreib/lxgui/issues/88
    if (bNumericOnly_)
    {
        if (sUnicode == U'.')
        {
            if (bIntegerOnly_)
                return false;

            if (sUnicodeText_.find(U'.') != utils::ustring::npos)
                return false;
        }
        else if (sUnicode == U'+' || sUnicode == U'-')
        {
            if (bPositiveOnly_)
                return false;

            if (iterCarretPos_ != sUnicodeText_.begin() ||
                sUnicodeText_.find(U'+') != utils::ustring::npos ||
                sUnicodeText_.find(U'-') != utils::ustring::npos)
                return false;
        }
        else if (!utils::is_number(sUnicode))
            return false;
    }

    iterCarretPos_ = sUnicodeText_.insert(iterCarretPos_, sUnicode) + 1;

    update_displayed_text_();
    update_font_string_();
    update_carret_position_();

    if (pCarret_)
        pCarret_->show();

    mCarretTimer_.zero();

    return true;
}

bool edit_box::remove_char_()
{
    if (bSelectedText_)
    {
        if (uiSelectionStartPos_ != uiSelectionEndPos_)
        {
            std::size_t uiLeft = std::min(uiSelectionStartPos_, uiSelectionEndPos_);
            std::size_t uiRight = std::max(uiSelectionStartPos_, uiSelectionEndPos_);

            sUnicodeText_.erase(uiLeft, uiRight - uiLeft);

            iterCarretPos_ = sUnicodeText_.begin() + uiLeft;
        }

        unlight_text();
    }
    else
    {
        if (iterCarretPos_ == sUnicodeText_.end())
            return false;

        iterCarretPos_ = sUnicodeText_.erase(iterCarretPos_);
    }

    update_displayed_text_();
    update_font_string_();
    update_carret_position_();

    if (pCarret_)
        pCarret_->show();

    mCarretTimer_.zero();

    return true;
}

std::size_t edit_box::get_letter_id_at_(const vector2f& mPosition)
{
    if (pFontString_ && pFontString_->get_text_object())
    {
        if (sDisplayedText_.empty())
            return uiDisplayPos_;

        text* pText = pFontString_->get_text_object();

        float fLocalX = mPosition.x - lBorderList_.left - lTextInsets_.left;
        // float fLocalY = mPosition.y - lBorderList_.top  - lTextInsets_.top;

        if (!bMultiLine_)
        {
            if (mPosition.x < lBorderList_.left + lTextInsets_.left)
                return uiDisplayPos_;
            else if (mPosition.x > lBorderList_.right - lTextInsets_.right)
                return sDisplayedText_.size() + uiDisplayPos_;

            std::size_t uiNumLetters = std::min<std::size_t>(
                pText->get_num_letters(), sDisplayedText_.size());

            for (std::size_t uiIndex = 0u; uiIndex < uiNumLetters; ++uiIndex)
            {
                const auto& mQuad = pText->get_letter_quad(uiIndex);
                if (fLocalX < 0.5f*(mQuad[0].pos.x + mQuad[2].pos.x))
                    return uiIndex + uiDisplayPos_;
            }

            return sDisplayedText_.size() + uiDisplayPos_;
        }
        else
        {
            // TODO: Implement for multi line edit_box
            // https://github.com/cschreib/lxgui/issues/39
            return uiDisplayPos_;
        }

    }

    return std::numeric_limits<std::size_t>::max();
}

bool edit_box::move_carret_at_(const vector2f& mPosition)
{
    std::size_t uiPos = get_letter_id_at_(mPosition);
    if (uiPos != std::numeric_limits<std::size_t>::max())
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
            update_displayed_text_();
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
            update_displayed_text_();
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
        // TODO: Implement for multi line edit_box
        // https://github.com/cschreib/lxgui/issues/39
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
            update_displayed_text_();
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

void edit_box::process_key_(key mKey, bool bShiftIsPressed, bool bCtrlIsPressed)
{
    alive_checker mChecker(*this);

    if (mKey == key::K_RETURN || mKey == key::K_NUMPADENTER)
    {
        if (bMultiLine_)
        {
            if (add_char_(U'\n'))
            {
                event_data mKeyEvent;
                mKeyEvent.add(std::string("\n"));
                fire_script("OnChar", mKeyEvent);
                if (!mChecker.is_alive())
                    return;

                fire_script("OnTextChanged");
                if (!mChecker.is_alive())
                    return;
            }
        }
    }
    else if (mKey == key::K_END)
    {
        std::size_t uiPreviousCarretPos = get_cursor_position();
        set_cursor_position(get_num_letters());

        if (bShiftIsPressed)
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
    else if (mKey == key::K_HOME)
    {
        std::size_t uiPreviousCarretPos = get_cursor_position();
        set_cursor_position(0u);

        if (bShiftIsPressed)
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
    else if (mKey == key::K_BACK || mKey == key::K_DELETE)
    {
        if (bSelectedText_ || mKey == key::K_DELETE || move_carret_horizontally_(false))
        {
            remove_char_();
            fire_script("OnTextChanged");
            if (!mChecker.is_alive())
                return;
        }
    }
    else if (mKey == key::K_LEFT || mKey == key::K_RIGHT ||
            (bMultiLine_ && (mKey == key::K_UP || mKey == key::K_DOWN)))
    {
        if (!bArrowsIgnored_)
        {
            std::size_t uiPreviousCarretPos = iterCarretPos_ - sUnicodeText_.begin();

            if (mKey == key::K_LEFT || mKey == key::K_RIGHT)
            {
                if (bSelectedText_ && !bShiftIsPressed)
                {
                    std::size_t uiOffset = 0;
                    if (mKey == key::K_LEFT)
                        uiOffset = std::min(uiSelectionStartPos_, uiSelectionEndPos_);
                    else
                        uiOffset = std::max(uiSelectionStartPos_, uiSelectionEndPos_);

                    iterCarretPos_ = sUnicodeText_.begin() + uiOffset;
                    update_carret_position_();
                }
                else
                    move_carret_horizontally_(mKey == key::K_RIGHT);
            }
            else
            {
                if (bMultiLine_)
                    move_carret_vertically_(mKey == key::K_DOWN);
            }

            if (bShiftIsPressed)
            {
                if (bSelectedText_)
                {
                    std::size_t uiNewEndPos = iterCarretPos_ - sUnicodeText_.begin();
                    if (uiNewEndPos != uiSelectionStartPos_)
                        highlight_text(uiSelectionStartPos_, uiNewEndPos);
                    else
                        unlight_text();
                }
                else
                    highlight_text(uiPreviousCarretPos, iterCarretPos_ - sUnicodeText_.begin());
            }
            else
                unlight_text();
        }
    }
    else if (!bMultiLine_ && (mKey == key::K_UP || mKey == key::K_DOWN) && !lHistoryLineList_.empty())
    {
        if (mKey == key::K_UP)
        {
            if (uiCurrentHistoryLine_ != 0u)
            {
                if (uiCurrentHistoryLine_ == std::numeric_limits<std::size_t>::max())
                    uiCurrentHistoryLine_ = lHistoryLineList_.size()-1;
                else
                    --uiCurrentHistoryLine_;

                set_text(lHistoryLineList_[uiCurrentHistoryLine_]);
                if (!mChecker.is_alive())
                    return;
            }
        }
        else
        {
            if (uiCurrentHistoryLine_ != std::numeric_limits<std::size_t>::max())
            {
                if (uiCurrentHistoryLine_ + 1 == lHistoryLineList_.size())
                {
                    uiCurrentHistoryLine_ = std::numeric_limits<std::size_t>::max();
                    set_text(U"");
                    if (!mChecker.is_alive())
                        return;
                }
                else
                {
                    ++uiCurrentHistoryLine_;
                    set_text(lHistoryLineList_[uiCurrentHistoryLine_]);
                    if (!mChecker.is_alive())
                        return;
                }
            }
        }
    }
    else if (mKey == key::K_C && bCtrlIsPressed)
    {
        if (uiSelectionEndPos_ != uiSelectionStartPos_)
        {
            std::size_t uiMinPos = std::min(uiSelectionStartPos_, uiSelectionEndPos_);
            std::size_t uiMaxPos = std::max(uiSelectionStartPos_, uiSelectionEndPos_);
            utils::ustring sSelected = sUnicodeText_.substr(uiMinPos, uiMaxPos - uiMinPos);
            get_manager().get_window().set_clipboard_content(sSelected);
        }
    }
    else if (mKey == key::K_V && bCtrlIsPressed)
    {
        for (char32_t cChar : get_manager().get_window().get_clipboard_content())
        {
            if (!add_char_(cChar))
                break;
            if (!mChecker.is_alive())
                return;
        }
    }
}

periodic_timer::periodic_timer(double dDuration, start_type mType, bool bTickFirst) :
    dElapsed_(bTickFirst ? dDuration : 0.0), dDuration_(dDuration), mType_(mType)
{
    if (mType == start_type::NOW)
        start();
}

double periodic_timer::get_elapsed() const
{
    return dElapsed_;
}

double periodic_timer::get_period() const
{
    return dDuration_;
}

bool periodic_timer::is_paused() const
{
    return bPaused_;
}

bool periodic_timer::ticks()
{
    if (mType_ == start_type::FIRST_TICK && bFirstTick_)
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
}
