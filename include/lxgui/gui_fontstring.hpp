#ifndef LXGUI_GUI_FONTSTRING_HPP
#define LXGUI_GUI_FONTSTRING_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_text.hpp"

namespace lxgui {
namespace gui
{
    /// A #layered_region that can draw text on the screen.
    /** This class holds a string and a reference to a font, which
    *   is used to draw the string on the screen. The appearance of
    *   the string can be changed (font, size, color, alignment, wrapping).
    *   In addition, it is possible to change the color of a portion of
    *   the string, for example to highlight a particular name.
    *
    *   __Sizing.__ The #font_string class has a special property when it
    *   comes to determining the size of its region on the screen, hence
    *   how other object anchor to it, and how it anchors to other objects.
    *   See the documentation for #uiobject for more information on anchors.
    *   While other uiobjects must either have a fixed size or more than two
    *   anchors constraining their size, the #font_string does not. If only
    *   one anchor is specified, the width and height of the #font_string will
    *   be determined by the area occupied by the displayed text, however long and
    *   tall this may be. If the width is already constrained by the fixed size
    *   or anchors, then the text will word wrap (if allowed) and the
    *   #font_string's height will be as tall as the height of the wrapped text.
    *   Finally, if both the width and height are constrained by fixed sizes or
    *   anchors, the text will simply word wrap (if allowed) and be cut to fit
    *   in the specified area.
    */
    class font_string : public layered_region
    {
    public :

        /// Constructor.
        explicit font_string(manager* pManager);

        /// Prints all relevant information about this widget in a string.
        /** \param sTab The offset to give to all lines
        *   \return All relevant information about this widget
        */
        std::string serialize(const std::string& sTab) const override;

        /// Renders this widget on the current render target.
        void render() override;

        /// Copies an uiobject's parameters into this font_string (inheritance).
        /** \param pObj The uiobject to copy
        */
        void copy_from(uiobject* pObj) override;

        /// Returns the name of the font file.
        /** \return The name of the font file
        */
        const std::string& get_font_name() const;

        /// Returns the heigh of the font.
        /** \return The heigh of the font
        */
        float get_font_height() const;

        /// Adds or remove the outline around the text.
        /** \param bIsOutlined 'true' to enable the outline
        *   \note The thickness of this outline is constant and
        *         does not depend on the font's size.
        */
        void set_outlined(bool bIsOutlined);

        /// Check if this font_string is outlined.
        /** \return 'true' if this font_string is outlined
        */
        bool is_outlined() const;

        /// Returns the horizontal alignment behavior.
        /** \return The horizontal alignment behavior
        */
        text::alignment get_justify_h() const;

        /// Returns the vertical alignment behavior.
        /** \return The vertical alignment behavior
        */
        text::vertical_alignment get_justify_v() const;

        /// Returns this font_string's shadow color.
        /** \return This font_string's shadow color
        */
        const color& get_shadow_color() const;

        /// Returns this font_string's shadow offsets.
        /** \return This font_string's shadow offsets
        *   \note Contains (X, Y) offsets.
        */
        vector2f get_shadow_offsets() const;

        /// Returns this font_string's shadow X offset.
        /** \return This font_string's shadow X offset
        */
        float get_shadow_x_offset() const;

        /// Returns this font_string's shadow Y offset.
        /** \return This font_string's shadow Y offset
        */
        float get_shadow_y_offset() const;

        /// Returns this font_string's offsets.
        /** \return This font_string's offsets
        *   \note Contains (X, Y) offsets.
        */
        vector2f get_offsets() const;

        /// Returns the space between each letter.
        /** \return The space between each letter
        */
        float get_spacing() const;

        /// Returns the text color.
        /** \return The text color
        */
        const color& get_text_color() const;

        /// Sets this font_string's font (file and size).
        /** \param sFontName The file path to the .ttf file
        *   \param fHeight   The font height
        */
        void set_font(const std::string& sFontName, float fHeight);

        /// Sets this font_string's horizontal aligment behavior.
        /** \param mJustifyH The horizontal alignment behavior
        */
        void set_justify_h(text::alignment mJustifyH);

        /// Sets this font_string's vertical aligment behavior.
        /** \param mJustifyV The vertical alignment behavior
        */
        void set_justify_v(text::vertical_alignment mJustifyV);

        /// Sets this font_string's shadow color.
        /** \param mShadowColor The shadow color
        */
        void set_shadow_color(const color& mShadowColor);

        /// Sets this font_string's shadow offsets.
        /** \param fShadowXOffset The horizontal offset
        *   \param fShadowYOffset The vertical offset
        */
        void set_shadow_offsets(float fShadowXOffset, float fShadowYOffset);

        /// Sets this font_string's shadow offsets.
        /** \param mShadowOffsets Offsets
        *   \note Contains (X, Y) offsets.
        */
        void set_shadow_offsets(const vector2f& mShadowOffsets);

        /// Sets this font_string's offsets.
        /** \param fXOffset The horizontal offset
        *   \param fYOffset The vertical offset
        */
        void set_offsets(float fXOffset, float fYOffset);

        /// Sets this font_string's offsets.
        /** \param mOffsets Offsets
        *   \note Contains (X, Y) offsets.
        */
        void set_offsets(const vector2f& mOffsets);

        /// Sets the space between each letter.
        /** \param fSpacing The space between each letter
        */
        void set_spacing(float fSpacing);

        /// Sets the text color.
        /** \param mTextColor The text color
        */
        void set_text_color(const color& mTextColor);

        /// Checks is large text is truncated or wrapped.
        /** \return 'true' if larget text is truncated
        *   \note See set_non_space_wrap for more infos.
        */
        bool can_non_space_wrap() const;

