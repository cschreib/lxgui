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

// #define DEBUG_LOG(msg) gui::out << (msg) << std::endl
#define DEBUG_LOG(msg)

namespace lxgui {
namespace gui
{

namespace parser
{
    enum class color_action
    {
        NONE,
        SET,
        RESET
    };

    struct format
    {
        color        mColor = color::WHITE;
        color_action mColorAction = color_action::NONE;
    };

    struct texture
    {
        std::string               sFileName;
        float                     fWidth = 0.0f;
        float                     fHeight = 0.0f;
        std::shared_ptr<material> pMaterial;
    };

    using item = std::variant<char32_t, format, texture>;

    struct line
    {
        std::vector<item> lContent;
        float             fWidth = 0.0f;
    };

    std::vector<item> parse_string(const text& mText, const utils::ustring& sCaption,
        bool bFormattingEnabled)
    {
        std::vector<item> lContent;
        for (auto iterChar = sCaption.begin(); iterChar != sCaption.end(); ++iterChar)
        {
            // Read format tags
            if (*iterChar == U'|' && bFormattingEnabled)
            {
                ++iterChar;
                if (iterChar == sCaption.end()) break;

                if (*iterChar != U'|')
                {
                    if (*iterChar == U'r')
                    {
                        format mFormat;
                        mFormat.mColorAction = color_action::RESET;
                        lContent.push_back(mFormat);
                    }
                    else if (*iterChar == U'c')
                    {
                        format mFormat;
                        mFormat.mColorAction = color_action::SET;

                        auto mReadTwo = [&](float& fOut)
                        {
                            ++iterChar;
                            if (iterChar == sCaption.end()) return false;
                            utils::ustring sColorPart(2, U'0');
                            sColorPart[0] = *iterChar;
                            ++iterChar;
                            if (iterChar == sCaption.end()) return false;
                            sColorPart[1] = *iterChar;
                            fOut = utils::hex_to_uint(utils::unicode_to_utf8(sColorPart))/255.0f;
                            return true;
                        };

                        if (!mReadTwo(mFormat.mColor.a)) break;
                        if (!mReadTwo(mFormat.mColor.r)) break;
                        if (!mReadTwo(mFormat.mColor.g)) break;
                        if (!mReadTwo(mFormat.mColor.b)) break;

                        lContent.push_back(mFormat);
                    }
                    else if (*iterChar == U'T')
                    {
                        ++iterChar;

                        const auto uiBegin = iterChar - sCaption.begin();
                        const auto uiPos = sCaption.find(U"|t", uiBegin);
                        if (uiPos == sCaption.npos) break;

                        const std::string sExtracted = utils::unicode_to_utf8(
                            sCaption.substr(uiBegin, uiPos - uiBegin));

                        const auto lWords = utils::cut(sExtracted, ":");
                        if (!lWords.empty())
                        {
                            texture mTexture;
                            mTexture.pMaterial = mText.get_renderer()->create_material(lWords[0]);
                            if (lWords.size() == 2)
                            {
                                mTexture.fWidth = mTexture.fHeight = utils::string_to_float(lWords[1]);
                            }
                            else if (lWords.size() > 2)
                            {
                                mTexture.fWidth = utils::string_to_float(lWords[1]);
                                mTexture.fHeight = utils::string_to_float(lWords[2]);
                            }
                            else
                            {
                                mTexture.fWidth = mTexture.fHeight =
                                    std::numeric_limits<float>::quiet_NaN();
                            }

                            lContent.push_back(mTexture);
                        }

                        iterChar += sExtracted.size() + 1;
                    }

                    continue;
                }
            }

            // Add characters
            lContent.push_back(*iterChar);
        }

        return lContent;
    }

    bool is_whitespace(const item& mItem)
    {
        return std::visit([](const auto& mValue)
        {
            using type = std::decay_t<decltype(mValue)>;
            if constexpr (std::is_same_v<type, char32_t>)
            {
                return utils::is_whitespace(mValue);
            }
            else
            {
                return false;
            }
        }, mItem);
    }

