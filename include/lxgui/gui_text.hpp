#ifndef LXGUI_GUI_TEXT_HPP
#define LXGUI_GUI_TEXT_HPP

#include "lxgui/gui_color.hpp"
#include "lxgui/gui_font.hpp"
#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/utils_maths.hpp"
#include "lxgui/utils_string.hpp"

#include <array>
#include <limits>
#include <memory>
#include <vector>

namespace lxgui::gui {

class renderer;
class vertex_cache;
struct vertex;

enum class alignment_x { left, center, right };

enum class alignment_y { top, middle, bottom };

/// Used to draw some text on the screen
/**
 */
class text {
public:
    /// Constructor.
    /** \param mRenderer    The renderer instance to use
     *   \param pFont        The font to use for rendering
     *   \param pOutlineFont The font to use for outlines
     */
    explicit text(
        renderer&                  m_renderer,
        std::shared_ptr<gui::font> p_font,
        std::shared_ptr<gui::font> p_outline_font);

    /// Non-copiable
    text(const text&) = delete;

    /// Non-movable
    text(text&&) = delete;

    /// Non-copiable
    text& operator=(const text&) = delete;

    /// Non-movable
    text& operator=(text&&) = delete;

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
    void set_scaling_factor(float f_scaling_factor);

    /// Returns the scaling factor used when rendering glyphs.
    /** \return The scaling factor used when rendering glyphs
     */
    float get_scaling_factor() const;

    /// Sets the text to render (unicode character set).
    /** \param content The text to render
     *   \note This text can be formated :<br>
     *         - "|cAARRGGBB" : sets text color (hexadecimal).<br>
     *         - "|r" : sets text color to default.<br>
     *         - "||" : writes "|".
     */
    void set_text(const utils::ustring& content);

    /// Returns the text that will be rendered (unicode character set).
    /** \return The text that will be rendered (unicode character set)
     *   \note This string contains format tags.
     */
    const utils::ustring& get_text() const;

    /// Sets this text's default color.
    /** \param mColor      The default color
     *   \param bForceColor 'true' to ignore color tags
     */
    void set_color(const color& m_color, bool b_force_color = false);

    /// Returns this text's default color.
    /** \return This text's default color
     */
    const color& get_color() const;

    /// Sets this text's transparency (alpha).
    /** \param fAlpha The new alpha value
     */
    void set_alpha(float f_alpha);

    /// Returns this text's transparency (alpha).
    /** \return This text's transparency (alpha)
     */
    float get_alpha() const;

    /// Sets the dimensions of the text box.
    /** \param fW The new witdh
     *   \param fH The new height
     *   \note To remove the text box, use 0.0f.
     */
    void set_dimensions(float f_w, float f_h);

    /// Sets the width of the text box.
    /** \param fBoxW The new witdh
     *   \note To remove it, use 0.0f.
     */
    void set_box_width(float f_box_w);

    /// Sets the height of the text box.
    /** \param fBoxH The new height
     *   \note To remove it, use 0.0f.
     */
    void set_box_height(float f_box_h);

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
    std::size_t get_num_lines() const;

    /// Returns the lenght of a provided string.
    /** \param content The string to measure
     *   \return The lenght of the provided string
     */
    float get_string_width(const std::string& content) const;

    /// Returns the lenght of a provided string.
    /** \param content The string to measure
     *   \return The lenght of the provided string
     */
    float get_string_width(const utils::ustring& content) const;

    /// Returns the length of a single character.
    /** \param uiChar The character to measure
     *   \return The lenght of this character
     */
    float get_character_width(char32_t ui_char) const;

    /// Returns the kerning between two characters.
    /** \param uiChar1 The first character
     *   \param uiChar2 The second character
     *   \return The kerning between two characters
     *   \note Kerning is a letter spacing adjustment that makes the
     *         text look more condensed : is you stick an A near a V,
     *         you can reduce the space between the two letters, but not
     *         if you put two Vs side to side.
     */
    float get_character_kerning(char32_t ui_char1, char32_t ui_char2) const;

    /// Returns the height of the text.
    /** \return The height of one text
     *   \note Ignores the text box, but not manual line jumps.
     */
    float get_text_height() const;

    /// Sets text horizontal alignment.
    /** \param mAlignX The new horizontal alignment
     */
    void set_alignment_x(const alignment_x& m_align_x);

    /// Sets text vertical alignment.
    /** \param mAlignY The new vertical alignment
     */
    void set_alignment_y(const alignment_y& m_align_y);

    /// Returns the text horizontal alignment.
    /** \return The text horizontal alignment
     */
    const alignment_x& get_alignment_x() const;

