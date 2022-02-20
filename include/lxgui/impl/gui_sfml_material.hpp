#ifndef LXGUI_GUI_SFML_MATERIAL_HPP
#define LXGUI_GUI_SFML_MATERIAL_HPP

#include "lxgui/gui_bounds2.hpp"
#include "lxgui/gui_color.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/utils.hpp"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <memory>
#include <vector>

namespace lxgui::gui::sfml {

/// A class that holds rendering data
/** This implementation can contain either a plain color
 *   or a real sf::Texture. It is also used by the
 *   gui::sfml::render_target class to store the output data.
 */
class material final : public gui::material {
public:
    /// Constructor for textures.
    /** \param dimensions   The requested texture dimensions
     *   \param is_render_target Create the material for a render target or only for display
     *   \param wrp         How to adjust texture coordinates that are outside the [0,1] range
     *   \param filt       Use texture filtering or not (see set_filter())
     */
    material(
        const vector2ui& dimensions,
        bool             is_render_target,
        wrap             wrp  = wrap::repeat,
        filter           filt = filter::none);

    /// Constructor for textures.
    /** \param data         The image data to use as texture
     *   \param wrp         How to adjust texture coordinates that are outside the [0,1] range
     *   \param filt       Use texture filtering or not (see set_filter())
     */
    explicit material(const sf::Image& data, wrap wrp = wrap::repeat, filter filt = filter::none);

    /// Constructor for textures.
    /** \param file_name     The file from which the texture data is loaded
     *   \param wrp         How to adjust texture coordinates that are outside the [0,1] range
     *   \param filt       Use texture filtering or not (see set_filter())
     */
    explicit material(
        const std::string& file_name, wrap wrp = wrap::repeat, filter filt = filter::none);

    /// Constructor for atlas textures.
    /** \param texture  The atlas texture holding this material's texture
     *   \param location The location of the texture inside the atlas texture (in pixels)
     *   \param filt   Use texture filtering or not (see set_filter())
     */
    explicit material(
        const sf::Texture& texture, const bounds2f& location, filter filt = filter::none);

    material(const material& tex) = delete;
    material(material&& tex)      = delete;
    material& operator=(const material& tex) = delete;
    material& operator=(material&& tex) = delete;

    /// Returns the pixel rect in pixels of the canvas containing this texture (if any).
    /** \return The pixel rect in pixels of the canvas containing this texture (if any)
     */
    bounds2f get_rect() const override;

    /// Returns the physical dimensions (in pixels) of the canvas containing this texture (if any).
    /** \return The physical dimensions (in pixels) of the canvas containing this (if any)
     *   \note When a texture is loaded, most of the time it will fill the entire "canvas",
     *         namely, the 2D pixel array containing the texture data. However, some old
     *         hardware don't support textures that have non power-of-two dimensions.
     *         If the user creates a material for such a texture, the gui::renderer will
     *         create a bigger canvas that has power-of-two dimensions, and store the
     *         texture in it. Likewise, if a texture is placed in a wider texture atlas,
     *         the canvas will contain more than one texture.
     */
    vector2ui get_canvas_dimensions() const override;

    /// Checks if another material is based on the same texture as the current material.
    /** \return 'true' if both materials use the same texture, 'false' otherwise
     */
    bool uses_same_texture(const gui::material& other) const override;

    /// Resizes this texture.
    /** \param dimensions The new texture dimensions
     *   \return 'true' if the function had to re-create a new texture object
     *   \note All the previous data that was stored in this texture will be lost.
     */
    bool set_dimensions(const vector2ui& dimensions);

    /// Premultiplies an image by its alpha component.
    /** \note Premultiplied alpha is a rendering technique that allows perfect
     *         alpha blending when using render targets.
     */
    static void premultiply_alpha(sf::Image& data);

    /// Sets the wrap mode of this texture.
    /** \param wrp How to adjust texture coordinates that are outside the [0,1] range
     */
    void set_wrap(wrap wrp);

    /// Sets the filter mode of this texture.
    /** \param filt Use texture filtering or not
     *   \note When texture filtering is disabled, enlarged textures get pixelated.
     *         Else, the GPU uses an averaging algorithm to blur the pixels.
     */
    void set_filter(filter filt);

    /// Returns the filter mode of this texture.
    /** \return The filter mode of this texture
     */
    filter get_filter() const;

    /// Updates the texture that is in GPU memory.
    /** \param data The new pixel data
     */
    void update_texture(const ub32color* data);

    /// Returns the underlying SFML render texture object.
    /** return The underlying SFML render texture object
     */
    sf::RenderTexture* get_render_texture();

    /// Returns the underlying SFML texture object.
    /** return The underlying SFML texture object
     */
    const sf::Texture* get_texture() const;

private:
    vector2ui dimensions_;
    vector2ui canvas_dimensions_;
    bounds2f  rect_;
    wrap      wrap_   = wrap::repeat;
    filter    filter_ = filter::none;

    bool               is_render_target_ = false;
    sf::RenderTexture  render_texture_;
    sf::Texture        texture_;
    const sf::Texture* atlas_texture_ = nullptr;

    static const std::size_t maximum_size;
};

} // namespace lxgui::gui::sfml

#endif
