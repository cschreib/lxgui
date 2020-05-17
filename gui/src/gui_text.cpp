#include "lxgui/gui_text.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

typedef unsigned char uchar;

namespace lxgui {
namespace gui
{
text::text(manager* pManager, const std::string& sFileName, float fSize) :
    pManager_(pManager), sFileName_(sFileName), bReady_(false), fSize_(fSize), fTracking_(0.0f),
    fLineSpacing_(1.5f), fSpaceWidth_(0.0f), bRemoveStartingSpaces_(false), bWordWrap_(true),
    bAddEllipsis_(false), mColor_(color::WHITE), bForceColor_(false), bFormattingEnabled_(false),
    fW_(0.0f), fH_(0.0f), fX_(std::numeric_limits<float>::infinity()),
    fY_(std::numeric_limits<float>::infinity()), fBoxW_(std::numeric_limits<float>::infinity()),
    fBoxH_(std::numeric_limits<float>::infinity()), mAlign_(alignment::LEFT), mVertAlign_(vertical_alignment::MIDDLE),
    bUpdateCache_(false), bUpdateQuads_(false)

{
    pFont_ = pManager_->create_font(sFileName_, fSize_);
    if (!pFont_)
    {
        gui::out << gui::error << "gui::text : "
            "Error initializing \"" << sFileName << "\" (size : " << fSize << ")." << std::endl;
        return;
    }

    fSpaceWidth_ = pFont_->get_character_width(32);
    mSprite_ = pManager_->create_sprite(pFont_->get_texture().lock());

    bReady_ = true;
}

const std::string& text::get_font_name() const
{
    return sFileName_;
}

float text::get_font_size() const
{
    return fSize_;
}

float text::get_line_height() const
{
    return fSize_;
}

void text::set_text(const std::string& sText)
{
    if (sText_ != sText)
    {
        sText_ = sText;
        sUnicodeText_ = utils::UTF8_to_unicode(sText_);
        bUpdateCache_ = true;
    }
}

const std::string& text::get_text() const
{
    return sText_;
}

const utils::ustring& text::get_unicode_text() const
{
    return sUnicodeText_;
}

void text::set_color(const color& mColor, bool bForceColor)
{
    if (mColor_ != mColor || bForceColor_ != bForceColor)
    {
        mColor_ = mColor;
        bForceColor_ = bForceColor;
        bUpdateQuads_ = true;
    }
}

const color& text::get_color() const
{
    return mColor_;
}

void text::set_dimensions(float fW, float fH)
{
    if (fBoxW_ != fW && fBoxH_ != fH)
    {
        fBoxW_ = fW; fBoxH_ = fH;
        bUpdateCache_ = true;
    }
}

void text::set_box_width(float fBoxW)
{
    if (fBoxW_ != fBoxW)
    {
        fBoxW_ = fBoxW;
        bUpdateCache_ = true;
    }
}

void text::set_box_height(float fBoxH)
{
    if (fBoxH_ != fBoxH)
    {
        fBoxH_ = fBoxH;
        bUpdateCache_ = true;
    }
}

float text::get_width()
{
    update();

    return fW_;
}

float text::get_height()
{
    update();

    return fH_;
}

float text::get_box_width() const
{
    return fBoxW_;
}

float text::get_box_height() const
{
    return fBoxH_;
}

float text::get_text_width() const
{
    return get_string_width(sUnicodeText_);
}

float text::get_text_height() const
{
    if (!bReady_)
        return 0.0f;

    uint count = std::count(sText_.begin(), sText_.end(), '\n');
    float fHeight = (1.0f + count*fLineSpacing_)*get_line_height();

    return fHeight;
}

uint text::get_num_lines() const
{
    return lLineList_.size();
}

float text::get_string_width(const std::string& sString) const
{
    return get_string_width(utils::UTF8_to_unicode(sString));
}

float text::get_string_width(const utils::ustring& sString) const
{
    if (!bReady_)
        return 0.0f;

    float fWidth = 0.0f;
    float fMaxWidth = 0.0f;

    for (auto iterChar : utils::range::iterator(sString))
    {
        if (*iterChar == '\n')
        {
            if (fWidth > fMaxWidth)
                fMaxWidth = fWidth;

            fWidth = 0.0f;
        }
        else
        {
            fWidth += get_character_width(*iterChar) + fTracking_;

            auto iterNext = iterChar + 1;
            if (iterNext != sString.end())
            {
                if (*iterChar != U' ' && *iterNext != U' ' && *iterNext != U'\n')
                    fWidth += get_character_kerning(*iterChar, *iterNext);
            }
        }
    }

    if (fWidth > fMaxWidth)
        fMaxWidth = fWidth;

    return fMaxWidth;
}

float text::get_character_width(char32_t uiChar) const
{
    if (bReady_)
    {
        if (uiChar == U' ')
            return fSpaceWidth_;
        else if (uiChar == U'\t')
            return 4.0f*fSpaceWidth_;
        else
            return pFont_->get_character_width(uiChar);
    }
    else
        return 0.0f;
}

float text::get_character_kerning(char32_t uiChar1, char32_t uiChar2) const
{
    return pFont_->get_character_kerning(uiChar1, uiChar2);
}

void text::set_alignment(const text::alignment& mAlign)
{
    if (mAlign_ != mAlign)
    {
        mAlign_ = mAlign;
        bUpdateCache_ = true;
    }
}

void text::set_vertical_alignment(const text::vertical_alignment& mVertAlign)
{
    if (mVertAlign_ != mVertAlign)
    {
        mVertAlign_ = mVertAlign;
        bUpdateCache_ = true;
    }
}

const text::alignment& text::get_alignment() const
{
    return mAlign_;
}

const text::vertical_alignment& text::get_vertical_alignment() const
{
    return mVertAlign_;
}

void text::set_tracking(float fTracking)
{
    if (fTracking_ != fTracking)
    {
        fTracking_ = fTracking;
        bUpdateCache_ = true;
    }
}

float text::get_tracking() const
{
    return fTracking_;
}

void text::set_line_spacing(float fLineSpacing)
{
    if (fLineSpacing_ != fLineSpacing)
    {
        fLineSpacing_ = fLineSpacing;
        bUpdateCache_ = true;
    }
}

float text::get_line_spacing() const
{
    return fLineSpacing_;
}

void text::set_remove_starting_spaces(bool bRemoveStartingSpaces)
{
    if (bRemoveStartingSpaces_ != bRemoveStartingSpaces)
    {
        bRemoveStartingSpaces_ = bRemoveStartingSpaces;
        bUpdateCache_ = true;
    }
}

bool text::get_remove_starting_spaces() const
{
    return bRemoveStartingSpaces_;
}

void text::enable_word_wrap(bool bWrap, bool bAddEllipsis)
{
    if (bWordWrap_ != bWrap || bAddEllipsis_ != bAddEllipsis)
    {
        bWordWrap_ = bWrap;
        bAddEllipsis_ = bAddEllipsis;
        bUpdateCache_ = true;
    }
}

bool text::is_word_wrap_enabled() const
{
    return bWordWrap_;
}

void text::enable_formatting(bool bFormatting)
{
    if (bFormattingEnabled_ != bFormatting)
    {
        bFormattingEnabled_ = bFormatting;
        bUpdateCache_ = true;
    }
}

void text::render(float fX, float fY)
{
    if (!bReady_)
        return;

    update();

    if (fX != fX_ || fY != fY_)
        bUpdateQuads_ = true;

    if (bUpdateQuads_)
    {
        fX_ = fX;
        fY_ = fY;

        lQuadList_.clear();

        std::array<vertex,4> lVertexList;

        if (!bFormattingEnabled_)
        {
            for (uint i = 0; i < 4; ++i)
                lVertexList[i].col = mColor_;
        }

        for (const auto& mLetter : lLetterCache_)
        {
            if (mLetter.bNoRender)
                continue;

            quad2f mQuad = mLetter.mQuad + vector2f(fX, fY);

            lVertexList[0].pos = mQuad.top_left();
            lVertexList[1].pos = mQuad.top_right();
            lVertexList[2].pos = mQuad.bottom_right();
            lVertexList[3].pos = mQuad.bottom_left();

            lVertexList[0].uvs = mLetter.mUVs.top_left();
            lVertexList[1].uvs = mLetter.mUVs.top_right();
            lVertexList[2].uvs = mLetter.mUVs.bottom_right();
            lVertexList[3].uvs = mLetter.mUVs.bottom_left();

            if (bFormattingEnabled_)
            {
                if (mLetter.mColor != color::EMPTY && !bForceColor_)
                {
                    for (uint i = 0; i < 4; ++i)
                        lVertexList[i].col = mLetter.mColor;
                }
                else
                {
                    for (uint i = 0; i < 4; ++i)
                        lVertexList[i].col = mColor_;
                }
            }

            lQuadList_.push_back(lVertexList);
        }

        bUpdateQuads_ = false;
    }

    mSprite_.render_quads(lQuadList_);
}

void text::update()
{
    //#define DEBUG_LOG(msg) gui::out << (msg) << std::endl
    #define DEBUG_LOG(msg)

    if (bReady_ && bUpdateCache_)
    {
        DEBUG_LOG("    Update lines");
        update_lines_();
        DEBUG_LOG("    Update cache");
        update_cache_();
        DEBUG_LOG("    .");
        bUpdateCache_ = false;
        bUpdateQuads_ = true;
    }
}

void get_format(utils::ustring::const_iterator& iterChar, text::format& mFormat)
{
    if (*iterChar == 'r')
    {
        mFormat.mColorAction = text::color_action::RESET;
    }
    else if (*iterChar == 'c')
    {
        std::string sColorPart;
        ++iterChar;
        sColorPart += *iterChar; ++iterChar;
        sColorPart += *iterChar; ++iterChar;
        float fA = utils::hex_to_uint(sColorPart)/255.0f;
        sColorPart.clear();
        sColorPart += *iterChar; ++iterChar;
        sColorPart += *iterChar; ++iterChar;
        float fR = utils::hex_to_uint(sColorPart)/255.0f;
        sColorPart.clear();
        sColorPart += *iterChar; ++iterChar;
        sColorPart += *iterChar; ++iterChar;
        float fG = utils::hex_to_uint(sColorPart)/255.0f;
        sColorPart.clear();
        sColorPart += *iterChar; ++iterChar;
        sColorPart += *iterChar;
        float fB = utils::hex_to_uint(sColorPart)/255.0f;

        mFormat.mColorAction = text::color_action::SET;
        mFormat.mColor = color(fR, fG, fB, fA);
    }
}

void text::update_lines_()
{
    // Update the line list, read format tags, do word wrapping, ...
    lLineList_.clear();
    lFormatList_.clear();

    DEBUG_LOG("     Get max line nbr");
    uint uiMaxLineNbr, uiCounter = 0;
    if (fBoxH_ != 0.0f && !math::isinf(fBoxH_))
    {
        if (fBoxH_ < get_line_height())
        {
            uiMaxLineNbr = 0;
        }
        else
        {
            float fRemaining = fBoxH_ - get_line_height();
            uiMaxLineNbr = 1 + floor(fRemaining/(get_line_height()*fLineSpacing_));
        }
    }
    else
        uiMaxLineNbr = uint(-1);

    if (uiMaxLineNbr != 0)
    {
        std::vector<utils::ustring> lManualLineList = utils::cut_each(sUnicodeText_, U"\n");
        for (auto iterManual : utils::range::iterator(lManualLineList))
        {
            const utils::ustring& sManualLine = *iterManual;

            DEBUG_LOG("     Line : '" + utils::unicode_to_UTF8(sManualLine) + "'");
            // Make a temporary line array
            std::vector<line> lLines;
            line mLine; mLine.fWidth = 0.0f;
            std::map<uint, format> lTempFormatList;

            DEBUG_LOG("     Read chars");
            for (auto iterChar1 = sManualLine.begin(); iterChar1 != sManualLine.end(); ++iterChar1)
            {
                DEBUG_LOG("      char '" + utils::to_string(*iterChar1) + "'");
                DEBUG_LOG("      Read format");
                // Read format tags
                if (*iterChar1 == U'|' && bFormattingEnabled_)
                {
                    ++iterChar1;
                    if (iterChar1 != sManualLine.end())
                    {
                        if (*iterChar1 != U'|')
                        {
                            get_format(iterChar1, lTempFormatList[uiCounter+mLine.sCaption.size()]);
                            continue;
                        }
                    }
                    else
                        break;
                }

                DEBUG_LOG("      Get width");
                if (*iterChar1 == U' ')
                    mLine.fWidth += fSpaceWidth_ + fTracking_;
                else if (*iterChar1 == U'\t')
                    mLine.fWidth += 4*fSpaceWidth_ + fTracking_;
                else
                {
                    mLine.fWidth += get_character_width(*iterChar1) + fTracking_;
                    auto iterNext = iterChar1 + 1;
                    if (iterNext != sManualLine.end())
                    {
                        if (*iterNext != U' ')
                            mLine.fWidth += get_character_kerning(*iterChar1, *iterNext);
                    }
                }

                mLine.sCaption += *iterChar1;

                if (mLine.fWidth > fBoxW_)
                {
                    DEBUG_LOG("      Box break " + utils::to_string(mLine.fWidth) + " > " + utils::to_string(fBoxW_));
                    // Whoops, the line is too long...
                    if (mLine.sCaption.find(U" ") != mLine.sCaption.npos && bWordWrap_)
                    {
                        DEBUG_LOG("       Spaced");
                        // There are several words on this line, we'll
                        // be able to put the last one on the next line
                        utils::ustring::iterator iterChar2 = mLine.sCaption.end();
                        utils::ustring sErasedString;
                        uint uiCharToErase = 0;
                        float fErasedWidth = 0.0f;
                        bool bLastWasWord = false;
                        while (mLine.fWidth > fBoxW_ && iterChar2 != mLine.sCaption.begin())
                        {
                            --iterChar2;
                            if (*iterChar2 == U' ')
                            {
                                if (!bLastWasWord || bRemoveStartingSpaces_ || mLine.fWidth - fErasedWidth > fBoxW_)
                                {
                                    mLine.fWidth -= fErasedWidth + fSpaceWidth_ + fTracking_;
                                    sErasedString.insert(sErasedString.begin(), *iterChar2);
                                    fErasedWidth = 0.0f;
                                    ++uiCharToErase;
                                }
                                else
                                    break;
                            }
                            else
                            {
                                fErasedWidth += get_character_width(*iterChar2) + fTracking_;
                                sErasedString.insert(sErasedString.begin(), *iterChar2);
                                ++uiCharToErase;
                                bLastWasWord = true;
                            }
                        }

                        if (bRemoveStartingSpaces_)
                        {
                            while (iterChar2 != mLine.sCaption.end() && *iterChar2 == U' ')
                            {
                                --uiCharToErase;
                                sErasedString.erase(0, 1);
                                ++iterChar2;
                            }
                        }

                        mLine.sCaption.erase(mLine.sCaption.size() - uiCharToErase, uiCharToErase);

                        lLines.push_back(mLine);
                        for (auto& mFormat : lTempFormatList)
                            lFormatList_.insert(std::move(mFormat));

                        lTempFormatList.clear();
                        uiCounter += mLine.sCaption.size();
                        mLine.fWidth = get_string_width(sErasedString);
                        mLine.sCaption = sErasedString;
                    }
                    else
                    {
                        DEBUG_LOG("       Single word");
                        // There is only one word on this line, or word
                        // wrap is disabled. Anyway, this line is just
                        // too long for the text box : our only option
                        // is to truncate it.
                        if (bAddEllipsis_)
                        {
                            DEBUG_LOG("       Ellipsis");
                            float fWordWidth = 3.0f*(get_character_width(U'.') + fTracking_);
                            utils::ustring::iterator iterChar2 = mLine.sCaption.end();
                            uint uiCharToErase = 0;
                            while ((mLine.fWidth + fWordWidth > fBoxW_) && (iterChar2 != mLine.sCaption.begin()))
                            {
                                --iterChar2;
                                mLine.fWidth -= get_character_width(*iterChar2) + fTracking_;
                                ++uiCharToErase;
                            }

                            DEBUG_LOG("       Char to erase : " + utils::to_string(uiCharToErase) + " / "
                                + utils::to_string(mLine.sCaption.size()));

                            mLine.sCaption.erase(mLine.sCaption.size() - uiCharToErase, uiCharToErase);
                            mLine.sCaption += U"...";
                        }
                        else
                        {
                            DEBUG_LOG("       Truncate");
                            utils::ustring::iterator iterChar2 = mLine.sCaption.end();
                            uint uiCharToErase = 0;
                            while (mLine.fWidth  > fBoxW_ && iterChar2 != mLine.sCaption.begin())
                            {
                                --iterChar2;
                                mLine.fWidth -= get_character_width(*iterChar2) + fTracking_;
                                ++uiCharToErase;
                            }
                            mLine.sCaption.erase(mLine.sCaption.size() - uiCharToErase, uiCharToErase);
                        }

                        if (!bWordWrap_)
                        {
                            DEBUG_LOG("       Display single line");
                            // Word wrap is disabled, so we can only display one line anyway.
                            lLineList_.push_back(mLine);
                            for (const auto& mFormat : lTempFormatList)
                                lFormatList_[mFormat.first] = mFormat.second;

                            return;
                        }

                        DEBUG_LOG("       Continue");
                        auto iterTemp = iterChar1;
                        size_t pos = sManualLine.find(U" ", iterChar1 - sManualLine.begin());
                        if (pos == sManualLine.npos)
                            iterChar1 = sManualLine.end();
                        else
                            iterChar1 = sManualLine.begin() + pos;

                        if (iterChar1 != sManualLine.end())
                        {
                            // Read cutted format tags
                            if (bFormattingEnabled_)
                            {
                                while (iterTemp != iterChar1)
                                {
                                    if (*iterTemp == U'|')
                                    {
                                        ++iterTemp;
                                        if (iterTemp != iterChar1 && *iterTemp != U'|')
                                            get_format(iterTemp, lTempFormatList[uiCounter+mLine.sCaption.size()]);
                                    }
                                    ++iterTemp;
                                }
                            }

                            // Look for the next word
                            while (iterChar1 != sManualLine.end())
                            {
                                if ((*iterChar1) == U' ')
                                    ++iterChar1;
                                else
                                    break;
                            }

                            // Add the line
                            if (iterChar1 != sManualLine.end())
                            {
                                --iterChar1;
                                lLines.push_back(mLine);
                                uiCounter += mLine.sCaption.size();

                                for (auto& mFormat : lTempFormatList)
                                    lFormatList_.insert(std::move(mFormat));

                                lTempFormatList.clear();
                                mLine.fWidth = 0.0f;
                                mLine.sCaption.clear();
                            }
                            else
                                break;
                        }
                        else
                            break;
                    }
                }
            }

            DEBUG_LOG("     End");

            if (iterManual != lManualLineList.end() - 1)
                mLine.sCaption += U"\n";

            lLines.push_back(mLine);
            for (auto& mFormat : lTempFormatList)
                lFormatList_.insert(std::move(mFormat));

            lTempFormatList.clear();
            uiCounter += mLine.sCaption.size() - 1;

            // Add the maximum number of line to this text
            for (auto& sLine : lLines)
            {
                lLineList_.push_back(std::move(sLine));
                if (lLineList_.size() == uiMaxLineNbr)
                    return;
            }
            DEBUG_LOG("     .");
        }
    }
}

void text::update_cache_()
{
    lLetterCache_.clear();

    if (!lLineList_.empty())
    {
        if (fBoxW_ == 0.0f || math::isinf(fBoxW_))
        {
            fW_ = 0.0f;
            for (const auto& mLine : lLineList_)
                fW_ = std::max(fW_, mLine.fWidth);
        }
        else
            fW_ = fBoxW_;

        fH_ = (1.0f + (lLineList_.size() - 1)*fLineSpacing_)*get_line_height();

        float fX  = 0.0f, fY = 0.0f;
        float fX0 = 0.0f;

        if (fBoxW_ != 0.0f && !math::isinf(fBoxW_))
        {
            switch (mAlign_)
            {
                case alignment::LEFT :
                    fX0 = 0.0f;
                    break;
                case alignment::CENTER :
                    fX0 = floor(fBoxW_*0.5f);
                    break;
                case alignment::RIGHT :
                    fX0 = fBoxW_;
                    break;
            }
        }
        else
            fX0 = 0.0f;

        if (!math::isinf(fBoxH_))
        {
            switch (mVertAlign_)
            {
                case vertical_alignment::TOP :
                    fY = 0.0f;
                    break;
                case vertical_alignment::MIDDLE :
                    fY = floor((fBoxH_ - fH_)*0.5f);
                    break;
                case vertical_alignment::BOTTOM :
                    fY = (fBoxH_ - fH_);
                    break;
            }
        }
        else
        {
            switch (mVertAlign_)
            {
                case vertical_alignment::TOP :
                    fY = 0.0f;
                    break;
                case vertical_alignment::MIDDLE :
                    fY = -floor(fH_*0.5f);
                    break;
                case vertical_alignment::BOTTOM :
                    fY = -fH_;
                    break;
            }
        }

        uint   uiCounter = 0;
        letter mLetter;
        color  mColor = color::EMPTY;

        for (const auto& mLine : lLineList_)
        {
            switch (mAlign_)
            {
                case alignment::LEFT :
                    fX = fX0;
                    break;
                case alignment::CENTER :
                    fX = fX0 - floor(mLine.fWidth*0.5f);
                    break;
                case alignment::RIGHT :
                    fX = fX0 - mLine.fWidth;
                    break;
            }

            for (auto iterChar : utils::range::iterator(mLine.sCaption))
            {
                // Format our text
                if (bFormattingEnabled_ && lFormatList_.find(uiCounter) != lFormatList_.end())
                {
                    const format& mFormat = lFormatList_[uiCounter];
                    switch (mFormat.mColorAction)
                    {
                        case color_action::SET :
                            mColor = mFormat.mColor;
                            break;
                        case color_action::RESET :
                            mColor = color::EMPTY;
                            break;
                        default : break;
                    }
                }

                // Add the character to the cache
                mLetter.mQuad = pFont_->get_character_bounds(*iterChar) + vector2f(round(fX), round(fY));
                mLetter.mUVs = pFont_->get_character_uvs(*iterChar);
                mLetter.mColor = mColor;
                mLetter.bNoRender = (*iterChar == '\n' || *iterChar == ' ' || *iterChar == '\t');

                lLetterCache_.push_back(mLetter);

                if (*iterChar == '\n') continue; // Don't increase the uiCounter

                float fKerning = 0.0f;

                auto iterNext = iterChar + 1;
                if (iterNext != mLine.sCaption.end() && *iterNext != U' ' && *iterChar != U' ')
                    fKerning = get_character_kerning(*iterChar, *iterNext);

                fX += pFont_->get_character_width(*iterChar) + fKerning + fTracking_;
                ++uiCounter;
            }

            fY += get_line_height()*fLineSpacing_;
        }
    }
    else
    {
        fW_ = 0.0f;
        fH_ = 0.0f;
    }
}

sprite text::create_sprite(char32_t uiChar) const
{
    const quad2f lUVs = pFont_->get_character_uvs(uiChar);

    const quad2f mBounds = pFont_->get_character_bounds(uiChar);
    const float fWidth = mBounds.right - mBounds.left;
    const float fHeight = mBounds.bottom - mBounds.top;

    sprite mSprite = pManager_->create_sprite(pFont_->get_texture().lock(), fWidth, fHeight);
    mSprite.set_texture_rect(lUVs.left, lUVs.top, lUVs.right, lUVs.bottom, true);
    mSprite.set_color(mColor_);

    return mSprite;
}

const std::vector<text::letter>& text::get_letter_cache()
{
    update();
    return lLetterCache_;
}
}
}
