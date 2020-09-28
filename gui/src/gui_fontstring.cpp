#include "lxgui/gui_fontstring.hpp"

#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_uiobject_tpl.hpp"

#include <sstream>

namespace lxgui {
namespace gui
{
const uint  OUTLINE_QUALITY   = 10;
const float OUTLINE_THICKNESS = 2.0f;

font_string::font_string(manager* pManager) : layered_region(pManager)
{
    lType_.push_back(CLASS_NAME);
}

void font_string::render()
{
    if (pText_ && is_visible())
    {
        float fX = 0.0f, fY = 0.0f;

        if (math::isinf(pText_->get_box_width()))
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

        if (math::isinf(pText_->get_box_height()))
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

        fX += iXOffset_;
        fY += iYOffset_;

        if (bHasShadow_)
        {
            pText_->set_color(mShadowColor_, true);
            pText_->render(fX + iShadowXOffset_, fY + iShadowYOffset_);
        }

        if (bIsOutlined_)
        {
            pText_->set_color(color(0, 0, 0, mTextColor_.a), true);
            for (uint i = 0; i < OUTLINE_QUALITY; ++i)
            {
                static const float PI2 = 2.0f*acos(-1.0f);

                pText_->render(
                    fX + OUTLINE_THICKNESS*cos(PI2*float(i)/OUTLINE_QUALITY),
                    fY + OUTLINE_THICKNESS*sin(PI2*float(i)/OUTLINE_QUALITY)
                );
            }
        }

        pText_->set_color(mTextColor_);
        pText_->render(fX, fY);
    }
}

void font_string::update(float fDelta)
{
    //#define DEBUG_LOG(msg) gui::out << (msg) << std::endl
    #define DEBUG_LOG(msg)

    DEBUG_LOG("  ~");
    if (pText_)
    {
        DEBUG_LOG("   Update text");
        pText_->update();
    }

    uiobject::update(fDelta);
    DEBUG_LOG("   .");
}

std::string font_string::serialize(const std::string& sTab) const
{
    std::ostringstream sStr;

    sStr << layered_region::serialize(sTab);

    sStr << sTab << "  # Font name   : " << sFontName_ << "\n";
    sStr << sTab << "  # Font height : " << uiHeight_ << "\n";
    sStr << sTab << "  # Text ready  : " << (pText_ != nullptr) << "\n";
    sStr << sTab << "  # Text        : \"" << sText_ << "\"\n";
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
    sStr << sTab << "  # Shadow off. : (" << iShadowXOffset_ << ", " << iShadowYOffset_ << ")\n";
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

    font_string* pFontString = pObj->down_cast<font_string>();
    if (!pFontString)
        return;

    std::string sFontName = pFontString->get_font_name();
    uint uiHeight = pFontString->get_font_height();
    if (!sFontName.empty() && uiHeight != 0)
        this->set_font(sFontName, uiHeight);

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

uint font_string::get_font_height() const
{
    return uiHeight_;
}

void font_string::set_outlined(bool bIsOutlined)
{
    if (bIsOutlined_ != bIsOutlined)
    {
        bIsOutlined_ = bIsOutlined;
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

vector2i font_string::get_shadow_offsets() const
{
    return vector2i(iShadowXOffset_, iShadowYOffset_);
}

vector2i font_string::get_offsets() const
{
    return vector2i(iXOffset_, iYOffset_);
}

int font_string::get_shadow_x_offset() const
{
    return iShadowXOffset_;
}

int font_string::get_shadow_y_offset() const
{
    return iShadowYOffset_;
}

float font_string::get_spacing() const
{
    return fSpacing_;
}

const color& font_string::get_text_color() const
{
    return mTextColor_;
}

void font_string::set_font(const std::string& sFontName, uint uiHeight)
{
    sFontName_ = sFontName;
    uiHeight_ = uiHeight;

    pText_ = std::unique_ptr<text>(new text(get_top_level_renderer(), sFontName, uiHeight));
    pText_->set_remove_starting_spaces(true);
    pText_->set_text(sText_);
    pText_->set_alignment(mJustifyH_);
    pText_->set_vertical_alignment(mJustifyV_);
    pText_->set_tracking(fSpacing_);
    pText_->enable_word_wrap(bCanWordWrap_, bAddEllipsis_);
    pText_->enable_formatting(bFormattingEnabled_);

    fire_update_borders();
    notify_renderer_need_redraw();
}

void font_string::set_justify_h(text::alignment mJustifyH)
{
    if (mJustifyH_ != mJustifyH)
    {
        mJustifyH_ = mJustifyH;
        if (pText_)
        {
            pText_->set_alignment(mJustifyH_);
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
            notify_renderer_need_redraw();
        }
    }
}

void font_string::set_shadow_color(const color& mShadowColor)
{
    if (mShadowColor_ != mShadowColor)
    {
        mShadowColor_ = mShadowColor;
        if (bHasShadow_)
            notify_renderer_need_redraw();
    }
}

void font_string::set_shadow_offsets(int iShadowXOffset, int iShadowYOffset)
{
    if (iShadowXOffset_ != iShadowXOffset || iShadowYOffset_ != iShadowYOffset)
    {
        iShadowXOffset_ = iShadowXOffset;
        iShadowYOffset_ = iShadowYOffset;
        if (bHasShadow_)
            notify_renderer_need_redraw();
    }
}

void font_string::set_shadow_offsets(const vector2i& mShadowOffsets)
{
    if (iShadowXOffset_ != mShadowOffsets.x || iShadowYOffset_ != mShadowOffsets.y)
    {
        iShadowXOffset_ = mShadowOffsets.x;
        iShadowYOffset_ = mShadowOffsets.y;
        if (bHasShadow_)
            notify_renderer_need_redraw();
    }
}

void font_string::set_offsets(int iXOffset, int iYOffset)
{
    if (iXOffset_ != iXOffset || iYOffset_ != iYOffset)
    {
        iXOffset_ = iXOffset;
        iYOffset_ = iYOffset;
        notify_renderer_need_redraw();
    }
}

void font_string::set_offsets(const vector2i& mOffsets)
{
    if (iXOffset_ != mOffsets.x || iYOffset_ != mOffsets.y)
    {
        iXOffset_ = mOffsets.x;
        iYOffset_ = mOffsets.y;
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
            notify_renderer_need_redraw();
        }
    }
}

void font_string::set_text_color(const color& mTextColor)
{
    if (mTextColor_ != mTextColor)
    {
        mTextColor_ = mTextColor;
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

const std::string& font_string::get_text() const
{
    return sText_;
}

const utils::ustring& font_string::get_unicode_text() const
{
    static const utils::ustring empty;
    if (pText_)
        return pText_->get_unicode_text();
    else
        return empty;
}

void font_string::set_non_space_wrap(bool bCanNonSpaceWrap)
{
    if (bCanNonSpaceWrap_ != bCanNonSpaceWrap)
    {
        bCanNonSpaceWrap_ = bCanNonSpaceWrap;
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

void font_string::set_text(const std::string& sText)
{
    if (sText_ != sText)
    {
        sText_ = sText;
        if (pText_)
        {
            pText_->set_text(sText_);
            fire_update_borders();
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

    if (bUpdateDimensions_)
    {
        DEBUG_LOG("  Update dimentions");
        update_dimensions_();
        bUpdateDimensions_ = false;
    }

    if (!lAnchorList_.empty())
    {
        float fLeft = 0.0f, fRight = 0.0f, fTop = 0.0f, fBottom = 0.0f;
        float fXCenter = 0.0f, fYCenter = 0.0f;

        DEBUG_LOG("  Read anchors");
        read_anchors_(fLeft, fRight, fTop, fBottom, fXCenter, fYCenter);

        if (uiAbsWidth_ == 0u)
        {
            if (lDefinedBorderList_.left && lDefinedBorderList_.right)
                pText_->set_box_width(fRight - fLeft);
            else
                pText_->set_box_width(std::numeric_limits<float>::infinity());
        }
        else
            pText_->set_box_width(uiAbsWidth_);

        if (uiAbsHeight_ == 0u)
        {
            if (lDefinedBorderList_.top && lDefinedBorderList_.bottom)
                pText_->set_box_height(fBottom - fTop);
            else
                pText_->set_box_height(std::numeric_limits<float>::infinity());
        }
        else
            pText_->set_box_height(uiAbsHeight_);

        DEBUG_LOG("  Make borders");
        if (uiAbsHeight_ != 0u)
            make_borders_(fTop, fBottom, fYCenter, uiAbsHeight_);
        else
            make_borders_(fTop, fBottom, fYCenter, pText_->get_height());

        if (uiAbsWidth_ != 0u)
            make_borders_(fLeft, fRight, fXCenter, uiAbsWidth_);
        else
            make_borders_(fLeft, fRight, fXCenter, pText_->get_width());

        if (bReady_)
        {
            int iLeft = fLeft, iRight = fRight, iTop = fTop, iBottom = fBottom;

            if (iRight < iLeft)
                iRight = iLeft+1;
            if (iBottom < iTop)
                iBottom = iTop+1;

            lBorderList_.left   = iLeft;
            lBorderList_.right  = iRight;
            lBorderList_.top    = iTop;
            lBorderList_.bottom = iBottom;

            DEBUG_LOG("  Update dimentions");
            update_dimensions_();
        }
        else
            lBorderList_ = quad2i::ZERO;

        bUpdateBorders_ = false;
    }
    else
        bReady_ = false;

    if (bReady_ || (!bReady_ && bOldReady))
    {
        DEBUG_LOG("  Fire redraw");
        notify_renderer_need_redraw();
    }
    DEBUG_LOG("  @");
}
}
}
