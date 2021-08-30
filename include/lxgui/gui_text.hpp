#ifndef LXGUI_GUI_TEXT_HPP
#define LXGUI_GUI_TEXT_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_color.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_sprite.hpp"

#include <lxgui/utils.hpp>
#include <lxgui/utils_string.hpp>

#include <vector>
#include <array>
#include <memory>
#include <limits>

namespace lxgui {
namespace gui
{
    class  renderer;
    class  vertex_cache;
    struct vertex;

    /// Used to draw some text on the screen
    /**
    */
    class text
    {
    public :

        enum class alignment
        {
            LEFT,
            CENTER,
            RIGHT
        };

        enum class vertical_alignment
        {
            TOP,
            MIDDLE,
            BOTTOM
        };

        /// Constructor.
        /** \param pRenderer The renderer instance to use
        *   \param pFont        The font to use for rendering
        *   \param pOutlineFont The font to use for outlines
        */
        explicit text(const renderer* pRenderer, std::shared_ptr<gui::font> pFont,
            std::shared_ptr<gui::font> pOutlineFont);

        /// Returns the height of one line (constant).
        /** \return The height of one line (constant)
        */
        float get_line_height() const;

        /// Set the scaling factor to use when rendering glyphs.
        /** \param fScalingFactor The scaling factor
        *   \note This defines the conversion factor between pixels (from the texture of the
        *         font object) and interface units. By default this is set to 1, but needs to
        *         be changed on high DPI systems.
        */
        void set_scaling_factor(float fScalingFactor);

        /// Returns the scaling factor used when rendering glyphs.
        /** \return The scaling factor used when rendering glyphs
        */
        float get_scaling_factor() const;

        /// Sets the text to render (unicode character set).
        /** \param sText The text to render
        *   \note This text can be formated :<br>
        *         - "|cAARRGGBB" : sets text color (hexadecimal).<br>
        *         - "|r" : sets text color to default.<br>
        *         - "||" : writes "|".
        */
        void set_text(const utils::ustring& sText);

        /// Returns the text that will be rendered (unicode character set).
        /** \return The text that will be rendered (unicode character set)
        *   \note This string contains format tags.
        */
        const utils::ustring& get_text() const;

        /// Sets this text's default color.
        /** \param mColor      The default color
        *   \param bForceColor 'true' to ignore color tags
        */
        void set_color(const color& mColor, bool bForceColor = false);

        /// Returns this text's default color.
        /** \return This text's default color
        */
        const color& get_color() const;

        /// Sets this text's transparency (alpha).
        /** \param fAlpha The new alpha value
        */
        void set_alpha(float fAlpha);

        /// Returns this text's transparency (alpha).
        /** \return This text's transparency (alpha)
        */
        float get_alpha() const;

        /// Sets the dimensions of the text box.
        /** \param fW The new witdh
        *   \param fH The new height
        *   \note To remove the text box, use 0.0f.
        */
        void set_dimensions(float fW, float fH);

        /// Sets the width of the text box.
        /** \param fBoxW The new witdh
        *   \note To remove it, use 0.0f.
        */
        void set_box_width(float fBoxW);

        /// Sets the height of the text box.
        /** \param fBoxH The new height
        *   \note To remove it, use 0.0f.
        */
        void set_box_height(float fBoxH);

        /// Returns the width of the rendered text.
        /** \return The width of the rendered text
        *   \note Takes the text box into account if any.
        */
        float get_width() const;

        /// Returns the height of the rendered text.
        /** \return The height of the rendered text
        *   \note Takes the text box into account if any.
        */
        float get_height() const;

        /// Returns the width of the text box.
        /** \return The width of the text box
        */
        float get_box_width() const;

        /// Returns the height of the text box.
        /** \return The height of the text box
        */
        float get_box_height() const;

        /// Returns the length of the text.
        /** \return The length of the text
        *   \note Ignores the text box, but not manual line jumps.
        */
        float get_text_width() const;

        /// Returns the number of text lines.
        /** \return The number of text lines
        */
        uint get_num_lines() const;

        /// Returns the lenght of a provided string.
        /** \param sString The string to measure
        *   \return The lenght of the provided string
        */
        float get_string_width(const std::string& sString) const;

        /// Returns the lenght of a provided string.
        /** \param sString The string to measure
        *   \return The lenght of the provided string
        */
        float get_string_width(const utils::ustring& sString) const;

        /// Returns the length of a single character.
        /** \param uiChar The character to measure
        *   \return The lenght of this character
        */
        float get_character_width(char32_t uiChar) const;

        /// Returns the kerning between two characters.
        /** \param uiChar1 The first character
        *   \param uiChar2 The second character
        *   \return The kerning between two characters
        *   \note Kerning is a letter spacing adjustment that makes the
        *         text look more condensed : is you stick an A near a V,
        *         you can reduce the space between the two letters, but not
        *         if you put two Vs side to side.
        */
        float get_character_kerning(char32_t uiChar1, char32_t uiChar2) const;

        /// Returns the height of the text.
        /** \return The height of one text
        *   \note Ignores the text box, but not manual line jumps.
        */
        float get_text_height() const;

        /// Sets text alignment.
        /** \param mAlign The new alignment
        */
        void set_alignment(const alignment& mAlign);

        /// Sets text vertical alignment.
        /** \param mVertAlign The new vertical alignment
        */
        void set_vertical_alignment(const vertical_alignment& mVertAlign);

        /// Returns the text aligment.
        /** \return The text alignment
        */
        const alignment& get_alignment() const;

        /// Returns the text aligment.
        /** \return The text alignment
        */
        const vertical_alignment& get_vertical_alignment() const;

