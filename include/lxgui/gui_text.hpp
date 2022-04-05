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
class text {
public:
    /**
     * \brief Constructor.
     * \param rdr The renderer instance to use
     * \param fnt The font to use for rendering
     * \param outline_fnt The font to use for outlines
     */
    explicit text(
        renderer&                   rdr,
        std::shared_ptr<const font> fnt,
        std::shared_ptr<const font> outline_fnt = nullptr);

    // Non-copiable, non-movable
    text(const text&) = delete;
    text(text&&)      = delete;
    text& operator=(const text&) = delete;
    text& operator=(text&&) = delete;

    /**
     * \brief Returns the height of one line (constant).
     * \return The height of one line (constant)
     */
    float get_line_height() const;

    /**
     * \brief Set the scaling factor to use when rendering glyphs.
     * \param scaling_factor The scaling factor
     * \note This defines the conversion factor between pixels (from the texture of the
     * font object) and interface units. By default this is set to 1, but needs to
     * be changed on high DPI systems.
     */
    void set_scaling_factor(float scaling_factor);

    /**
     * \brief Returns the scaling factor used when rendering glyphs.
     * \return The scaling factor used when rendering glyphs
     */
    float get_scaling_factor() const;

    /**
     * \brief Sets the text to render (unicode character set).
     * \param content The text to render
     * \note This text can be formatted :
     *  - "|cAARRGGBB": sets text color (hexadecimal).
     *  - "|r": sets text color to default.
     *  - "||": writes "|".
     */
    void set_text(const utils::ustring& content);

    /**
     * \brief Returns the text that will be rendered (unicode character set).
     * \return The text that will be rendered (unicode character set)
     * \note This string contains format tags.
     */
    const utils::ustring& get_text() const;

    /**
     * \brief Sets this text's default color.
     * \param c The default color
     * \param force_color 'true' to ignore color tags
     */
    void set_color(const color& c, bool force_color = false);

    /**
     * \brief Returns this text's default color.
     * \return This text's default color
     */
    const color& get_color() const;

    /**
     * \brief Sets this text's transparency (alpha).
     * \param alpha The new alpha value
     */
    void set_alpha(float alpha);

    /**
     * \brief Returns this text's transparency (alpha).
     * \return This text's transparency (alpha)
     */
    float get_alpha() const;

    /**
     * \brief Sets the dimensions of the text box.
     * \param box_width The new width
     * \param box_height The new height
     * \note To remove the text box, use 0.0f.
     */
    void set_box_dimensions(float box_width, float box_height);

    /**
     * \brief Sets the width of the text box.
     * \param box_width The new width
     * \note To remove it, use 0.0f.
     */
    void set_box_width(float box_width);

    /**
     * \brief Sets the height of the text box.
     * \param box_height The new height
     * \note To remove it, use 0.0f.
     */
    void set_box_height(float box_height);

    /**
     * \brief Returns the width of the rendered text.
     * \return The width of the rendered text
     * \note Takes the text box into account if any.
     */
    float get_width() const;

    /**
     * \brief Returns the height of the rendered text.
     * \return The height of the rendered text
     * \note Takes the text box into account if any.
     */
    float get_height() const;

    /**
     * \brief Returns the width of the text box.
     * \return The width of the text box
     */
    float get_box_width() const;

    /**
     * \brief Returns the height of the text box.
     * \return The height of the text box
     */
    float get_box_height() const;

    /**
     * \brief Returns the length of the text.
     * \return The length of the text
     * \note Ignores the text box, but not manual line jumps.
     */
    float get_text_width() const;

    /**
     * \brief Returns the number of text lines.
     * \return The number of text lines
     */
    std::size_t get_line_count() const;

    /**
     * \brief Returns the length of a provided string.
     * \param content The string to measure
     * \return The length of the provided string
     */
    float get_string_width(const std::string& content) const;

    /**
     * \brief Returns the length of a provided string.
     * \param content The string to measure
     * \return The length of the provided string
     */
    float get_string_width(const utils::ustring& content) const;

    /**
     * \brief Returns the length of a single character.
     * \param c The character to measure
     * \return The length of this character
     */
    float get_character_width(char32_t c) const;

    /**
     * \brief Returns the kerning between two characters.
     * \param c1 The first character
     * \param c2 The second character
     * \return The kerning between two characters
     * \note Kerning is a letter spacing adjustment that makes the
     * text look more condensed: is you stick an A near a V,
     * you can reduce the space between the two letters, but not
     * if you put two Vs side to side.
     */
    float get_character_kerning(char32_t c1, char32_t c2) const;

    /**
     * \brief Returns the height of the text.
     * \return The height of one text
     * \note Ignores the text box, but not manual line jumps.
     */
    float get_text_height() const;

    /**
     * \brief Sets text horizontal alignment.
     * \param align_x The new horizontal alignment
     */
    void set_alignment_x(alignment_x align_x);

    /**
     * \brief Sets text vertical alignment.
     * \param align_y The new vertical alignment
     */
    void set_alignment_y(alignment_y align_y);

    /**
     * \brief Returns the text horizontal alignment.
     * \return The text horizontal alignment
     */
    alignment_x get_alignment_x() const;

    /**
     * \brief Returns the text vertical alignment.
     * \return The text vertical alignment
     */
    alignment_y get_alignment_y() const;

