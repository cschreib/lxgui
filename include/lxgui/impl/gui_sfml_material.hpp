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
    /** \param mDimensions   The requested texture dimensions
     *   \param bRenderTarget Create the material for a render target or only for display
     *   \param mWrap         How to adjust texture coordinates that are outside the [0,1] range
     *   \param mFilter       Use texture filtering or not (see set_filter())
     */
    material(
        const vector2ui& m_dimensions,
        bool             b_render_target,
        wrap             m_wrap   = wrap::repeat,
        filter           m_filter = filter::none);

    /// Constructor for textures.
    /** \param mData         The image data to use as texture
     *   \param mWrap         How to adjust texture coordinates that are outside the [0,1] range
     *   \param mFilter       Use texture filtering or not (see set_filter())
     */
    explicit material(
        const sf::Image& m_data, wrap m_wrap = wrap::repeat, filter m_filter = filter::none);

    /// Constructor for textures.
    /** \param sFileName     The file from which the texture data is loaded
     *   \param mWrap         How to adjust texture coordinates that are outside the [0,1] range
     *   \param mFilter       Use texture filtering or not (see set_filter())
     */
    explicit material(
        const std::string& s_file_name, wrap m_wrap = wrap::repeat, filter m_filter = filter::none);

    /// Constructor for atlas textures.
    /** \param mTexture  The atlas texture holding this material's texture
     *   \param mLocation The location of the texture inside the atlas texture (in pixels)
     *   \param mFilter   Use texture filtering or not (see set_filter())
     */
    explicit material(
        const sf::Texture& m_texture, const bounds2f& m_location, filter m_filter = filter::none);

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
    bool uses_same_texture(const gui::material& m_other) const override;

    /// Resizes this texture.
    /** \param mDimensions The new texture dimensions
     *   \return 'true' if the function had to re-create a new texture object
     *   \note All the previous data that was stored in this texture will be lost.
     */
    bool set_dimensions(const vector2ui& m_dimensions);

    /// Premultiplies an image by its alpha component.
    /** \note Premultiplied alpha is a rendering technique that allows perfect
     *         alpha blending when using render targets.
     */
    static void premultiply_alpha(sf::Image& m_data);

    /// Sets the wrap mode of this texture.
    /** \param mWrap How to adjust texture coordinates that are outside the [0,1] range
     */
    void set_wrap(wrap m_wrap);

    /// Sets the filter mode of this texture.
    /** \param mFilter Use texture filtering or not
     *   \note When texture filtering is disabled, enlarged textures get pixelated.
     *         Else, the GPU uses an averaging algorithm to blur the pixels.
     */
    void set_filter(filter m_filter);

    /// Returns the filter mode of this texture.
    /** \return The filter mode of this texture
     */
    filter get_filter() const;

    /// Updates the texture that is in GPU memory.
    /** \param pData The new pixel data
     */
    void update_texture(const ub32color* p_data);

    /// Returns the underlying SFML render texture object.
    /** return The underlying SFML render texture object
     */
    sf::RenderTexture* get_render_texture();

    /// Returns the underlying SFML texture object.
    /** return The underlying SFML texture object
     */
    const sf::Texture* get_texture() const;

private:
    vector2ui m_dimensions_;
    vector2ui m_canvas_dimensions_;
    bounds2f  m_rect_;
    wrap      m_wrap_   = wrap::repeat;
    filter    m_filter_ = filter::none;

    bool               b_render_target_ = false;
    sf::RenderTexture  m_render_texture_;
    sf::Texture        m_texture_;
    const sf::Texture* p_atlas_texture_ = nullptr;

    static const std::size_t maximum_size;
};

} // namespace lxgui::gui::sfml

#endif
