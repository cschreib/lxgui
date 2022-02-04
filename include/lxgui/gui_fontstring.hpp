#ifndef LXGUI_GUI_FONTSTRING_HPP
#define LXGUI_GUI_FONTSTRING_HPP

#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_text.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

namespace lxgui::gui {

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
 *   See the documentation for #region for more information on anchors.
 *   While other regions must either have a fixed size or more than two
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
class font_string : public layered_region {
    using base = layered_region;

public:
    /// Constructor.
    explicit font_string(utils::control_block& mBlock, manager& mManager);

    /// Prints all relevant information about this region in a string.
    /** \param sTab The offset to give to all lines
     *   \return All relevant information about this region
     */
    std::string serialize(const std::string& sTab) const override;

    /// Renders this region on the current render target.
    void render() const override;

    /// Copies a region's parameters into this font_string (inheritance).
    /** \param mObj The region to copy
     */
    void copy_from(const region& mObj) override;

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
    alignment_x get_alignment_x() const;

    /// Returns the vertical alignment behavior.
    /** \return The vertical alignment behavior
     */
    alignment_y get_alignment_y() const;

    /// Returns this font_string's shadow color.
    /** \return This font_string's shadow color
     */
    const color& get_shadow_color() const;

    /// Returns this font_string's shadow offset.
    /** \return This font_string's shadow offset
     *   \note Contains (X, Y) offset.
     */
    const vector2f& get_shadow_offset() const;

    /// Returns this font_string's offset.
    /** \return This font_string's offset
     *   \note Contains (X, Y) offset.
     */
    const vector2f& get_offset() const;

    /// Returns the space between each letter.
    /** \return The space between each letter
     */
    float get_spacing() const;

    /// Returns the space between each line as a fraction of the font height.
    /** \return The space between each line as a fraction of the font height (default is 1).
     */
    float get_line_spacing() const;

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
    /** \param mAlignX The horizontal alignment behavior
     */
    void set_alignment_x(alignment_x mAlignX);

    /// Sets this font_string's vertical aligment behavior.
    /** \param mAlignY The vertical alignment behavior
     */
    void set_alignment_y(alignment_y mAlignY);

    /// Sets this font_string's shadow color.
    /** \param mShadowColor The shadow color
     */
    void set_shadow_color(const color& mShadowColor);

    /// Sets this font_string's shadow offset.
    /** \param mShadowOffset Offset
     *   \note Contains (X, Y) offset.
     */
    void set_shadow_offset(const vector2f& mShadowOffset);

    /// Sets this font_string's offset.
    /** \param mOffset Offset
     *   \note Contains (X, Y) offset.
     */
    void set_offset(const vector2f& mOffset);

    /// Sets the space between each letter.
    /** \param fSpacing The space between each letter
     */
    void set_spacing(float fSpacing);

    /// Sets the space between each line as a fraction of the font height.
    /** \param fLineSpacing The space between each line, as a relative factor of the font height
     *   \note A line spacing of 1 is the default and results in fairly dense text. To increase
     *         the space between lines, set the line spacing to a larger value, for example 1.5
     *         results in 50% more space.
     */
    void set_line_spacing(float fLineSpacing);

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

    /// Tells this region that the global interface scaling factor has changed.
    void notify_scaling_factor_updated() override;

    /// Creates the associated Lua glue.
    void create_glue() override;

    /// Parses data from a layout_node.
    /** \param mNode The layout node
     */
    void parse_layout(const layout_node& mNode) override;

    /// Returns the text used to render this font_string.
    /** \return The text used to render this font_string
     */
    text* get_text_object();

    /// Returns the text used to render this font_string.
    /** \return The text used to render this font_string
     */
    const text* get_text_object() const;

    /// Registers this region class to the provided Lua state
    static void register_on_lua(sol::state& mLua);

    static constexpr const char* CLASS_NAME = "FontString";

private:
    void parse_attributes_(const layout_node& mNode) override;
    void parse_shadow_node_(const layout_node& mNode);

    void create_text_object_();

    void update_borders_() override;

    std::unique_ptr<text> pText_;

    utils::ustring sText_;
    std::string    sFontName_;
    float          fHeight_ = 0.0f;

    float       fSpacing_     = 0.0f;
    float       fLineSpacing_ = 1.0f;
    alignment_x mAlignX_      = alignment_x::CENTER;
    alignment_y mAlignY_      = alignment_y::MIDDLE;
    vector2f    mOffset_      = vector2f::ZERO;

    bool  bIsOutlined_        = false;
    bool  bCanNonSpaceWrap_   = false;
    bool  bCanWordWrap_       = true;
    bool  bAddEllipsis_       = true;
    bool  bFormattingEnabled_ = true;
    color mTextColor_         = color::WHITE;

    bool     bHasShadow_    = false;
    color    mShadowColor_  = color::BLACK;
    vector2f mShadowOffset_ = vector2f::ZERO;
};

} // namespace lxgui::gui

#endif
