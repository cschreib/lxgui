#ifndef LXGUI_GUI_FONT_STRING_HPP
#define LXGUI_GUI_FONT_STRING_HPP

#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_text.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

namespace lxgui::gui {

/**
 * \brief A #layered_region that can draw text on the screen.
 * \details This class holds a string and a reference to a font, which
 * is used to draw the string on the screen. The appearance of
 * the string can be changed (font, size, color, alignment, wrapping).
 * In addition, it is possible to change the color of a portion of
 * the string, for example to highlight a particular name.
 *
 * __Sizing.__ The #font_string class has a special property when it
 * comes to determining the size of its region on the screen, hence
 * how other object anchor to it, and how it anchors to other objects.
 * See the documentation for #region for more information on anchors.
 * While other regions must either have a fixed size or more than two
 * anchors constraining their size, the #font_string does not. If only
 * one anchor is specified, the width and height of the #font_string will
 * be determined by the area occupied by the displayed text, however long and
 * tall this may be. If the width is already constrained by the fixed size
 * or anchors, then the text will word wrap (if allowed) and the
 * #font_string's height will be as tall as the height of the wrapped text.
 * Finally, if both the width and height are constrained by fixed sizes or
 * anchors, the text will simply word wrap (if allowed) and be cut to fit
 * in the specified area.
 */
class font_string : public layered_region {
public:
    using base = layered_region;

    /// Constructor.
    explicit font_string(
        utils::control_block& block, manager& mgr, const region_core_attributes& attr);

    /**
     * \brief Prints all relevant information about this region in a string.
     * \param tab The offset to give to all lines
     * \return All relevant information about this region
     */
    std::string serialize(const std::string& tab) const override;

    /// Renders this region on the current render target.
    void render() const override;

    /**
     * \brief Copies a region's parameters into this font_string (inheritance).
     * \param obj The region to copy
     */
    void copy_from(const region& obj) override;

    /**
     * \brief Returns the name of the font file.
     * \return The name of the font file
     */
    const std::string& get_font_name() const;

    /**
     * \brief Returns the height of the font.
     * \return The height of the font
     */
    float get_font_height() const;

    /**
     * \brief Adds or remove the outline around the text.
     * \param is_outlined 'true' to enable the outline
     * \note The thickness of this outline is constant and
     * does not depend on the font's size.
     */
    void set_outlined(bool is_outlined);

    /**
     * \brief Check if this font_string is outlined.
     * \return 'true' if this font_string is outlined
     */
    bool is_outlined() const;

    /**
     * \brief Returns the horizontal alignment behavior.
     * \return The horizontal alignment behavior
     */
    alignment_x get_alignment_x() const;

    /**
     * \brief Returns the vertical alignment behavior.
     * \return The vertical alignment behavior
     */
    alignment_y get_alignment_y() const;

    /**
     * \brief Returns this font_string's shadow color.
     * \return This font_string's shadow color
     */
    const color& get_shadow_color() const;

    /**
     * \brief Returns this font_string's shadow offset.
     * \return This font_string's shadow offset
     * \note Contains (X, Y) offset.
     */
    const vector2f& get_shadow_offset() const;

    /**
     * \brief Returns this font_string's offset.
     * \return This font_string's offset
     * \note Contains (X, Y) offset.
     */
    const vector2f& get_offset() const;

    /**
     * \brief Returns the space between each letter.
     * \return The space between each letter
     */
    float get_spacing() const;

    /**
     * \brief Returns the space between each line as a fraction of the font height.
     * \return The space between each line as a fraction of the font height (default is 1).
     */
    float get_line_spacing() const;

    /**
     * \brief Returns the text color.
     * \return The text color
     */
    const color& get_text_color() const;

    /**
     * \brief Sets this font_string's font (file and size).
     * \param font_name The file path to the .ttf file
     * \param height The font height
     */
    void set_font(const std::string& font_name, float height);