        /// Returns the height of the string if no format or wrapping is applied.
        /** \return The height of the string if no format or wrapping is applied
        */
        float get_string_height() const;

        /// Returns the width of the string if no format or wrapping is applied.
        /** \return The width of the string if no format or wrapping is applied
        */
        float get_string_width() const;

        /// Returns the width of a string if no format or wrapping is applied.
        /** \param sString The string for which to calculate the width
        *   \return The width of a string if no format or wrapping is applied
        */
        float get_string_width(const utils::ustring& sString) const;

        /// Returns the rendered text (with format tags).
        /** \return The rendered text (with format tags)
        */
        const utils::ustring& get_text() const;

        /// Sets whether large text is truncated or wrapped.
        /** \param bCanNonSpaceWrap 'true' to truncate the text
        *   \note This applies to large chunks of text with no
        *         spaces. When truncated, "..." is appended at
        *         the line's end. Else, the "word" is cut and
        *         continues on the next line.
        */
        void set_non_space_wrap(bool bCanNonSpaceWrap);

        /// Checks if this font_string draws a shadow under its text.
        /** \return 'true' if this font_string draws a shadow under its text
        */
        bool has_shadow() const;

        /// Sets whether this font_string should draw a shadow under its text.
        /** \param bHasShadow 'true' to enable shadow
        */
        void set_shadow(bool bHasShadow);

        /// Enables word wrap.
        /** \param bCanWordWrap 'true' to enable word wrap
        *   \param bAddEllipsis 'true' to put "..." at the end of a truncated line
        *   \note Enabled by default.
        */
        void set_word_wrap(bool bCanWordWrap, bool bAddEllipsis);

        /// Checks if word wrap is enabled.
        /** \return 'true' if word wrap is enabled
        */
        bool can_word_wrap() const;

        /// Enables color formatting.
        /** \param bFormatting 'true' to enable color formatting
        *   \note Enabled by default. See text::enable_formatting().
        */
        void enable_formatting(bool bFormatting);

        /// Checks if color formatting is enabled.
        /** \return 'true' if color formatting is enabled
        */
        bool is_formatting_enabled() const;

        /// Sets the rendered text.
        /** \param sText The rendered text
        *   \note See text::set_text for more infos about formatting.
        */
        void set_text(const utils::ustring& sText);

        /// Tells this widget that the global interface scaling factor has changed.
        void notify_scaling_factor_updated() override;

        /// Creates the associated Lua glue.
        void create_glue() override;

        /// Parses data from an xml::block.
        /** \param pBlock The font_string's xml::block
        */
        void parse_block(xml::block* pBlock) override;

        /// Returns the text used to render this fontString.
        /** \return The text used to render this fontString
        */
        text* get_text_object();

        /// Returns the text used to render this fontString.
        /** \return The text used to render this fontString
        */
        const text* get_text_object() const;

        /// Registers this widget to the provided lua::state
        static void register_glue(lua::state& mLua);

        static constexpr const char* CLASS_NAME = "FontString";

    private :

        void parse_attributes_(xml::block* pBlock) override;
        void parse_shadow_block_(xml::block* pBlock);

        void create_text_object_();

        void update_borders_() const override;

        mutable std::unique_ptr<text> pText_;

        utils::ustring sText_;
        std::string    sFontName_;
        float          fHeight_ = 0.0f;

        float                    fSpacing_ = 0.0f;
        text::alignment          mJustifyH_ = text::alignment::CENTER;
        text::vertical_alignment mJustifyV_ = text::vertical_alignment::MIDDLE;
        float                    fXOffset_ = 0.0f;
        float                    fYOffset_ = 0.0f;

        bool  bIsOutlined_ = false;
        bool  bCanNonSpaceWrap_ = false;
        bool  bCanWordWrap_ = true;
        bool  bAddEllipsis_ = true;
        bool  bFormattingEnabled_ = true;
        color mTextColor_ = color::WHITE;

        bool  bHasShadow_ = false;
        color mShadowColor_ = color::BLACK;
        float fShadowXOffset_ = 0.0f;
        float fShadowYOffset_ = 0.0f;
    };

    /** \cond NOT_REMOVE_FROM_DOC
    */

    class lua_font_string : public lua_layered_region
    {
    public :

        explicit lua_font_string(lua_State* pLua);
        font_string* get_object() { return static_cast<font_string*>(pObject_); }

        int _get_font(lua_State*);
        int _get_justify_h(lua_State*);
        int _get_justify_v(lua_State*);
        int _get_shadow_color(lua_State*);
        int _get_shadow_offset(lua_State*);
        int _get_spacing(lua_State*);
        int _get_text_color(lua_State*);
        int _set_font(lua_State*);
        int _set_justify_h(lua_State*);
        int _set_justify_v(lua_State*);
        int _set_shadow_color(lua_State*);
        int _set_shadow_offset(lua_State*);
        int _set_spacing(lua_State*);
        int _set_text_color(lua_State*);
        int _can_non_space_wrap(lua_State*);
        int _can_word_wrap(lua_State*);
        int _enable_formatting(lua_State*);
        int _get_string_height(lua_State*);
        int _get_string_width(lua_State*);
        int _get_text(lua_State*);
        int _is_formatting_enabled(lua_State*);
        int _set_non_space_wrap(lua_State*);
        int _set_text(lua_State*);
        int _set_word_wrap(lua_State*);

        static const char className[];
        static const char* classList[];
        static lua::lunar_binding<lua_font_string> methods[];
    };

    /** \endcond
    */
}
}

#endif