    bool is_word(const item& mItem)
    {
        return std::visit([](const auto& mValue)
        {
            using type = std::decay_t<decltype(mValue)>;
            if constexpr (std::is_same_v<type, char32_t>)
            {
                return !utils::is_whitespace(mValue);
            }
            else
            {
                return false;
            }
        }, mItem);
    }

    bool is_character(const item& mItem)
    {
        return mItem.index() == 0u;
    }

    bool is_format(const item& mItem)
    {
        return mItem.index() == 1u;
    }

    bool is_character(const item& mItem, char32_t uiChar)
    {
        return mItem.index() == 0u && std::get<char32_t>(mItem) == uiChar;
    }

    float get_width(const text& mText, const item& mItem)
    {
        return std::visit([&](const auto& mValue)
        {
            using type = std::decay_t<decltype(mValue)>;
            if constexpr (std::is_same_v<type, char32_t>)
            {
                return mText.get_character_width(mValue);
            }
            else if constexpr (std::is_same_v<type, texture>)
            {
                if (std::isnan(mValue.fWidth))
                    return mText.get_line_height();
                else
                    return mValue.fWidth*mText.get_scaling_factor();
            }
            else
            {
                return 0.0f;
            }
        }, mItem);
    }

    float get_kerning(const text& mText, const item& mItem1, const item& mItem2)
    {
        return std::visit([&](const auto& mValue1)
        {
            using type1 = std::decay_t<decltype(mValue1)>;
            if constexpr (std::is_same_v<type1, char32_t>)
            {
                return std::visit([&](const auto& mValue2)
                {
                    using type2 = std::decay_t<decltype(mValue2)>;
                    if constexpr (std::is_same_v<type2, char32_t>)
                    {
                        return mText.get_character_kerning(mValue1, mValue2);
                    }
                    else
                    {
                        return 0.0f;
                    }
                }, mItem2);
            }
            else
            {
                return 0.0f;
            }
        }, mItem1);
    }

    float get_tracking(const text& mText, const item& mItem)
    {
        return std::visit([&](const auto& mValue)
        {
            using type = std::decay_t<decltype(mValue)>;
            if constexpr (std::is_same_v<type, char32_t>)
            {
                if (mValue != U'\n')
                    return mText.get_tracking();
                else
                    return 0.0f;
            }
            else
            {
                return 0.0f;
            }
        }, mItem);
    }

    std::pair<float,float> get_advance(const text& mText, std::vector<item>::const_iterator iterChar,
        std::vector<item>::const_iterator iterBegin)
    {
        float fAdvance = parser::get_width(mText, *iterChar);
        float fKerning = 0.0f;

        auto iterPrev = iterChar;
        while (iterPrev != iterBegin)
        {
            --iterPrev;
            if (parser::is_format(*iterPrev)) continue;

            fKerning = parser::get_tracking(mText, *iterChar);

            if (!parser::is_whitespace(*iterChar) && !parser::is_whitespace(*iterPrev))
                fKerning += parser::get_kerning(mText, *iterPrev, *iterChar);

            break;
        }

        return std::make_pair(fKerning, fAdvance);
    }

    float get_full_advance(const text& mText, std::vector<item>::const_iterator iterChar,
        std::vector<item>::const_iterator iterBegin)
    {
        const auto mAdvance = get_advance(mText, iterChar, iterBegin);
        return mAdvance.first + mAdvance.second;
    }

    float get_string_width(const text& mText, const std::vector<item>& lContent)
    {
        float fWidth = 0.0f;
        float fMaxWidth = 0.0f;

        for (auto iterChar : utils::range::iterator(lContent))
        {
            if (parser::is_character(*iterChar, U'\n'))
            {
                if (fWidth > fMaxWidth)
                    fMaxWidth = fWidth;

                fWidth = 0.0f;
            }
            else
            {
                fWidth += parser::get_full_advance(mText, iterChar, lContent.begin());
            }
        }

        if (fWidth > fMaxWidth)
            fMaxWidth = fWidth;

        return fMaxWidth;
    }
}

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

void text::set_alpha(float fAlpha)
{
    if (fAlpha == fAlpha_) return;

    fAlpha_ = fAlpha;
    if (pRenderer_->is_vertex_cache_enabled())
        notify_cache_dirty_();
}

float text::get_alpha() const
{
    return fAlpha_;
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

    return parser::get_string_width(*this,
        parser::parse_string(*this, sString, bFormattingEnabled_));
}

float text::get_character_width(char32_t uiChar) const
{
    if (bReady_)
    {
        if (uiChar == U'\t')
            return 4.0f*pFont_->get_character_width(U' ')*fScalingFactor_;
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
                mQuad[i].col.a *= fAlpha_;
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

            mQuad[i].col.a *= fAlpha_;
        }

        pRenderer_->render_quads(pMat, lQuadsCopy);
    }

