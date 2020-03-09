#include "lxgui/gui_text.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_sprite.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

typedef unsigned char uchar;

namespace gui
{
text::text(manager* pManager, const std::string& sFileName, float fSize) :
    pManager_(pManager), sFileName_(sFileName), bReady_(false), fSize_(fSize), fTracking_(0.0f),
    fLineSpacing_(1.5f), fSpaceWidth_(0.0f), bRemoveStartingSpaces_(false), bWordWrap_(true),
    bAddEllipsis_(false), mColor_(color::WHITE), bForceColor_(false), bFormattingEnabled_(false),
    fW_(0.0f), fH_(0.0f), fX_(std::numeric_limits<float>::infinity()),
    fY_(std::numeric_limits<float>::infinity()), fBoxW_(std::numeric_limits<float>::infinity()),
    fBoxH_(std::numeric_limits<float>::infinity()), mAlign_(ALIGN_LEFT), mVertAlign_(ALIGN_MIDDLE),
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
    pSprite_ = pManager_->create_sprite(pFont_->get_texture().lock());

    bReady_ = true;
}

text::~text()
{
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
    float fWidth = 0.0f;
    float fMaxWidth = std::numeric_limits<float>::infinity();

    if (bReady_)
    {
        utils::ustring::const_iterator iterChar, iterNext;
        foreach (iterChar, sUnicodeText_)
        {
            iterNext = iterChar + 1;
            if (*iterChar == '\n')
            {
                if (fWidth > fMaxWidth)
                    fMaxWidth = fWidth;

                fWidth = 0.0f;
            }
            else
            {
                fWidth += get_character_width(*iterChar) + fTracking_;
                if (iterNext != sUnicodeText_.end() && *iterChar != TO_U(' ') && *iterNext != TO_U(' ') && *iterNext != TO_U('\n'))
                    fWidth += get_character_kerning(*iterChar, *iterNext);
            }
        }
    }

    return fWidth;
}

float text::get_text_height() const
{
    float fHeight = 0.0f;

    if (bReady_)
    {
        uint count = 0;
        size_t pos = sText_.find("\n");
        while (pos != sText_.npos)
            pos = sText_.find("\n", pos+1);

        fHeight = (1.0f + count*fLineSpacing_)*get_line_height();
    }

    return fHeight;
}

float text::get_string_width(const std::string& sString) const
{
    return get_string_width(utils::UTF8_to_unicode(sString));
}

float text::get_string_width(const utils::ustring& sString) const
{
    float fWidth = 0.0f;
    float fMaxWidth = std::numeric_limits<float>::infinity();
    if (bReady_)
    {
        utils::ustring::const_iterator iterChar, iterNext;
        foreach (iterChar, sString)
        {
            iterNext = iterChar + 1;
            if (*iterChar == ' ')
                fWidth += fSpaceWidth_ + fTracking_;
            else if (*iterChar == '\t')
                fWidth += 4*fSpaceWidth_ + fTracking_;
            else if (*iterChar == '\n')
            {
                if (fWidth > fMaxWidth)
                    fMaxWidth = fWidth;
                fWidth = 0.0f;
            }
            else
            {
                fWidth += get_character_width(*iterChar) + fTracking_;
                if (iterNext != sString.end())
                {
                    if (*iterNext != TO_U(' ') && *iterNext != TO_U('\n'))
                        fWidth += get_character_kerning(*iterChar, *iterNext);
                }
            }
        }
    }

    return fWidth;
}

