#ifndef LXGUI_GUI_FONT_HPP
#define LXGUI_GUI_FONT_HPP

#include "lxgui/gui_bounds2.hpp"
#include "lxgui/gui_code_point_range.hpp"
#include "lxgui/gui_vector2.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

#include <array>
#include <memory>

namespace lxgui::gui {

class material;

/**
 * \brief A texture containing characters
 * This class is purely virtual. It needs to be implemented
 * and created by the corresponding gui::renderer.
 */
class font {
public:
    /// Constructor.
    font() = default;

    /// Destructor.
    virtual ~font() = default;

    /**
     * \brief Get the size of the font in pixels.
     * \return The size of the font in pixels
     */
    virtual std::size_t get_size() const = 0;

    /**
     * \brief Returns the uv coordinates of a character on the texture.
     * \param c The unicode character
     * \return The uv coordinates of this character on the texture
     * \note The uv coordinates are normalized, i.e. they range from
     *       0 to 1. They are arranged as {u1, v1, u2, v2}.
     */
    virtual bounds2f get_character_uvs(char32_t c) const = 0;

    /**
     * \brief Returns the rect coordinates of a character as it should be drawn relative to the baseline.
     * \param c The unicode character
     * \return The rect coordinates of this character (in pixels, relative to the baseline)
     */
    virtual bounds2f get_character_bounds(char32_t c) const = 0;

    /**
     * \brief Returns the width of a character in pixels.
     * \param c The unicode character
     * \return The width of the character in pixels.
     */
    virtual float get_character_width(char32_t c) const = 0;

    /**
     * \brief Returns the height of a character in pixels.
     * \param c The unicode character
     * \return The height of the character in pixels.
     */
    virtual float get_character_height(char32_t c) const = 0;

    /**
     * \brief Return the kerning amount between two characters.
     * \param c1 The first unicode character
     * \param c2 The second unicode character
     * \return The kerning amount between the two characters
     * \note Kerning is a font rendering adjustment that makes some
     *       letters closer, for example in 'VA', there is room for
     *       the two to be closer than with 'VW'. This has no effect
     *       for fixed width fonts (like Courrier, etc).
     */
    virtual float get_character_kerning(char32_t c1, char32_t c2) const = 0;

    /**
     * \brief Returns the underlying material to use for rendering.
     * \return The underlying material to use for rendering
     */
    virtual std::weak_ptr<material> get_texture() const = 0;

    /**
     * \brief Update the material to use for rendering.
     * \param mat The material to use for rendering
     */
    virtual void update_texture(std::shared_ptr<material> mat) = 0;
};

} // namespace lxgui::gui

#endif