    /// Returns the text vertical alignment.
    /** \return The text vertical alignment
     */
    const alignment_y& get_alignment_y() const;

    /// Sets this text's tracking.
    /** \param fTracking The new tracking
     *   \note Tracking is the space between each character. Default
     *         is 0.
     */
    void set_tracking(float f_tracking);

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
    void set_line_spacing(float f_line_spacing);

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
    void set_remove_starting_spaces(bool b_remove_starting_spaces);

    /// Checks if starting spaces removing is active.
    /** \return 'true' if starting spaces removing is active
     */
    bool get_remove_starting_spaces() const;

    /// Allows word wrap when the line is too long for the text box.
    /** \param bWrap        'true' to enable word wrap
     *   \param bAddEllipsis 'true' to put "..." at the end of a truncated line
     *   \note Enabled by default.
     */

    void enable_word_wrap(bool b_wrap, bool b_add_ellipsis);

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
    void enable_formatting(bool b_formatting);

    /// Renders this text at the given position.
    /** \param mTransform The transform to apply to the text
     *   \note Must be called between renderer::begin() and
     *         renderer::end(). If the transform is left to the default (IDENTITY),
     *         the text will be rendered at the top-left corner of the screen, with the
     *         anchor position (coordinate [0,0]) set by the vertical and horizontal
     *         alignment (see get_alignment() and get_vertical_alignment()).
     */
    void render(const matrix4f& m_transform = matrix4f::identity) const;

    /// Returns the number of letters currently displayed.
    /** \return The number of letters currently displayed
     *   \note This function may update the quad cache as needed.
     */
    std::size_t get_num_letters() const;

    /// Returns the quad for the letter at the provided index (position, texture coords, color).
    /** \param uiIndex The index of the letter (0: first letter);
     *                  must be less than get_num_letters().
     *   \return The quad of the specified letter
     *   \note The vertex positions in the quad do not account for the rendering position
     *         provided to render(). The first letter always has its top-left corner as
     *         the position (0,0) (if left-aligned). This function may update the quad cache as
     *         needed.
     */
    const std::array<vertex, 4>& get_letter_quad(std::size_t ui_index) const;

    /// Creates a quad that contains the provided character.
    /** \param uiChar The character to draw
     *   \note Uses this text's font texture.
     */
    quad create_letter_quad(char32_t ui_char) const;

    /// Returns the renderer used to render this text.
    /** \return The renderer used to render this text
     */
    const renderer& get_renderer() const {
        return m_renderer_;
    }

    /// Returns the renderer used to render this text.
    /** \return The renderer used to render this text
     */
    renderer& get_renderer() {
        return m_renderer_;
    }

private:
    void  update_() const;
    void  notify_cache_dirty_() const;
    float round_to_pixel_(
        float f_value, utils::rounding_method m_method = utils::rounding_method::nearest) const;
    std::array<vertex, 4> create_letter_quad_(gui::font& m_font, char32_t ui_char) const;
    std::array<vertex, 4> create_letter_quad_(char32_t ui_char) const;
    std::array<vertex, 4> create_outline_letter_quad_(char32_t ui_char) const;

    renderer& m_renderer_;

    bool        b_ready_                  = false;
    float       f_scaling_factor_         = 1.0f;
    float       f_tracking_               = 0.0f;
    float       f_line_spacing_           = 1.0f;
    bool        b_remove_starting_spaces_ = false;
    bool        b_word_wrap_              = true;
    bool        b_add_ellipsis_           = false;
    color       m_color_                  = color::white;
    bool        b_force_color_            = false;
    float       f_alpha_                  = 1.0f;
    bool        b_formatting_enabled_     = false;
    float       f_box_w_                  = std::numeric_limits<float>::infinity();
    float       f_box_h_                  = std::numeric_limits<float>::infinity();
    alignment_x m_align_x_                = alignment_x::left;
    alignment_y m_align_y_                = alignment_y::middle;

    std::shared_ptr<font> p_font_;
    std::shared_ptr<font> p_outline_font_;
    utils::ustring        unicode_text_;

    mutable bool        b_update_cache_ = false;
    mutable float       f_w_            = 0.0f;
    mutable float       f_h_            = 0.0f;
    mutable std::size_t ui_num_lines_   = 0u;

    mutable std::vector<std::array<vertex, 4>> quad_list_;
    mutable std::shared_ptr<vertex_cache>      p_vertex_cache_;
    mutable std::vector<std::array<vertex, 4>> outline_quad_list_;
    mutable std::shared_ptr<vertex_cache>      p_outline_vertex_cache_;
    mutable std::vector<quad>                  icons_list_;
};

} // namespace lxgui::gui

#endif