    /**
     * \brief Sets this font_string's horizontal alignment behavior.
     * \param align_x The horizontal alignment behavior
     */
    void set_alignment_x(alignment_x align_x);

    /**
     * \brief Sets this font_string's vertical alignment behavior.
     * \param align_y The vertical alignment behavior
     */
    void set_alignment_y(alignment_y align_y);

    /**
     * \brief Sets this font_string's shadow color.
     * \param shadow_color The shadow color
     */
    void set_shadow_color(const color& shadow_color);

    /**
     * \brief Sets this font_string's shadow offset.
     * \param shadow_offset Offset
     * \note Contains (X, Y) offset.
     */
    void set_shadow_offset(const vector2f& shadow_offset);

    /**
     * \brief Sets this font_string's offset.
     * \param offset Offset
     * \note Contains (X, Y) offset.
     */
    void set_offset(const vector2f& offset);

    /**
     * \brief Sets the space between each letter.
     * \param spacing The space between each letter
     */
    void set_spacing(float spacing);

    /**
     * \brief Sets the space between each line as a fraction of the font height.
     * \param line_spacing The space between each line, as a relative factor of the font height
     * \note A line spacing of 1 is the default and results in fairly dense text. To increase
     * the space between lines, set the line spacing to a larger value, for example 1.5
     * results in 50% more space.
     */
    void set_line_spacing(float line_spacing);

    /**
     * \brief Sets the text color.
     * \param text_color The text color
     */
    void set_text_color(const color& text_color);

    /**
     * \brief Checks if large text is truncated or wrapped.
     * \return 'true' if large text is truncated
     * \note See set_non_space_wrap_enabled for more information.
     */
    bool is_non_space_wrap_enabled() const;

    /**
     * \brief Returns the height of the string if no format or wrapping is applied.
     * \return The height of the string if no format or wrapping is applied
     */
    float get_string_height() const;

    /**
     * \brief Returns the width of the string if no format or wrapping is applied.
     * \return The width of the string if no format or wrapping is applied
     */
    float get_string_width() const;

    /**
     * \brief Returns the width of a string if no format or wrapping is applied.
     * \param content The string for which to calculate the width
     * \return The width of a string if no format or wrapping is applied
     */
    float get_string_width(const utils::ustring& content) const;

    /**
     * \brief Returns the rendered text (with format tags).
     * \return The rendered text (with format tags)
     */
    const utils::ustring& get_text() const;

    /**
     * \brief Sets whether large text without whitespace is truncated or wrapped.
     * \param enabled 'true' to wrap, 'false' to truncate
     * \note This applies to large chunks of text with no spaces. When truncated, "..." is appended
     * at the line's end. Else, the word is cut and continues on the next line.
     */
    void set_non_space_wrap_enabled(bool enabled);

    /**
     * \brief Allows large text without whitespace to wrap.
     * \see set_non_space_wrap_enabled
     */
    void enable_non_space_wrap() {
        set_non_space_wrap_enabled(true);
    }

    /**
     * \brief Does not allow large text without whitespace to wrap.
     * \see set_non_space_wrap_enabled
     */
    void disable_non_space_wrap() {
        set_non_space_wrap_enabled(false);
    }

    /**
     * \brief Checks if this font_string draws a shadow under its text.
     * \return 'true' if this font_string draws a shadow under its text
     */
    bool is_shadow_enabled() const;

    /**
     * \brief Sets whether this font_string should draw a shadow under its text.
     * \param enabled 'true' to enable shadow
     */
    void set_shadow_enabled(bool enabled);

    /**
     * \brief Makes this font_string draw a shadow under its text.
     * \see set_shadow_enabled
     */
    void enable_shadow() {
        set_shadow_enabled(true);
    }

    /**
     * \brief Makes this font_string draw a shadow under its text.
     * \see set_shadow_enabled
     */
    void disable_shadow() {
        set_shadow_enabled(false);
    }

