#ifndef GUI_TEXT_HPP
#define GUI_TEXT_HPP

#include <lxgui/utils.hpp>
#include "lxgui/gui_color.hpp"
#include "lxgui/gui_quad2.hpp"
#include "lxgui/gui_font.hpp"

#include <lxgui/utils_string.hpp>
#include <lxgui/utils_refptr.hpp>
#include <map>
#include <array>

namespace gui
{
    class  manager;
    class  sprite;
    struct vertex;

    /// Used to draw some text on the screen
    /**
    */
    class text
    {
    public :

        /// Contains a string that will be drawn on a line
        struct line
        {
            utils::ustring sCaption;
            float          fWidth;
        };

        enum color_action
        {
            COLOR_ACTION_NONE,
            COLOR_ACTION_SET,
            COLOR_ACTION_RESET
        };

        /// Contains information about the text at a given position
        struct format
        {
            format() : mColorAction(COLOR_ACTION_NONE)
            {}

            color        mColor;
            color_action mColorAction;
        };

        enum alignment
        {
            ALIGN_LEFT,
            ALIGN_CENTER,
            ALIGN_RIGHT
        };

        enum vertical_alignment
        {
            ALIGN_TOP,
            ALIGN_MIDDLE,
            ALIGN_BOTTOM
        };

        /// Holds the position, tex. coordinates and color of a character.
        struct letter
        {
            quad2f mQuad;
            quad2f mUVs;
            color  mColor;
            bool   bNoRender;
        };

        /// Constructor.
        /** \param sFileName The path to the .ttf file to use
        *   \param fSize    The size of the font (in point)
        */
        text(manager* pManager, const std::string& sFileName, float fSize);

        /// Destructor.
        ~text();

        /// Returns the path to the .ttf file.
        /** \return The path to the .ttf file
        */
        const std::string& get_font_name() const;

        /// Returns the size of the font.
        /** \return The size of the font
        */
        float get_font_size() const;

        /// Returns the height of one line (constant).
        /** \return The height of one line (constant)
        */
        float get_line_height() const;

        /// Sets the text to render.
        /** \param sText The text to render
        *   \note This text can be formated :<br>
        *         - "|cAARRGGBB" : sets text color (hexadecimal).<br>
        *         - "|r" : sets text color to default.<br>
        *         - "||" : writes "|".
        */
        void set_text(const std::string& sText);

        /// Returns the text that will be rendered.
        /** \return The text that will be rendered
        *   \note This string contains format tags.
        */
        const std::string& get_text() const;

        /// Returns the text that will be rendered (unicode character set).
        /** \return The text that will be rendered (unicode character set)
        *   \note This string contains format tags.
        */
        const utils::ustring& get_unicode_text() const;

        /// Sets this text's default color.
        /** \param mColor      The default color
        *   \param bForceColor 'true' to ignore color tags
        */
        void set_color(const color& mColor, bool bForceColor = false);

        /// Returns this text's default color.
        /** \return This text's default color
        */
        const color& get_color() const;

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
        float get_width();

        /// Returns the height of the rendered text.
        /** \return The height of the rendered text
        *   \note Takes the text box into account if any.
        */
        float get_height();

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
        /** \param fX The horizontal position of the top left corner
        *   \param fY The vertical position of the top left corner
        *   \note Must be called between spriteManager::begin() and
        *         spriteManager::end().
        */
        void render(float fX, float fY);

        /// updates this text's cache.
        /** \note Automatically done by render().<br>
        *         Only use this method if you need it to
        *         be Updated sooner.
        */
        void update();

        /// Returns the cached letters (position, size, texture coordinates)
        /** \return The cached letters
        *   \note updates the letter cache if needed.
        */
        const std::vector<letter>& get_letter_cache();

        /// Creates a sprite that contains the provided character.
        /** \param uiChar The character to draw
        *   \note Uses this text's font texture.
        */
        std::unique_ptr<sprite> create_sprite(char32_t uiChar) const;

    private :

        void update_lines_();
        void update_cache_();

        manager* pManager_;

        std::string sFileName_;

        bool  bReady_;
        float fSize_;
        float fTracking_;
        float fLineSpacing_;
        float fSpaceWidth_;
        bool  bRemoveStartingSpaces_;
        bool  bWordWrap_;
        bool  bAddEllipsis_;
        color mColor_;
        bool  bForceColor_;
        bool  bFormattingEnabled_;
        float fW_, fH_;
        float fX_, fY_;
        float fBoxW_, fBoxH_;

        std::string        sText_;
        utils::ustring     sUnicodeText_;
        alignment          mAlign_;
        vertical_alignment mVertAlign_;

        std::vector<line>      lLineList_;
        std::map<uint, format> lFormatList_;

        bool                bUpdateCache_;
        std::vector<letter> lLetterCache_;

        bool                              bUpdateQuads_;
        std::vector<std::array<vertex,4>> lQuadList_;
        std::unique_ptr<sprite>           pSprite_;

        utils::refptr<font> pFont_;
    };
}

#endif
