#include "lxgui/gui_fontstring.hpp"

#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"
#include "lxgui/gui_localizer.hpp"

#include <sstream>

namespace lxgui {
namespace gui
{

font_string::font_string(manager* pManager) : layered_region(pManager)
{
    lType_.push_back(CLASS_NAME);
}

void font_string::render()
{
    update_borders_();

    if (!pText_ || !bReady_ || !is_visible())
        return;

    float fX = 0.0f, fY = 0.0f;

    if (std::isinf(pText_->get_box_width()))
    {
        switch (mJustifyH_)
        {
            case text::alignment::LEFT   : fX = lBorderList_.left; break;
            case text::alignment::CENTER : fX = (lBorderList_.left + lBorderList_.right)/2; break;
            case text::alignment::RIGHT  : fX = lBorderList_.right; break;
        }
    }
    else
        fX = lBorderList_.left;

    if (std::isinf(pText_->get_box_height()))
    {
        switch (mJustifyV_)
        {
            case text::vertical_alignment::TOP    : fY = lBorderList_.top; break;
            case text::vertical_alignment::MIDDLE : fY = (lBorderList_.top + lBorderList_.bottom)/2; break;
            case text::vertical_alignment::BOTTOM : fY = lBorderList_.bottom; break;
        }
    }
    else
        fY = lBorderList_.top;

    fX += fXOffset_;
    fY += fYOffset_;

    pText_->set_alpha(get_effective_alpha());

    if (bHasShadow_)
    {
        pText_->set_color(mShadowColor_, true);
        pText_->render(fX + fShadowXOffset_, fY + fShadowYOffset_);
    }

    pText_->set_color(mTextColor_);
    pText_->render(fX, fY);
}

std::string font_string::serialize(const std::string& sTab) const
{
    std::ostringstream sStr;

    sStr << layered_region::serialize(sTab);

    sStr << sTab << "  # Font name   : " << sFontName_ << "\n";
    sStr << sTab << "  # Font height : " << fHeight_ << "\n";
    sStr << sTab << "  # Text ready  : " << (pText_ != nullptr) << "\n";
    sStr << sTab << "  # Text        : \"" << utils::unicode_to_utf8(sText_) << "\"\n";
    sStr << sTab << "  # Outlined    : " << bIsOutlined_ << "\n";
    sStr << sTab << "  # Text color  : " << mTextColor_ << "\n";
    sStr << sTab << "  # Spacing     : " << fSpacing_ << "\n";
    sStr << sTab << "  # Justify     :\n";
    sStr << sTab << "  #-###\n";
    sStr << sTab << "  |   # horizontal : ";
    switch (mJustifyH_)
    {
        case text::alignment::LEFT :   sStr << "LEFT\n"; break;
        case text::alignment::CENTER : sStr << "CENTER\n"; break;
        case text::alignment::RIGHT :  sStr << "RIGHT\n"; break;
        default : sStr << "<error>\n"; break;
    }
    sStr << sTab << "  |   # vertical   : ";
    switch (mJustifyV_)
    {
        case text::vertical_alignment::TOP :    sStr << "TOP\n"; break;
        case text::vertical_alignment::MIDDLE : sStr << "MIDDLE\n"; break;
        case text::vertical_alignment::BOTTOM : sStr << "BOTTOM\n"; break;
        default : sStr << "<error>\n"; break;
    }
    sStr << sTab << "  #-###\n";
    sStr << sTab << "  # NonSpaceW.  : " << bCanNonSpaceWrap_ << "\n";
    if (bHasShadow_)
    {
    sStr << sTab << "  # Shadow off. : (" << fShadowXOffset_ << ", " << fShadowYOffset_ << ")\n";
    sStr << sTab << "  # Shadow col. : " <<  mShadowColor_ << "\n";
    }

    return sStr.str();
}

void font_string::create_glue()
{
    create_glue_<lua_font_string>();
}

void font_string::copy_from(uiobject* pObj)
{
    uiobject::copy_from(pObj);

    font_string* pFontString = down_cast<font_string>(pObj);
    if (!pFontString)
        return;

    std::string sFontName = pFontString->get_font_name();
    float fHeight = pFontString->get_font_height();
    if (!sFontName.empty() && fHeight != 0)
        this->set_font(sFontName, fHeight);

    this->set_justify_h(pFontString->get_justify_h());
    this->set_justify_v(pFontString->get_justify_v());
    this->set_spacing(pFontString->get_spacing());
    this->set_text(pFontString->get_text());
    this->set_outlined(pFontString->is_outlined());
    if (pFontString->has_shadow())
    {
        this->set_shadow(true);
        this->set_shadow_color(pFontString->get_shadow_color());
        this->set_shadow_offsets(pFontString->get_shadow_offsets());
    }
    this->set_text_color(pFontString->get_text_color());
    this->set_non_space_wrap(pFontString->can_non_space_wrap());
}

const std::string& font_string::get_font_name() const
{
    return sFontName_;
}

float font_string::get_font_height() const
{
    return fHeight_;
}

void font_string::set_outlined(bool bIsOutlined)
{
    if (bIsOutlined_ != bIsOutlined)
    {
        bIsOutlined_ = bIsOutlined;

        create_text_object_();

        notify_renderer_need_redraw();
    }
}

bool font_string::is_outlined() const
{
    return bIsOutlined_;
}

text::alignment font_string::get_justify_h() const
{
    return mJustifyH_;
}

text::vertical_alignment font_string::get_justify_v() const
{
    return mJustifyV_;
}

const color& font_string::get_shadow_color() const
{
    return mShadowColor_;
}

vector2f font_string::get_shadow_offsets() const
{
    return vector2f(fShadowXOffset_, fShadowYOffset_);
}

vector2f font_string::get_offsets() const
{
    return vector2f(fXOffset_, fYOffset_);
}

float font_string::get_shadow_x_offset() const
{
    return fShadowXOffset_;
}

float font_string::get_shadow_y_offset() const
{
    return fShadowYOffset_;
}

float font_string::get_spacing() const
{
    return fSpacing_;
}

const color& font_string::get_text_color() const
{
    return mTextColor_;
}

void font_string::notify_scaling_factor_updated()
{
    uiobject::notify_scaling_factor_updated();

    if (pText_)
        set_font(sFontName_, fHeight_);
}

void font_string::create_text_object_()
{
    if (sFontName_.empty()) return;

    uint uiPixelHeight = std::round(pManager_->get_interface_scaling_factor()*fHeight_);

    renderer* pRenderer = pManager_->get_renderer();
    const localizer& mLocalizer = pManager_->get_localizer();

    const auto& lCodePoints = mLocalizer.get_allowed_code_points();
    const char32_t uiDefaultCodePoint = mLocalizer.get_fallback_code_point();

    std::shared_ptr<gui::font> pOutlineFont;
    if (bIsOutlined_)
    {
        pOutlineFont = pRenderer->create_atlas_font(
            "GUI", sFontName_, uiPixelHeight,
            std::min(2u, static_cast<uint>(std::round(0.2*uiPixelHeight))),
            lCodePoints, uiDefaultCodePoint);
    }

    auto pFont = pRenderer->create_atlas_font(
        "GUI", sFontName_, uiPixelHeight,
        0u, lCodePoints, uiDefaultCodePoint);

    pText_ = std::unique_ptr<text>(new text(pRenderer, pFont, pOutlineFont));

    pText_->set_scaling_factor(1.0f/pManager_->get_interface_scaling_factor());
    pText_->set_remove_starting_spaces(true);
    pText_->set_text(sText_);
    pText_->set_alignment(mJustifyH_);
    pText_->set_vertical_alignment(mJustifyV_);
    pText_->set_tracking(fSpacing_);
    pText_->enable_word_wrap(bCanWordWrap_, bAddEllipsis_);
    pText_->enable_formatting(bFormattingEnabled_);
}

void font_string::set_font(const std::string& sFontName, float fHeight)
{
    sFontName_ = sFontName;
    fHeight_ = fHeight;

    create_text_object_();

    if (!bVirtual_)
    {
        notify_borders_need_update();
        notify_renderer_need_redraw();
    }
}

void font_string::set_justify_h(text::alignment mJustifyH)
{
    if (mJustifyH_ != mJustifyH)
    {
        mJustifyH_ = mJustifyH;
        if (pText_)
        {
            pText_->set_alignment(mJustifyH_);

            if (!bVirtual_)
                notify_renderer_need_redraw();
        }
    }
}

void font_string::set_justify_v(text::vertical_alignment mJustifyV)
{
    if (mJustifyV_ != mJustifyV)
    {
        mJustifyV_ = mJustifyV;
        if (pText_)
        {
            pText_->set_vertical_alignment(mJustifyV_);

            if (!bVirtual_)
                notify_renderer_need_redraw();
        }
    }
}

void font_string::set_shadow_color(const color& mShadowColor)
{
    if (mShadowColor_ != mShadowColor)
    {
        mShadowColor_ = mShadowColor;
        if (bHasShadow_ && !bVirtual_)
            notify_renderer_need_redraw();
    }
}

void font_string::set_shadow_offsets(float fShadowXOffset, float fShadowYOffset)
{
    if (fShadowXOffset_ != fShadowXOffset || fShadowYOffset_ != fShadowYOffset)
    {
        fShadowXOffset_ = fShadowXOffset;
        fShadowYOffset_ = fShadowYOffset;
        if (bHasShadow_ && !bVirtual_)
            notify_renderer_need_redraw();
    }
}

void font_string::set_shadow_offsets(const vector2f& mShadowOffsets)
{
    if (fShadowXOffset_ != mShadowOffsets.x || fShadowYOffset_ != mShadowOffsets.y)
    {
        fShadowXOffset_ = mShadowOffsets.x;
        fShadowYOffset_ = mShadowOffsets.y;
        if (bHasShadow_ && !bVirtual_)
            notify_renderer_need_redraw();
    }
}

void font_string::set_offsets(float fXOffset, float fYOffset)
{
    if (fXOffset_ != fXOffset || fYOffset_ != fYOffset)
    {
        fXOffset_ = fXOffset;
        fYOffset_ = fYOffset;
        if (!bVirtual_)
            notify_renderer_need_redraw();
    }
}

void font_string::set_offsets(const vector2f& mOffsets)
{
    if (fXOffset_ != mOffsets.x || fYOffset_ != mOffsets.y)
    {
        fXOffset_ = mOffsets.x;
        fYOffset_ = mOffsets.y;
        if (!bVirtual_)
            notify_renderer_need_redraw();
    }
}

void font_string::set_spacing(float fSpacing)
{
    if (fSpacing_ != fSpacing)
    {
        fSpacing_ = fSpacing;
        if (pText_)
        {
            pText_->set_tracking(fSpacing_);
            if (!bVirtual_)
                notify_renderer_need_redraw();
        }
    }
}

void font_string::set_text_color(const color& mTextColor)
{
    if (mTextColor_ != mTextColor)
    {
        mTextColor_ = mTextColor;
        if (!bVirtual_)
            notify_renderer_need_redraw();
    }
}

bool font_string::can_non_space_wrap() const
{
    return bCanNonSpaceWrap_;
}

float font_string::get_string_height() const
{
    if (pText_)
        return pText_->get_text_height();
    else
        return 0.0f;
}

float font_string::get_string_width() const
{
    if (pText_)
        return pText_->get_text_width();
    else
        return 0.0f;
}

float font_string::get_string_width(const utils::ustring& sString) const
{
    if (pText_)
        return pText_->get_string_width(sString);
    else
        return 0.0f;
}

const utils::ustring& font_string::get_text() const
{
    return sText_;
}

void font_string::set_non_space_wrap(bool bCanNonSpaceWrap)
{
    if (bCanNonSpaceWrap_ != bCanNonSpaceWrap)
    {
        bCanNonSpaceWrap_ = bCanNonSpaceWrap;
        if (!bVirtual_)
            notify_renderer_need_redraw();
    }
}

bool font_string::has_shadow() const
{
    return bHasShadow_;
}

void font_string::set_shadow(bool bHasShadow)
{
    if (bHasShadow_ != bHasShadow)
    {
        bHasShadow_ = bHasShadow;
        if (!bVirtual_)
            notify_renderer_need_redraw();
    }
}

void font_string::set_word_wrap(bool bCanWordWrap, bool bAddEllipsis)
{
    bCanWordWrap_ = bCanWordWrap;
    bAddEllipsis_ = bAddEllipsis;
    if (pText_)
        pText_->enable_word_wrap(bCanWordWrap_, bAddEllipsis_);
}

bool font_string::can_word_wrap() const
{
    return bCanWordWrap_;
}

void font_string::enable_formatting(bool bFormatting)
{
    bFormattingEnabled_ = bFormatting;
    if (pText_)
        pText_->enable_formatting(bFormattingEnabled_);
}

bool font_string::is_formatting_enabled() const
{
    return bFormattingEnabled_;
}

void font_string::set_text(const utils::ustring& sText)
{
    if (sText_ != sText)
    {
        sText_ = sText;
        if (pText_)
        {
            pText_->set_text(sText_);
            if (!bVirtual_)
                notify_borders_need_update();
        }
    }
}

text* font_string::get_text_object()
{
    return pText_.get();
}

const text* font_string::get_text_object() const
{
    return pText_.get();
}

void font_string::update_borders_() const
{
    if (!pText_)
        return uiobject::update_borders_();

    if (!bUpdateBorders_)
        return;

    //#define DEBUG_LOG(msg) gui::out << (msg) << std::endl
    #define DEBUG_LOG(msg)

    bool bOldReady = bReady_;
    bReady_ = true;

    if (!lAnchorList_.empty())
    {
        float fLeft = 0.0f, fRight = 0.0f, fTop = 0.0f, fBottom = 0.0f;
        float fXCenter = 0.0f, fYCenter = 0.0f;

        DEBUG_LOG("  Read anchors");
        read_anchors_(fLeft, fRight, fTop, fBottom, fXCenter, fYCenter);

        float fBoxWidth = std::numeric_limits<float>::infinity();
        if (fAbsWidth_ != 0.0f)
            fBoxWidth = fAbsWidth_;
        else if (lDefinedBorderList_.left && lDefinedBorderList_.right)
            fBoxWidth = fRight - fLeft;

        float fBoxHeight = std::numeric_limits<float>::infinity();
        if (fAbsHeight_ != 0.0f)
            fBoxHeight = fAbsHeight_;
        else if (lDefinedBorderList_.top && lDefinedBorderList_.bottom)
            fBoxHeight = fBottom - fTop;

        fBoxWidth = round_to_pixel(fBoxWidth, utils::rounding_method::NEAREST_NOT_ZERO);
        fBoxHeight = round_to_pixel(fBoxHeight, utils::rounding_method::NEAREST_NOT_ZERO);

        pText_->set_dimensions(fBoxWidth, fBoxHeight);

        DEBUG_LOG("  Make borders");
        if (std::isinf(fBoxHeight))
            fBoxHeight = pText_->get_height();
        if (std::isinf(fBoxWidth))
            fBoxWidth = pText_->get_width();

        make_borders_(fTop, fBottom, fYCenter, fBoxHeight);
        make_borders_(fLeft, fRight, fXCenter, fBoxWidth);

        if (bReady_)
        {
            if (fRight < fLeft)
                fRight = fLeft + 1.0f;
            if (fBottom < fTop)
                fBottom = fTop + 1.0f;

            lBorderList_.left   = fLeft;
            lBorderList_.right  = fRight;
            lBorderList_.top    = fTop;
            lBorderList_.bottom = fBottom;
        }
        else
            lBorderList_ = bounds2f::ZERO;

        bUpdateBorders_ = false;
    }
    else
    {
        float fBoxWidth = fAbsWidth_;
        if (fBoxWidth == 0.0)
            fBoxWidth = pText_->get_width();

        float fBoxHeight = fAbsHeight_;
        if (fBoxHeight == 0.0)
            fBoxHeight = pText_->get_height();

        lBorderList_ = bounds2f(0.0, 0.0, fBoxWidth, fBoxHeight);
        bReady_ = false;
    }

    lBorderList_.left = round_to_pixel(lBorderList_.left);
    lBorderList_.right = round_to_pixel(lBorderList_.right);
    lBorderList_.top = round_to_pixel(lBorderList_.top);
    lBorderList_.bottom = round_to_pixel(lBorderList_.bottom);

    if (bReady_ || (!bReady_ && bOldReady))
    {
        DEBUG_LOG("  Fire redraw");
        notify_renderer_need_redraw();
    }
    DEBUG_LOG("  @");
}

}
}