    /**
     * \brief Enables/disables word wrap.
     * \param is_word_wrap_enabled 'true' to enable word wrap
     * \note Enabled by default.
     */
    void set_word_wrap_enabled(bool is_word_wrap_enabled);

    /**
     * \brief Enables word wrap.
     */
    void enable_word_wrap() {
        set_word_wrap_enabled(true);
    }

    /**
     * \brief Disables word wrap.
     */
    void disable_word_wrap() {
        set_word_wrap_enabled(false);
    }

    /**
     * \brief Checks if word wrap is enabled.
     * \return 'true' if word wrap is enabled
     */
    bool is_word_wrap_enabled() const;

    /**
     * \brief Sets whether to show an ellipsis "..." if words don't fit in the text box.
     * \param add_ellipsis 'true' to put "..." at the end of a truncated line
     * \note Disabled by default.
     */
    void set_word_ellipsis_enabled(bool add_ellipsis);

    /**
     * \brief Show an ellipsis "..." if words don't fit in the text box.
     */
    void enable_word_ellipsis() {
        set_word_ellipsis_enabled(true);
    }

    /**
     * \brief Do not show an ellipsis "..." if words don't fit in the text box.
     */
    void disable_word_ellipsis() {
        set_word_ellipsis_enabled(false);
    }

    /**
     * \brief Checks if word ellipsis is enabled.
     * \return 'true' if word ellipsis is enabled
     */
    bool is_word_ellipsis_enabled() const;

    /**
     * \brief Enables color formatting.
     * \param formatting 'true' to enable color formatting
     * \note Enabled by default. See text::set_formatting_enabled().
     */
    void set_formatting_enabled(bool formatting);

    /**
     * \brief Enables color formatting.
     * \see set_formatting_enabled
     */
    void enable_formatting() {
        set_formatting_enabled(true);
    }

    /**
     * \brief Disables color formatting.
     * \see set_formatting_enabled
     */
    void disable_formatting() {
        set_formatting_enabled(false);
    }

    /**
     * \brief Checks if color formatting is enabled.
     * \return 'true' if color formatting is enabled
     */
    bool is_formatting_enabled() const;

    /**
     * \brief Sets the rendered text.
     * \param content The rendered text
     * \note See text::set_text for more information about formatting.
     */
    void set_text(const utils::ustring& content);

    /// Tells this region that the global interface scaling factor has changed.
    void notify_scaling_factor_updated() override;

    /**
     * \brief Parses data from a layout_node.
     * \param node The layout node
     */
    void parse_layout(const layout_node& node) override;

    /**
     * \brief Returns the text used to render this font_string.
     * \return The text used to render this font_string
     */
    text* get_text_object();

    /**
     * \brief Returns the text used to render this font_string.
     * \return The text used to render this font_string
     */
    const text* get_text_object() const;

    /// Registers this region class to the provided Lua state
    static void register_on_lua(sol::state& lua);

    static constexpr const char* class_name = "FontString";

private:
    void parse_attributes_(const layout_node& node) override;
    void parse_shadow_node_(const layout_node& node);

    const std::vector<std::string>& get_type_list_() const override;

    void create_text_object_();

    void update_borders_() override;

    std::unique_ptr<text> text_;

    utils::ustring content_;
    std::string    font_name_;
    float          height_ = 0.0f;

    float       spacing_      = 0.0f;
    float       line_spacing_ = 1.0f;
    alignment_x align_x_      = alignment_x::center;
    alignment_y align_y_      = alignment_y::middle;
    vector2f    offset_       = vector2f::zero;

    bool  is_outlined_            = false;
    bool  non_space_wrap_enabled_ = false;
    bool  word_wrap_enabled_      = true;
    bool  ellipsis_enabled_       = true;
    bool  formatting_enabled_     = true;
    color text_color_             = color::white;

    bool     is_shadow_enabled_ = false;
    color    shadow_color_      = color::black;
    vector2f shadow_offset_     = vector2f::zero;
};

} // namespace lxgui::gui

#endif