    for (auto mQuad : lIconsList_)
    {
        for (uint i = 0; i < 4; ++i)
        {
            mQuad.v[i].pos += mOffset;
            mQuad.v[i].col.a *= fAlpha_;
        }

        pRenderer_->render_quad(mQuad);
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

            mQuad[i].col.a *= fAlpha_;
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

            mQuad[i].col.a *= fAlpha_;
        }
    }

    const material* pMat = pFont_->get_texture().lock().get();
    pRenderer_->render_quads(pMat, lQuadsCopy);
}

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
    if (!bReady_ || !bUpdateCache_) return;

    // Update the line list, read format tags, do word wrapping, ...
    std::vector<parser::line> lLineList;

    DEBUG_LOG("     Get max line nbr");
    uint uiMaxLineNbr;
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
            DEBUG_LOG("     Line : '" + utils::unicode_to_utf8(*iterManual) + "'");

            // Parse the line
            std::vector<parser::item> lParsedContent =
                parser::parse_string(*this, *iterManual, bFormattingEnabled_);

            // Make a temporary line array
            std::vector<parser::line> lLines;

            auto iterLineBegin = lParsedContent.begin();
            parser::line mLine;
            mLine.fWidth = 0.0f;

            bool bDone = false;
            for (auto iterChar1 = lParsedContent.begin(); iterChar1 != lParsedContent.end(); ++iterChar1)
            {
                DEBUG_LOG("      Get width");
                mLine.fWidth += parser::get_full_advance(*this, iterChar1, iterLineBegin);
                mLine.lContent.push_back(*iterChar1);

                if (round_to_pixel_(mLine.fWidth - fBoxW_) > 0)
                {
                    DEBUG_LOG("      Box break " + utils::to_string(mLine.fWidth) + " > " + utils::to_string(fBoxW_));

                    // Whoops, the line is too long...
                    auto mIterSpace = std::find_if(mLine.lContent.begin(), mLine.lContent.end(),
                        &parser::is_whitespace);

                    if (mIterSpace != mLine.lContent.end() && bWordWrap_)
                    {
                        DEBUG_LOG("       Spaced");
                        // There are several words on this line, we'll
                        // be able to put the last one on the next line
                        auto iterChar2 = iterChar1 + 1;
                        std::vector<parser::item> lErasedContent;
                        uint uiCharToErase = 0;
                        float fLastWordWidth = 0.0f;
                        bool bLastWasWord = false;
                        while (mLine.fWidth > fBoxW_ && iterChar2 != iterLineBegin)
                        {
                            --iterChar2;

                            if (parser::is_whitespace(*iterChar2))
                            {
                                if (!bLastWasWord || bRemoveStartingSpaces_ || mLine.fWidth - fLastWordWidth > fBoxW_)
                                {
                                    fLastWordWidth += parser::get_full_advance(*this, iterChar2, iterLineBegin);
                                    lErasedContent.insert(lErasedContent.begin(), *iterChar2);
                                    ++uiCharToErase;

                                    mLine.fWidth -= fLastWordWidth;
                                    fLastWordWidth = 0.0f;
                                }
                                else
                                    break;
                            }
                            else
                            {
                                fLastWordWidth += parser::get_full_advance(*this, iterChar2, iterLineBegin);
                                lErasedContent.insert(lErasedContent.begin(), *iterChar2);
                                ++uiCharToErase;

                                bLastWasWord = true;
                            }
                        }

                        if (bRemoveStartingSpaces_)
                        {
                            while (iterChar2 != iterChar1 + 1 && parser::is_whitespace(*iterChar2))
                            {
                                --uiCharToErase;
                                lErasedContent.erase(lErasedContent.begin());
                                ++iterChar2;
                            }
                        }

                        mLine.fWidth -= fLastWordWidth;
                        mLine.lContent.erase(mLine.lContent.end() - uiCharToErase, mLine.lContent.end());
                        lLines.push_back(mLine);

                        mLine.fWidth = parser::get_string_width(*this, lErasedContent);
                        mLine.lContent = lErasedContent;
                        iterLineBegin = iterChar1 - (mLine.lContent.size() - 1u);
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
                            // FIXME: this doesn't account for kerning between the "..." and prev char
                            float fWordWidth = get_string_width(U"...");
                            auto iterChar2 = iterChar1 + 1;
                            uint uiCharToErase = 0;
                            while (mLine.fWidth + fWordWidth > fBoxW_ && iterChar2 != iterLineBegin)
                            {
                                --iterChar2;
                                mLine.fWidth -= parser::get_full_advance(*this, iterChar2, iterLineBegin);
                                ++uiCharToErase;
                            }

                            DEBUG_LOG("       Char to erase : " + utils::to_string(uiCharToErase) + " / "
                                + utils::to_string(mLine.lContent.size()));

                            mLine.lContent.erase(mLine.lContent.end() - uiCharToErase, mLine.lContent.end());
                            mLine.lContent.push_back(U'.');
                            mLine.lContent.push_back(U'.');
                            mLine.lContent.push_back(U'.');
                            mLine.fWidth += fWordWidth;
                        }
                        else
                        {
                            DEBUG_LOG("       Truncate");
                            auto iterChar2 = iterChar1 + 1;
                            uint uiCharToErase = 0;
                            while (mLine.fWidth > fBoxW_ && iterChar2 != iterLineBegin)
                            {
                                --iterChar2;
                                mLine.fWidth -= parser::get_full_advance(*this, iterChar2, iterLineBegin);
                                ++uiCharToErase;
                            }

                            mLine.lContent.erase(mLine.lContent.end() - uiCharToErase, mLine.lContent.end());
                        }

                        if (!bWordWrap_)
                        {
                            DEBUG_LOG("       Display single line");
                            // Word wrap is disabled, so we can only display one line anyway.
                            lLineList.push_back(mLine);
                            bDone = true;
                            break;
                        }

                        // Add the line
                        lLines.push_back(mLine);
                        mLine.fWidth = 0.0f;
                        mLine.lContent.clear();

                        DEBUG_LOG("       Continue");

                        // Skip all following content (which we cannot display) until next whitespace
                        auto iterTemp = iterChar1;
                        iterChar1 = std::find_if(iterChar1, lParsedContent.end(),
                            &parser::is_whitespace);

                        if (iterChar1 == lParsedContent.end())
                            break;

                        // Apply the format tags that were cut
                        for (; iterTemp != iterChar1; ++iterTemp)
                        {
                            std::visit([&](const auto& mValue)
                            {
                                using type = std::decay_t<decltype(mValue)>;
                                if constexpr (std::is_same_v<type, parser::format>)
                                {
                                    mLine.lContent.push_back(mValue);
                                }
                            }, *iterTemp);
                        }

                        // Look for the next word
                        iterChar1 = std::find_if(iterChar1, lParsedContent.end(), &parser::is_word);
                        if (iterChar1 != lParsedContent.end())
                            break;

                        --iterChar1;
                        iterLineBegin = iterChar1;
                    }
                }
            }

            if (bDone) break;

            DEBUG_LOG("     End");

            lLines.push_back(mLine);

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
    lIconsList_.clear();

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

        std::vector<color> lColorStack;

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

            for (auto iterChar : utils::range::iterator(mLine.lContent))
            {
                const auto mAdvance = parser::get_advance(*this, iterChar, mLine.lContent.begin());

                fX += mAdvance.first;

                std::visit([&](const auto& mValue)
                {
                    using type = std::decay_t<decltype(mValue)>;
                    if constexpr (std::is_same_v<type, parser::format>)
                    {
                        switch (mValue.mColorAction)
                        {
                            case parser::color_action::SET :
                                lColorStack.push_back(mValue.mColor);
                                break;
                            case parser::color_action::RESET :
                                lColorStack.pop_back();
                                break;
                            default : break;
                        }
                    }
                    else if constexpr (std::is_same_v<type, parser::texture>)
                    {
                        float fTexWidth = 0.0f, fTexHeight = 0.0f;
                        if (std::isnan(mValue.fWidth))
                        {
                            fTexWidth = get_line_height();
                            fTexHeight = get_line_height();
                        }
                        else
                        {
                            fTexWidth = mValue.fWidth*get_scaling_factor();
                            fTexHeight = mValue.fHeight*get_scaling_factor();
                        }

                        fTexWidth  = round_to_pixel_(fTexWidth);
                        fTexHeight = round_to_pixel_(fTexHeight);

                        quad mIcon;
                        mIcon.mat = mValue.pMaterial;
                        mIcon.v[0].pos = vector2f(0.0f,      0.0f);
                        mIcon.v[1].pos = vector2f(fTexWidth, 0.0f);
                        mIcon.v[2].pos = vector2f(fTexWidth, fTexHeight);
                        mIcon.v[3].pos = vector2f(0.0f,      fTexHeight);
                        if (mIcon.mat)
                        {
                            mIcon.v[0].uvs = mIcon.mat->get_canvas_uv(vector2f(0.0f, 0.0f), true);
                            mIcon.v[1].uvs = mIcon.mat->get_canvas_uv(vector2f(1.0f, 0.0f), true);
                            mIcon.v[2].uvs = mIcon.mat->get_canvas_uv(vector2f(1.0f, 1.0f), true);
                            mIcon.v[3].uvs = mIcon.mat->get_canvas_uv(vector2f(0.0f, 1.0f), true);
                        }

                        for (uint i = 0; i < 4; ++i)
                        {
                            mIcon.v[i].pos += vector2f(round_to_pixel_(fX), round_to_pixel_(fY));
                        }

                        lIconsList_.push_back(mIcon);
                    }
                    else if constexpr (std::is_same_v<type, char32_t>)
                    {
                        if (pOutlineFont_)
                        {
                            std::array<vertex,4> lVertexList = create_outline_letter_quad_(mValue);
                            for (uint i = 0; i < 4; ++i)
                            {
                                lVertexList[i].pos += vector2f(round_to_pixel_(fX), round_to_pixel_(fY));
                                lVertexList[i].col = color::BLACK;
                            }

                            lOutlineQuadList_.push_back(lVertexList);
                        }

                        std::array<vertex,4> lVertexList = create_letter_quad_(mValue);
                        for (uint i = 0; i < 4; ++i)
                        {
                            lVertexList[i].pos += vector2f(round_to_pixel_(fX), round_to_pixel_(fY));
                            lVertexList[i].col = lColorStack.empty() ? color::EMPTY : lColorStack.back();
                        }

                        lQuadList_.push_back(lVertexList);
                    }
                }, *iterChar);

                fX += mAdvance.second;
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

            mQuad[i].col.a *= fAlpha_;
        }

        pVertexCache_->update(lQuadsCopy[0].data(), lQuadsCopy.size()*4);
    }

    bUpdateCache_ = false;
}

std::array<vertex,4> text::create_letter_quad_(gui::font& mFont, char32_t uiChar) const
{
    bounds2f mQuad = mFont.get_character_bounds(uiChar)*fScalingFactor_;
    mQuad.left = round_to_pixel_(mQuad.left);
    mQuad.top = round_to_pixel_(mQuad.top);
    mQuad.right = mQuad.left + round_to_pixel_(mQuad.right - mQuad.left);
    mQuad.bottom = mQuad.top + round_to_pixel_(mQuad.bottom - mQuad.top);

    std::array<vertex,4> lVertexList;
    lVertexList[0].pos = mQuad.top_left();
    lVertexList[1].pos = mQuad.top_right();
    lVertexList[2].pos = mQuad.bottom_right();
    lVertexList[3].pos = mQuad.bottom_left();

    bounds2f mUVs = mFont.get_character_uvs(uiChar);
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

const renderer* text::get_renderer() const
{
    return pRenderer_;
}

}
}