        /// Sets this text's tracking.
        /** \param fTracking The new tracking
        *   \note Tracking is the space between each character. Default
        *         is 0.
        */
        void set_tracking(float fTracking);

        /// Returns this text's tracking.
        /** \return This text's tracking
        */
        float get_tracking() const;

        /// Sets this text's line spacing.
        /** \param fLineSpacing The new line spacing
        *   \note Line spacing is a coefficient that, multiplied by the
        *         height of a line, gives the space between two lines.
        *         Default is 1.5f.
        */
        void set_line_spacing(float fLineSpacing);

        /// Returns this text's line spacing.
        /** \return This text's line spacing
        */
        float get_line_spacing() const;

        /// Allows removal of a line's starting spaces.
        /** \param bRemoveStartingSpaces 'true' to remove them
        *   \note The text box does word wrapping : it cuts too long
        *         lines only between words. But sometimes, the rendered
        *         text must be cut between several spaces. By default,
        *         the algorithm puts cuted spaces at the beginning of
        *         the next line. You can change this behavior by setting
        *         this function to 'true'.
        */
        void set_remove_starting_spaces(bool bRemoveStartingSpaces);

        /// Checks if starting spaces removing is active.
        /** \return 'true' if starting spaces removing is active
        */
        bool get_remove_starting_spaces() const;

        /// Allows word wrap when the line is too long for the text box.
        /** \param bWrap        'true' to enable word wrap
        *   \param bAddEllipsis 'true' to put "..." at the end of a truncated line
        *   \note Enabled by default.
        */

        void enable_word_wrap(bool bWrap, bool bAddEllipsis);

        /// Checks if word wrap is enabled.
        /** \return 'true' if word wrap is enabled
        */
        bool is_word_wrap_enabled() const;

        /// Enables color formatting.
        /** \param bFormatting 'true' to enable color formatting
        *   \note Enabled by default.
        *   \note - "|cAARRGGBB" : sets text color (hexadecimal).<br>
        *         - "|r" : sets text color to default.<br>
        *         - "||" : writes "|".
        */
        void enable_formatting(bool bFormatting);

        /// Renders this text at the given position.
        /** \param fX The horizontal position
        *   \param fY The vertical position
        *   \note Must be called between renderer::begin() and
        *         renderer::end(). The parameters fX and fY refer to the position
        *         of the top-left corner of the text (if alignment is LEFT),
        *         the center-top of the text (if alignment is CENTER), or the
        *         the top-right corner of the text (if alignment is RIGHT).
        */
        void render(float fX, float fY) const;

        /// Deforms this text and render it on the current render target.
        /** \param fX      The horizontal position
        *   \param fY      The vertical position
        *   \param fRot    The rotation to apply (angle in radian)
        *   \param fHScale The horizontal scale to apply
        *   \param fVScale The vertical scale to apply
        *   \note Must be called between begin() and end(). The parameters fX and fY refer to the position
        *         of the top-left corner of the text (if alignment is LEFT),
        *         the center-top of the text (if alignment is CENTER), or the
        *         the top-right corner of the text (if alignment is RIGHT).
        */
        void render_ex(float fX, float fY,
                      float fRot,
                      float fHScale = 1.0f, float fVScale = 1.0f) const;

        /// Returns the quad for the letter at the provided index (position, texture coords, color).
        /** \param uiIndex The index of the letter (0: first letter);
        *                  must be less than get_text().size()
        *   \return The quad of the specified letter
        *   \note The vertex positions in the quad do not account for the rendering position
        *         provided to render(). The first letter always has its top-left corner as
        *         the position (0,0) (if left-aligned). This function may update the quad cache as
        *         needed.
        */
        const std::array<vertex,4>& get_letter_quad(uint uiIndex) const;

        /// Creates a quad that contains the provided character.
        /** \param uiChar The character to draw
        *   \note Uses this text's font texture.
        */
        quad create_letter_quad(char32_t uiChar) const;

    private :

        void update_() const;
        void notify_cache_dirty_() const;
        float round_to_pixel_(float fValue) const;
        std::array<vertex,4> create_letter_quad_(gui::font& mFont, char32_t uiChar) const;
        std::array<vertex,4> create_letter_quad_(char32_t uiChar) const;
        std::array<vertex,4> create_outline_letter_quad_(char32_t uiChar) const;

        const renderer* pRenderer_ = nullptr;

        bool  bReady_ = false;
        float fScalingFactor_ = 1.0f;
        float fTracking_ = 0.0f;
        float fLineSpacing_ = 1.5f;
        bool  bRemoveStartingSpaces_ = false;
        bool  bWordWrap_ = true;
        bool  bAddEllipsis_ = false;
        color mColor_ = color::WHITE;
        bool  bForceColor_ = false;
        float fAlpha_ = 1.0f;
        bool  bFormattingEnabled_ = false;
        float fBoxW_ = std::numeric_limits<float>::infinity();
        float fBoxH_ = std::numeric_limits<float>::infinity();
        alignment          mAlign_ = alignment::LEFT;
        vertical_alignment mVertAlign_ = vertical_alignment::MIDDLE;

        std::shared_ptr<font> pFont_;
        std::shared_ptr<font> pOutlineFont_;
        utils::ustring        sUnicodeText_;

        mutable bool  bUpdateCache_ = false;
        mutable float fW_ = 0.0f;
        mutable float fH_ = 0.0f;
        mutable uint  uiNumLines_ = 0u;

        mutable std::vector<std::array<vertex,4>> lQuadList_;
        mutable std::shared_ptr<vertex_cache>     pVertexCache_;
        mutable std::vector<std::array<vertex,4>> lOutlineQuadList_;
        mutable std::shared_ptr<vertex_cache>     pOutlineVertexCache_;
    };
}
}

#endif
