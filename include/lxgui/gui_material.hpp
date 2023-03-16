#ifndef LXGUI_GUI_MATERIAL_HPP
#define LXGUI_GUI_MATERIAL_HPP

#include "lxgui/gui_bounds2.hpp"
#include "lxgui/gui_vector2.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

namespace lxgui::gui {

/**
 * \brief A class that holds rendering data
 * \details This is an abstract class that must be implemented
 * and created by the corresponding gui::renderer.
 */
class material {
public:
    enum class wrap { repeat, clamp };

    enum class filter { none, linear };

    /**
     * \brief Constructor.
     * \param is_atlas 'true' if this material comes from an atlas, 'false' otherwise
     */
    explicit material(bool is_atlas);

    /// Destructor.
    virtual ~material() = default;

    /// Non-copiable
    material(const material&) = delete;

    /// Non-movable
    material(material&&) = delete;

    /// Non-copiable
    material& operator=(const material&) = delete;

    /// Non-movable
    material& operator=(material&&) = delete;

    /**
     * \brief Returns the pixel rect in pixels of the canvas containing this texture (if any).
     * \return The pixel rect in pixels of the canvas containing this texture (if any)
     */
    virtual bounds2f get_rect() const = 0;

    /**
     * \brief Returns the physical dimensions (in pixels) of the canvas containing this texture (if any).
     * \return The physical dimensions (in pixels) of the canvas containing this (if any)
     * \note When a texture is loaded, most of the time it will fill the entire "canvas",
     * namely, the 2D pixel array containing the texture data. However, some old
     * hardware don't support textures that have non power-of-two dimensions.
     * If the user creates a material for such a texture, the gui::renderer will
     * create a bigger canvas that has power-of-two dimensions, and store the
     * texture in it. Likewise, if a texture is placed in a wider texture atlas,
     * the canvas will contain more than one texture.
     */
    virtual vector2ui get_canvas_dimensions() const = 0;

    /**
     * \brief Checks if another material is based on the same texture as the current material.
     * \return 'true' if both materials use the same texture, 'false' otherwise
     */
    virtual bool uses_same_texture(const material& other) const = 0;

    /**
     * \brief Returns normalized UV coordinates on the canvas, given local UV coordinates.
     * \param texture_uv The original UV coordinates, local to this texture
     * \param from_normalized Set to 'true' if input coordinates are normalized
     * \note Normalized coordinates range from 0 to 1, with 1 corresponding to the width/height of
     * the texture, while non-normalized coordinates are in pixels.
     */
    vector2f get_canvas_uv(const vector2f& texture_uv, bool from_normalized) const;

    /**
     * \brief Returns local UV coordinates on the texture, given canvas UV coordinates.
     * \param canvas_uv The canvas UV coordinates
     * \param as_normalized Set to 'true' if output coordinates should be normalized
     * \note Normalized coordinates range from 0 to 1, with 1 corresponding to the width/height of
     * the texture, while non-normalized coordinates are in pixels.
     */
    vector2f get_local_uv(const vector2f& canvas_uv, bool as_normalized) const;

    /**
     * \brief Checks if the material is embedded in an atlas.
     * \return 'true' if the material is inside an atlas, 'false' otherwise.
     */
    bool is_in_atlas() const;

protected:
    bool is_atlas_ = false;
};

} // namespace lxgui::gui

#endif