float text::get_character_width(char32_t uiChar) const
{
    if (bReady_)
    {
        if (uiChar == TO_U(' '))
            return fSpaceWidth_;
        else if (uiChar == TO_U('\t'))
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
    if (bReady_)
    {
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

            std::vector<letter>::iterator iterLetter;
            foreach (iterLetter, lLetterCache_)
            {
                if (iterLetter->bNoRender)
                    continue;

                quad2f mQuad = iterLetter->mQuad + vector2f(fX, fY);

                lVertexList[0].pos = mQuad.top_left();
                lVertexList[1].pos = mQuad.top_right();
                lVertexList[2].pos = mQuad.bottom_right();
                lVertexList[3].pos = mQuad.bottom_left();

                lVertexList[0].uvs = iterLetter->mUVs.top_left();
                lVertexList[1].uvs = iterLetter->mUVs.top_right();
                lVertexList[2].uvs = iterLetter->mUVs.bottom_right();
                lVertexList[3].uvs = iterLetter->mUVs.bottom_left();

                if (bFormattingEnabled_)
                {
                    if (iterLetter->mColor != color::EMPTY && !bForceColor_)
                    {
                        for (uint i = 0; i < 4; ++i)
                            lVertexList[i].col = iterLetter->mColor;
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

        pSprite_->render_quads(lQuadList_);
    }
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

void get_format(utils::ustring::iterator& iterChar, text::format& mFormat)
{
    if (*iterChar == 'r')
    {
        mFormat.mColorAction = text::COLOR_ACTION_RESET;
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

        mFormat.mColorAction = text::COLOR_ACTION_SET;
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
            return;
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
        std::vector<utils::ustring> lManualLineList = utils::cut_each(sUnicodeText_, TO_U("\n"));
        std::vector<utils::ustring>::iterator iterManual;
        foreach (iterManual, lManualLineList)
        {
            DEBUG_LOG("     Line : '" + utils::unicode_to_UTF8(*iterManual) + "'");
            // Make a temporary line array
            std::vector<line> lLines;
            line mLine; mLine.fWidth = 0.0f;
            std::map<uint, format> lTempFormatList;

            DEBUG_LOG("     Read chars");
            utils::ustring::iterator iterChar1;
            foreach (iterChar1, *iterManual)
            {
                DEBUG_LOG("      char '" + utils::to_string(*iterChar1) + "'");
                DEBUG_LOG("      Read format");
                // Read format tags
                if (*iterChar1 == TO_U('|') && bFormattingEnabled_)
                {
                    ++iterChar1;
                    if (iterChar1 != iterManual->end())
                    {
                        if (*iterChar1 != TO_U('|'))
                        {
                            get_format(iterChar1, lTempFormatList[uiCounter+mLine.sCaption.size()]);
                            continue;
                        }
                    }
                    else
                        break;
                }

                DEBUG_LOG("      Get width");
                if (*iterChar1 == TO_U(' '))
                    mLine.fWidth += fSpaceWidth_ + fTracking_;
                else if (*iterChar1 == TO_U('\t'))
                    mLine.fWidth += 4*fSpaceWidth_ + fTracking_;
                else
                {
                    mLine.fWidth += get_character_width(*iterChar1) + fTracking_;
                    utils::ustring::iterator iterNext = iterChar1 + 1;
                    if (iterNext != iterManual->end())
                    {
                        if (*iterNext != TO_U(' '))
                            mLine.fWidth += get_character_kerning(*iterChar1, *iterNext);
                    }
                }

                mLine.sCaption += *iterChar1;

                if (mLine.fWidth > fBoxW_)
                {
                    DEBUG_LOG("      Box break " + utils::to_string(mLine.fWidth) + " > " + utils::to_string(fBoxW_));
                    // Whoops, the line is too long...
                    if (mLine.sCaption.find(TO_U(" ")) != mLine.sCaption.npos && bWordWrap_)
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
                            if (*iterChar2 == TO_U(' '))
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
                            while (iterChar2 != mLine.sCaption.end() && *iterChar2 == TO_U(' '))
                            {
                                --uiCharToErase;
                                sErasedString.erase(0, 1);
                                ++iterChar2;
                            }
                        }

                        mLine.sCaption.erase(mLine.sCaption.size() - uiCharToErase, uiCharToErase);

                        lLines.push_back(mLine);
                        std::map<uint, format>::iterator iterFormat;
                        foreach (iterFormat, lTempFormatList)
                            lFormatList_[iterFormat->first] = iterFormat->second;

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
                            float fWordWidth = 3.0f*(get_character_width(TO_U('.')) + fTracking_);
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
                            mLine.sCaption += TO_U("...");
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
                            // Word wrap is disabled, so we can only display one line
                            // anyway.
                            lLineList_.push_back(mLine);
                            std::map<uint, format>::iterator iterFormat;
                            foreach (iterFormat, lTempFormatList)
                                lFormatList_[iterFormat->first] = iterFormat->second;

                            return;
                        }

                        DEBUG_LOG("       Continue");
                        utils::ustring::iterator iterTemp = iterChar1;
                        size_t pos = iterManual->find(TO_U(" "), iterChar1 - iterManual->begin());
                        if (pos == iterManual->npos)
                            iterChar1 = iterManual->end();
                        else
                            iterChar1 = iterManual->begin() + pos;

                        if (iterChar1 != iterManual->end())
                        {
                            // Read cutted format tags
                            if (bFormattingEnabled_)
                            {
                                while (iterTemp != iterChar1)
                                {
                                    if (*iterTemp == TO_U('|'))
                                    {
                                        ++iterTemp;
                                        if (iterTemp != iterChar1 && *iterTemp != TO_U('|'))
                                            get_format(iterTemp, lTempFormatList[uiCounter+mLine.sCaption.size()]);
                                    }
                                    ++iterTemp;
                                }
                            }

                            // Look for the next word
                            while (iterChar1 != iterManual->end())
                            {
                                if ((*iterChar1) == TO_U(' '))
                                    ++iterChar1;
                                else
                                    break;
                            }

                            // Add the line
                            if (iterChar1 != iterManual->end())
                            {
                                --iterChar1;
                                lLines.push_back(mLine);
                                uiCounter += mLine.sCaption.size();

                                std::map<uint, format>::iterator iterFormat;
                                foreach (iterFormat, lTempFormatList)
                                    lFormatList_[iterFormat->first] = iterFormat->second;

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
                mLine.sCaption += TO_U("\n");

            lLines.push_back(mLine);
            std::map<uint, format>::iterator iterFormat;
            foreach (iterFormat, lTempFormatList)
                lFormatList_[iterFormat->first] = iterFormat->second;

            lTempFormatList.clear();
            uiCounter += mLine.sCaption.size() - 1;

            // Add the maximum number of line to this text
            std::vector<line>::iterator iterLine;
            foreach (iterLine, lLines)
            {
                lLineList_.push_back(*iterLine);
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
            std::vector<line>::iterator iterLine;
            foreach (iterLine, lLineList_)
                fW_ = std::max(fW_, iterLine->fWidth);
        }
        else
            fW_ = fBoxW_;

        fH_ = (1.0f + (lLineList_.size() - 1)*fLineSpacing_)*get_line_height();

        float fX = 0.0f, fY = 0.0f;
        float fX0 = 0.0f;

        if (fBoxW_ != 0.0f && !math::isinf(fBoxW_))
        {
            switch (mAlign_)
            {
                case ALIGN_LEFT :
                    fX0 = 0.0f;
                    break;
                case ALIGN_CENTER :
                    fX0 = floor(fBoxW_*0.5f);
                    break;
                case ALIGN_RIGHT :
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
                case ALIGN_TOP :
                    fY = 0.0f;
                    break;
                case ALIGN_MIDDLE :
                    fY = floor((fBoxH_ - fH_)*0.5f);
                    break;
                case ALIGN_BOTTOM :
                    fY = (fBoxH_ - fH_);
                    break;
            }
        }
        else
        {
            switch (mVertAlign_)
            {
                case ALIGN_TOP :
                    fY = 0.0f;
                    break;
                case ALIGN_MIDDLE :
                    fY = -floor(fH_*0.5f);
                    break;
                case ALIGN_BOTTOM :
                    fY = -fH_;
                    break;
            }
        }

        uint uiCounter = 0;

        letter mLetter;

        color mColor = color::EMPTY;

        std::vector<line>::iterator iterLine;
        foreach (iterLine, lLineList_)
        {
            switch (mAlign_)
            {
                case ALIGN_LEFT :
                    fX = fX0;
                    break;
                case ALIGN_CENTER :
                    fX = fX0 - floor(iterLine->fWidth*0.5f);
                    break;
                case ALIGN_RIGHT :
                    fX = fX0 - iterLine->fWidth;
                    break;
            }

            utils::ustring::iterator iterChar, iterNext;
            foreach (iterChar, iterLine->sCaption)
            {
                // format our text
                if (bFormattingEnabled_ && lFormatList_.find(uiCounter) != lFormatList_.end())
                {
                    const format& mFormat = lFormatList_[uiCounter];
                    switch (mFormat.mColorAction)
                    {
                        case COLOR_ACTION_SET :
                            mColor = mFormat.mColor;
                            break;
                        case COLOR_ACTION_RESET :
                            mColor = color::EMPTY;
                            break;
                        default : break;
                    }
                }

                float fCharWidth, fCharHeight;

                // Add the character to the cache
                if (*iterChar == '\n')
                {
                    quad2f lUVs = pFont_->get_character_uvs(TO_U('_'));
                    fCharHeight = lUVs.height()*pFont_->get_texture()->get_height();
                    float fYOffset = floor(fSize_/2.0f + fSize_/8.0f - fCharHeight/2.0f);

                    mLetter.mQuad = quad2f(0.0f, 0.0f, fYOffset, fYOffset+fCharHeight) + vector2f(fX, fY);
                    mLetter.bNoRender = true;

                    lLetterCache_.push_back(mLetter);

                    continue; // Don't increase the uiCounter
                }
                else if (*iterChar == TO_U(' ') || *iterChar == TO_U('\t'))
                {
                    quad2f lUVs = pFont_->get_character_uvs(TO_U('!'));
                    fCharWidth = fSpaceWidth_;
                    if (*iterChar == TO_U('\t'))
                        fCharWidth *= 4;
                    fCharHeight = lUVs.height()*pFont_->get_texture()->get_height();
                    float fYOffset = floor(fSize_/2.0f + fSize_/8.0f - fCharHeight/2.0f);

                    mLetter.mQuad = quad2f(0.0f, fCharWidth, fYOffset, fYOffset+fCharHeight) + vector2f(fX, fY);
                    mLetter.bNoRender = true;

                    lLetterCache_.push_back(mLetter);
                }
                else
                {
                    quad2f lUVs = pFont_->get_character_uvs(*iterChar);
                    fCharWidth = get_character_width(*iterChar);
                    fCharHeight = lUVs.height()*pFont_->get_texture()->get_height();
                    float fYOffset = floor(fSize_/2.0f + fSize_/8.0f - fCharHeight/2.0f);

                    mLetter.mQuad = quad2f(0.0f, fCharWidth, fYOffset, fYOffset+fCharHeight) + vector2f(fX, fY);
                    mLetter.mUVs = lUVs;
                    mLetter.mColor = mColor;
                    mLetter.bNoRender = false;

                    lLetterCache_.push_back(mLetter);
                }

                iterNext = iterChar + 1;

                float fKerning = 0.0f;
                if (iterNext != iterLine->sCaption.end() && *iterNext != TO_U(' ') && *iterChar != TO_U(' '))
                    fKerning = get_character_kerning(*iterChar, *iterNext);

                fX += fCharWidth + fKerning + fTracking_;
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

std::unique_ptr<sprite> text::create_sprite(char32_t uiChar) const
{
    quad2f lUVs = pFont_->get_character_uvs(uiChar);

    float fWidth = get_character_width(uiChar);
    float fHeight = lUVs.height()*pFont_->get_texture()->get_height();

    std::unique_ptr<sprite> pSprite = pManager_->create_sprite(pFont_->get_texture().lock(), fWidth, fHeight);
    pSprite->set_texture_rect(lUVs.left, lUVs.top, lUVs.right, lUVs.bottom, true);

    pSprite->set_color(mColor_);

    return pSprite;
}

const std::vector<text::letter>& text::get_letter_cache()
{
    update();
    return lLetterCache_;
}
}