    /**
     * \brief Sets this text's tracking.
     * \param tracking The new tracking
     * \note Tracking is the space between each character. Default is 0.
     */
    void set_tracking(float tracking);

    /**
     * \brief Returns this text's tracking.
     * \return This text's tracking
     */
    float get_tracking() const;

    /**
     * \brief Sets this text's line spacing.
     * \param line_spacing The new line spacing
     * \note Line spacing is a coefficient that, multiplied by the
     * height of a line, gives the space between two lines.
     * Default is 1.5f.
     */
    void set_line_spacing(float line_spacing);

    /**
     * \brief Returns this text's line spacing.
     * \return This text's line spacing
     */
    float get_line_spacing() const;

    /**
     * \brief Allows removal of a line's starting spaces.
     * \param remove_starting_spaces 'true' to remove them
     * \note The text box does word wrapping: it cuts too long
     * lines only between words. But sometimes, the rendered
     * text must be cut between several spaces. By default,
     * the algorithm puts cut spaces at the beginning of
     * the next line. You can change this behavior by setting
     * this function to 'true'.
     */
    void set_remove_starting_spaces(bool remove_starting_spaces);

    /**
     * \brief Checks if starting spaces removing is active.
     * \return 'true' if starting spaces removing is active
     */
    bool get_remove_starting_spaces() const;

    /**
     * \brief Allows/disallows word wrap when the line is too long for the text box.
     * \param wrap 'true' to enable word wrap
     * \note Enabled by default.
     */
    void set_word_wrap_enabled(bool wrap);

    /**
     * \brief Allows word wrap when the line is too long for the text box.
     */
    void enable_word_wrap() {
        set_word_wrap_enabled(true);
    }

    /**
     * \brief Disallow word wrap when the line is too long for the text box.
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
     * \note Enabled by default. See \ref set_text for more information on formatting.
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
     * \brief Renders this text at the given position.
     * \param transform The transform to apply to the text
     * \note Must be called between renderer::begin() and
     * renderer::end(). If the transform is left to the default (IDENTITY),
     * the text will be rendered at the top-left corner of the screen, with the
     * anchor position (coordinate [0,0]) set by the vertical and horizontal
     * alignment (see get_alignment() and get_vertical_alignment()).
     */
    void render(const matrix4f& transform = matrix4f::identity) const;

    /**
     * \brief Returns the number of letters currently displayed.
     * \return The number of letters currently displayed
     * \note This function may update the quad cache as needed.
     */
    std::size_t get_letter_count() const;

    /**
     * \brief Returns the quad for the letter at the provided index (position, texture coords, color).
     * \param index The index of the letter (0: first letter); must be less than get_letter_count().
     * \return The quad of the specified letter
     * \note The vertex positions in the quad do not account for the rendering position
     * provided to render(). The first letter always has its top-left corner as
     * the position (0,0) (if left-aligned). This function may update the quad cache as
     * needed.
     */
    const std::array<vertex, 4>& get_letter_quad(std::size_t index) const;

    /**
     * \brief Creates a quad that contains the provided character.
     * \param c The character to draw
     * \note Uses this text's font texture.
     */
    quad create_letter_quad(char32_t c) const;

    /**
     * \brief Returns the renderer used to render this text.
     * \return The renderer used to render this text
     */
    const renderer& get_renderer() const {
        return renderer_;
    }

    /**
     * \brief Returns the renderer used to render this text.
     * \return The renderer used to render this text
     */
    renderer& get_renderer() {
        return renderer_;
    }

private:
    void update_() const;
    void notify_cache_dirty_() const;

    float round_to_pixel_(
        float value, utils::rounding_method method = utils::rounding_method::nearest) const;

    std::array<vertex, 4> create_letter_quad_(const gui::font& font, char32_t c) const;
    std::array<vertex, 4> create_letter_quad_(char32_t c) const;
    std::array<vertex, 4> create_outline_letter_quad_(char32_t c) const;

    renderer& renderer_;

    bool        is_ready_               = false;
    float       scaling_factor_         = 1.0f;
    float       tracking_               = 0.0f;
    float       line_spacing_           = 1.0f;
    bool        remove_starting_spaces_ = false;
    bool        word_wrap_enabled_      = true;
    bool        ellipsis_enabled_       = false;
    color       color_                  = color::white;
    bool        force_color_            = false;
    float       alpha_                  = 1.0f;
    bool        formatting_enabled_     = false;
    float       box_width_              = std::numeric_limits<float>::infinity();
    float       box_height_             = std::numeric_limits<float>::infinity();
    alignment_x align_x_                = alignment_x::left;
    alignment_y align_y_                = alignment_y::middle;

    std::shared_ptr<const font> font_;
    std::shared_ptr<const font> outline_font_;
    utils::ustring              unicode_text_;

    mutable bool        update_cache_flag_ = false;
    mutable float       width_             = 0.0f;
    mutable float       height_            = 0.0f;
    mutable std::size_t num_lines_         = 0u;

    mutable std::vector<std::array<vertex, 4>> quad_list_;
    mutable std::shared_ptr<vertex_cache>      vertex_cache_;
    mutable std::vector<std::array<vertex, 4>> outline_quad_list_;
    mutable std::shared_ptr<vertex_cache>      outline_vertex_cache_;
    mutable std::vector<quad>                  icons_list_;
};

} // namespace lxgui::gui

#endif
