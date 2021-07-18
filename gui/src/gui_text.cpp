#include "lxgui/gui_text.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_sprite.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_vertexcache.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils.hpp>
#include <lxgui/utils_range.hpp>

#include <map>

namespace lxgui {
namespace gui
{

text::text(const renderer* pRenderer, std::shared_ptr<gui::font> pFont,
    std::shared_ptr<gui::font> pOutlineFont) :
    pRenderer_(pRenderer), pFont_(std::move(pFont)), pOutlineFont_(pOutlineFont)
{
    if (!pFont_)
        return;

    bReady_ = true;
}

float text::get_line_height() const
{
    if (pFont_)
        return pFont_->get_size()*fScalingFactor_;
    else
        return 0.0;
}

void text::set_scaling_factor(float fScalingFactor)
{
    if (fScalingFactor_ == fScalingFactor) return;

    fScalingFactor_ = fScalingFactor;
    notify_cache_dirty_();
}

float text::get_scaling_factor() const
{
    return fScalingFactor_;
}

void text::set_text(const utils::ustring& sText)
{
    if (sUnicodeText_ != sText)
    {
        sUnicodeText_ = sText;
        notify_cache_dirty_();
    }
}

const utils::ustring& text::get_text() const
{
    return sUnicodeText_;
}

void text::set_color(const color& mColor, bool bForceColor)
{
    if (mColor_ != mColor || bForceColor_ != bForceColor)
    {
        mColor_ = mColor;
        bForceColor_ = bForceColor;
        if (pRenderer_->is_vertex_cache_enabled())
            notify_cache_dirty_();
    }
}

const color& text::get_color() const
{
    return mColor_;
}

void text::set_dimensions(float fW, float fH)
{
    if (fBoxW_ != fW || fBoxH_ != fH)
    {
        fBoxW_ = fW; fBoxH_ = fH;
        notify_cache_dirty_();
    }
}

void text::set_box_width(float fBoxW)
{
    if (fBoxW_ != fBoxW)
    {
        fBoxW_ = fBoxW;
        notify_cache_dirty_();
    }
}

void text::set_box_height(float fBoxH)
{
    if (fBoxH_ != fBoxH)
    {
        fBoxH_ = fBoxH;
        notify_cache_dirty_();
    }
}

float text::get_width() const
{
    update_();

    return fW_;
}

float text::get_height() const
{
    update_();

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

    uint count = std::count(sUnicodeText_.begin(), sUnicodeText_.end(), U'\n');
    float fHeight = (1.0f + count*fLineSpacing_)*get_line_height();

    return fHeight;
}

uint text::get_num_lines() const
{
    update_();
    return uiNumLines_;
}

float text::get_string_width(const std::string& sString) const
{
    return get_string_width(utils::utf8_to_unicode(sString));
}

float text::get_string_width(const utils::ustring& sString) const
{
    if (!bReady_)
        return 0.0f;

    float fWidth = 0.0f;
    float fMaxWidth = 0.0f;

    for (auto iterChar : utils::range::iterator(sString))
    {
        if (*iterChar == U'\n')
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
                if (!utils::is_whitespace(*iterChar) && !utils::is_whitespace(*iterNext))
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
            return pFont_->get_character_width(32)*fScalingFactor_;
        else if (uiChar == U'\t')
            return 4.0f*pFont_->get_character_width(32)*fScalingFactor_;
        else
            return pFont_->get_character_width(uiChar)*fScalingFactor_;
    }
    else
        return 0.0f;
}

float text::get_character_kerning(char32_t uiChar1, char32_t uiChar2) const
{
    return pFont_->get_character_kerning(uiChar1, uiChar2)*fScalingFactor_;
}

void text::set_alignment(const text::alignment& mAlign)
{
    if (mAlign_ != mAlign)
    {
        mAlign_ = mAlign;
        notify_cache_dirty_();
    }
}

void text::set_vertical_alignment(const text::vertical_alignment& mVertAlign)
{
    if (mVertAlign_ != mVertAlign)
    {
        mVertAlign_ = mVertAlign;
        notify_cache_dirty_();
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
        notify_cache_dirty_();
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
        notify_cache_dirty_();
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
        notify_cache_dirty_();
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
        notify_cache_dirty_();
    }
}

bool text::is_word_wrap_enabled() const
{
    return bWordWrap_;
}

void text::enable_formatting(bool bFormatting)
{
    if (bFormatting != bFormattingEnabled_)
    {
        bFormattingEnabled_ = bFormatting;
        if (pRenderer_->is_vertex_cache_enabled())
            notify_cache_dirty_();
    }
}

void text::render(float fX, float fY) const
{
    if (!bReady_ || sUnicodeText_.empty())
        return;

    bool bUseVertexCache = pRenderer_->is_vertex_cache_enabled() &&
                           !pRenderer_->is_quad_batching_enabled();

    if ((bUseVertexCache && !pVertexCache_) || (bUseVertexCache && lQuadList_.empty()))
        bUpdateCache_ = true;

    update_();

    vector2f mOffset(round_to_pixel_(fX), round_to_pixel_(fY));

    if (pOutlineFont_)
    {
        const material* pMat = pOutlineFont_->get_texture().lock().get();
        if (bUseVertexCache && pOutlineVertexCache_)
        {
            pRenderer_->render_cache(pMat, *pOutlineVertexCache_, matrix4f::translation(mOffset));
        }
        else
        {
            std::vector<std::array<vertex,4>> lQuadsCopy = lOutlineQuadList_;
            for (auto& mQuad : lQuadsCopy)
            for (uint i = 0; i < 4; ++i)
            {
                mQuad[i].pos += mOffset;
            }

            pRenderer_->render_quads(pMat, lQuadsCopy);
        }
    }

    const material* pMat = pFont_->get_texture().lock().get();
    if (bUseVertexCache && pVertexCache_)
    {
        pRenderer_->render_cache(pMat, *pVertexCache_, matrix4f::translation(mOffset));
    }
    else
    {
        std::vector<std::array<vertex,4>> lQuadsCopy = lQuadList_;
        for (auto& mQuad : lQuadsCopy)
        for (uint i = 0; i < 4; ++i)
        {
            mQuad[i].pos += mOffset;

            if (!bFormattingEnabled_ || bForceColor_ || mQuad[i].col == color::EMPTY)
            {
                mQuad[i].col = mColor_;
            }
        }

        pRenderer_->render_quads(pMat, lQuadsCopy);
    }
}

void text::render_ex(float fX, float fY, float fRot, float fHScale, float fVScale) const
{
    if (!bReady_)
        return;

    update_();

    std::vector<std::array<vertex,4>> lQuadsCopy = lQuadList_;

    if (fRot != 0.0f)
    {
        float cost = cos(fRot);
        float sint = sin(fRot);

        for (auto& mQuad : lQuadsCopy)
        for (uint i = 0; i < 4; ++i)
        {
            float fX0 = mQuad[i].pos.x*fHScale;
            float fY0 = mQuad[i].pos.y*fVScale;
            mQuad[i].pos = vector2f(fX + fX0*cost - fY0*sint, fY + fX0*sint + fY0*cost);

            if (!bFormattingEnabled_ || bForceColor_ || mQuad[i].col == color::EMPTY)
            {
                mQuad[i].col = mColor_;
            }
        }
    }
    else
    {
        for (auto& mQuad : lQuadsCopy)
        for (uint i = 0; i < 4; ++i)
        {
            float fX0 = mQuad[i].pos.x*fHScale;
            float fY0 = mQuad[i].pos.y*fVScale;
            mQuad[i].pos = vector2f(fX + fX0, fY + fY0);

            if (!bFormattingEnabled_ || bForceColor_ || mQuad[i].col == color::EMPTY)
            {
                mQuad[i].col = mColor_;
            }
        }
    }

    const material* pMat = pFont_->get_texture().lock().get();
    pRenderer_->render_quads(pMat, lQuadsCopy);
}

bool get_format(utils::ustring::const_iterator& iterChar, utils::ustring::const_iterator iterEnd,
    text::format& mFormat)
{
    if (*iterChar == 'r')
    {
        mFormat.mColorAction = text::color_action::RESET;
    }
    else if (*iterChar == 'c')
    {
        mFormat.mColorAction = text::color_action::SET;

        auto fuReadTwo = [&](float& fOut)
        {
            ++iterChar;
            if (iterChar == iterEnd) return false;
            std::string sColorPart(2, '0');
            sColorPart[0] = *iterChar;
            ++iterChar;
            if (iterChar == iterEnd) return false;
            sColorPart[1] = *iterChar;
            fOut = utils::hex_to_uint(sColorPart)/255.0f;
            return true;
        };

        if (!fuReadTwo(mFormat.mColor.a)) return false;
        if (!fuReadTwo(mFormat.mColor.r)) return false;
        if (!fuReadTwo(mFormat.mColor.g)) return false;
        if (!fuReadTwo(mFormat.mColor.b)) return false;
    }

    return true;
}

struct line
{
    utils::ustring sCaption;
    float          fWidth = 0.0f;
};

void text::notify_cache_dirty_() const
{
    bUpdateCache_ = true;
}

float text::round_to_pixel_(float fValue) const
{
    return std::floor(fValue/fScalingFactor_)*fScalingFactor_;
}

void text::update_() const
{
    // #define DEBUG_LOG(msg) gui::out << (msg) << std::endl
    #define DEBUG_LOG(msg)

    if (!bReady_ || !bUpdateCache_) return;

    // Update the line list, read format tags, do word wrapping, ...
    std::vector<line>                lLineList;
    std::unordered_map<uint, format> lFormatList;

    DEBUG_LOG("     Get max line nbr");
    uint uiMaxLineNbr, uiCounter = 0;
    if (fBoxH_ != 0.0f && !std::isinf(fBoxH_))
    {
        if (fBoxH_ < get_line_height())
        {
            uiMaxLineNbr = 0;
        }
        else
        {
            float fRemaining = fBoxH_ - get_line_height();
            uiMaxLineNbr = 1 + static_cast<uint>(std::floor(fRemaining/(get_line_height()*fLineSpacing_)));
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

            DEBUG_LOG("     Line : '" + utils::unicode_to_utf8(sManualLine) + "'");

            // Make a temporary line array
            std::vector<line> lLines;

            line mLine;
            mLine.fWidth = 0.0f;

            std::unordered_map<uint, format> lTempFormatList;

            bool bDone = false;
            DEBUG_LOG("     Read chars");
            for (auto iterChar1 = sManualLine.begin(); iterChar1 != sManualLine.end(); ++iterChar1)
            {
                DEBUG_LOG("      char '" + utils::to_string(*iterChar1) + "'");
                DEBUG_LOG("      Read format");
                // Read format tags
                if (*iterChar1 == U'|' && bFormattingEnabled_)
                {
                    ++iterChar1;
                    if (iterChar1 == sManualLine.end()) break;

                    if (*iterChar1 != U'|')
                    {
                        text::format mFormat;
                        if (get_format(iterChar1, sManualLine.end(), mFormat))
                            lTempFormatList[uiCounter+mLine.sCaption.size()] = mFormat;

                        if (iterChar1 != sManualLine.end()) continue;
                        if (iterChar1 == sManualLine.end()) break;
                    }
                }

                DEBUG_LOG("      Get width");
                mLine.fWidth += get_character_width(*iterChar1) + fTracking_;
                auto iterNext = iterChar1 + 1;
                if (!utils::is_whitespace(*iterChar1) && iterNext != sManualLine.end() &&
                    !utils::is_whitespace(*iterNext))
                {
                    mLine.fWidth += get_character_kerning(*iterChar1, *iterNext);
                }

                mLine.sCaption += *iterChar1;

                if (round_to_pixel_(mLine.fWidth - fBoxW_) > 0)
                {
                    DEBUG_LOG("      Box break " + utils::to_string(mLine.fWidth) + " > " + utils::to_string(fBoxW_));
                    // Whoops, the line is too long...
                    if (mLine.sCaption.find_first_of(U" \t\n\r") != mLine.sCaption.npos && bWordWrap_)
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
                            if (utils::is_whitespace(*iterChar2))
                            {
                                if (!bLastWasWord || bRemoveStartingSpaces_ || mLine.fWidth - fErasedWidth > fBoxW_)
                                {
                                    mLine.fWidth -= fErasedWidth + get_character_width(U' ') + fTracking_;
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
                            while (iterChar2 != mLine.sCaption.end() && utils::is_whitespace(*iterChar2))
                            {
                                --uiCharToErase;
                                sErasedString.erase(0, 1);
                                ++iterChar2;
                            }
                        }

                        mLine.sCaption.erase(mLine.sCaption.size() - uiCharToErase, uiCharToErase);

                        lLines.push_back(mLine);
                        for (auto& mFormat : lTempFormatList)
                            lFormatList.insert(mFormat);

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
                            lLineList.push_back(mLine);
                            for (const auto& mFormat : lTempFormatList)
                                lFormatList[mFormat.first] = mFormat.second;

                            bDone = true;
                            break;
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
                            // Read cut format tags
                            if (bFormattingEnabled_)
                            {
                                while (iterTemp != iterChar1)
                                {
                                    if (*iterTemp == U'|')
                                    {
                                        ++iterTemp;
                                        if (iterTemp == iterChar1) break;

                                        if (*iterTemp != U'|')
                                        {
                                            text::format mFormat;
                                            if (get_format(iterTemp, iterChar1, mFormat))
                                            {
                                                lTempFormatList[uiCounter+mLine.sCaption.size()] = mFormat;
                                            }
                                        }
                                    }

                                    if (iterTemp != iterChar1)
                                        ++iterTemp;
                                }
                            }

                            // Look for the next word
                            while (iterChar1 != sManualLine.end() && utils::is_whitespace(*iterChar1))
                            {
                                ++iterChar1;
                            }

                            // Add the line
                            if (iterChar1 != sManualLine.end())
                            {
                                --iterChar1;
                                lLines.push_back(mLine);
                                uiCounter += mLine.sCaption.size();

                                for (auto& mFormat : lTempFormatList)
                                    lFormatList.insert(std::move(mFormat));

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

            if (bDone) break;

            DEBUG_LOG("     End");

            if (iterManual != lManualLineList.end() - 1)
                mLine.sCaption += U"\n";

            lLines.push_back(mLine);
            uiCounter += mLine.sCaption.size();

            for (auto& mFormat : lTempFormatList)
                lFormatList.insert(mFormat);

            lTempFormatList.clear();

            // Add the maximum number of line to this text
            for (auto& sLine : lLines)
            {
                lLineList.push_back(std::move(sLine));
                if (lLineList.size() == uiMaxLineNbr)
                {
                    bDone = true;
                    break;
                }
            }

            if (bDone) break;
            DEBUG_LOG("     .");
        }
    }

    uiNumLines_ = lLineList.size();

    lQuadList_.clear();
    lOutlineQuadList_.clear();

    if (!lLineList.empty())
    {
        if (fBoxW_ == 0.0f || std::isinf(fBoxW_))
        {
            fW_ = 0.0f;
            for (const auto& mLine : lLineList)
                fW_ = std::max(fW_, mLine.fWidth);
        }
        else
            fW_ = fBoxW_;

        fH_ = (1.0f + (lLineList.size() - 1)*fLineSpacing_)*get_line_height();

        float fX  = 0.0f, fY = 0.0f;
        float fX0 = 0.0f;

        if (fBoxW_ != 0.0f && !std::isinf(fBoxW_))
        {
            switch (mAlign_)
            {
                case alignment::LEFT :
                    fX0 = 0.0f;
                    break;
                case alignment::CENTER :
                    fX0 = round_to_pixel_(fBoxW_*0.5f);
                    break;
                case alignment::RIGHT :
                    fX0 = fBoxW_;
                    break;
            }
        }
        else
            fX0 = 0.0f;

        if (!std::isinf(fBoxH_))
        {
            switch (mVertAlign_)
            {
                case vertical_alignment::TOP :
                    fY = 0.0f;
                    break;
                case vertical_alignment::MIDDLE :
                    fY = round_to_pixel_((fBoxH_ - fH_)*0.5f);
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
                    fY = -round_to_pixel_(fH_*0.5f);
                    break;
                case vertical_alignment::BOTTOM :
                    fY = -fH_;
                    break;
            }
        }

        uiCounter = 0;
        color mColor = color::EMPTY;

        for (const auto& mLine : lLineList)
        {
            switch (mAlign_)
            {
                case alignment::LEFT :
                    fX = fX0;
                    break;
                case alignment::CENTER :
                    fX = fX0 - round_to_pixel_(mLine.fWidth*0.5f);
                    break;
                case alignment::RIGHT :
                    fX = fX0 - mLine.fWidth;
                    break;
            }

            for (auto iterChar : utils::range::iterator(mLine.sCaption))
            {
                // Format our text
                if (bFormattingEnabled_)
                {
                    auto lFormatIter = lFormatList.find(uiCounter);
                    if (lFormatIter != lFormatList.end())
                    {
                        const format& mFormat = lFormatIter->second;
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
                }

                if (pOutlineFont_)
                {
                    std::array<vertex,4> lVertexList = create_outline_letter_quad_(*iterChar);
                    for (uint i = 0; i < 4; ++i)
                    {
                        lVertexList[i].pos += vector2f(round_to_pixel_(fX), round_to_pixel_(fY));
                        lVertexList[i].col = color::BLACK;
                    }

                    lOutlineQuadList_.push_back(lVertexList);
                }

                std::array<vertex,4> lVertexList = create_letter_quad_(*iterChar);
                for (uint i = 0; i < 4; ++i)
                {
                    lVertexList[i].pos += vector2f(round_to_pixel_(fX), round_to_pixel_(fY));
                    lVertexList[i].col = mColor;
                }

                lQuadList_.push_back(lVertexList);

                ++uiCounter;

                if (*iterChar == U'\n') continue;

                float fKerning = 0.0f;
                auto iterNext = iterChar + 1;
                if (iterNext != mLine.sCaption.end() &&
                    !utils::is_whitespace(*iterNext) && !utils::is_whitespace(*iterChar))
                {
                    fKerning = get_character_kerning(*iterChar, *iterNext);
                }

                fX += get_character_width(*iterChar) + fKerning + fTracking_;
            }

            fY += get_line_height()*fLineSpacing_;
        }
    }
    else
    {
        fW_ = 0.0f;
        fH_ = 0.0f;
    }

    if (pRenderer_->is_vertex_cache_enabled() && !pRenderer_->is_quad_batching_enabled())
    {
        if (!pOutlineVertexCache_)
            pOutlineVertexCache_ = pRenderer_->create_vertex_cache(vertex_cache::type::QUADS);

        pOutlineVertexCache_->update(lOutlineQuadList_[0].data(), lOutlineQuadList_.size()*4);

        if (!pVertexCache_)
            pVertexCache_ = pRenderer_->create_vertex_cache(vertex_cache::type::QUADS);

        std::vector<std::array<vertex,4>> lQuadsCopy = lQuadList_;
        for (auto& mQuad : lQuadsCopy)
        for (uint i = 0; i < 4; ++i)
        {
            if (!bFormattingEnabled_ || bForceColor_ || mQuad[i].col == color::EMPTY)
            {
                mQuad[i].col = mColor_;
            }
        }

        pVertexCache_->update(lQuadsCopy[0].data(), lQuadsCopy.size()*4);
    }

    bUpdateCache_ = false;
}

std::array<vertex,4> text::create_letter_quad_(gui::font& mFont, char32_t uiChar) const
{
    quad2f mQuad = mFont.get_character_bounds(uiChar)*fScalingFactor_;
    mQuad.left = round_to_pixel_(mQuad.left);
    mQuad.top = round_to_pixel_(mQuad.top);
    mQuad.right = mQuad.left + round_to_pixel_(mQuad.right - mQuad.left);
    mQuad.bottom = mQuad.top + round_to_pixel_(mQuad.bottom - mQuad.top);

    std::array<vertex,4> lVertexList;
    lVertexList[0].pos = mQuad.top_left();
    lVertexList[1].pos = mQuad.top_right();
    lVertexList[2].pos = mQuad.bottom_right();
    lVertexList[3].pos = mQuad.bottom_left();

    quad2f mUVs = mFont.get_character_uvs(uiChar);
    lVertexList[0].uvs = mUVs.top_left();
    lVertexList[1].uvs = mUVs.top_right();
    lVertexList[2].uvs = mUVs.bottom_right();
    lVertexList[3].uvs = mUVs.bottom_left();

    return lVertexList;
}

std::array<vertex,4> text::create_letter_quad_(char32_t uiChar) const
{
    return create_letter_quad_(*pFont_, uiChar);
}

std::array<vertex,4> text::create_outline_letter_quad_(char32_t uiChar) const
{
    return create_letter_quad_(*pOutlineFont_, uiChar);
}

quad text::create_letter_quad(char32_t uiChar) const
{
    quad mOutput;
    mOutput.mat = pFont_->get_texture().lock();
    mOutput.v = create_letter_quad_(uiChar);

    return mOutput;
}

const std::array<vertex,4>& text::get_letter_quad(uint uiIndex) const
{
    update_();
    return lQuadList_[uiIndex];
}

}
}
