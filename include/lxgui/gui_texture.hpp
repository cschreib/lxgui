#ifndef LXGUI_GUI_TEXTURE_HPP
#define LXGUI_GUI_TEXTURE_HPP

#include "lxgui/gui_gradient.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

#include <limits>
#include <variant>

namespace lxgui::gui {

class renderer;

/// A layered_region that can draw images and colored rectangles.
/** This object contains either a texture taken from a file,
 *   or a plain color (possibly with a different color on each corner).
 */
class texture : public layered_region {
    using base = layered_region;

public:
    enum class blend_mode { none, blend, key, add, mod };

    /// Constructor.
    explicit texture(utils::control_block& m_block, manager& m_manager);

    /// Prints all relevant information about this region in a string.
    /** \param tab The offset to give to all lines
     *   \return All relevant information about this region
     */
    std::string serialize(const std::string& tab) const override;

    /// Renders this region on the current render target.
    void render() const override;

    /// Copies a region's parameters into this texture (inheritance).
    /** \param mObj The region to copy
     */
    void copy_from(const region& m_obj) override;

    /// Returns this texture's blending mode.
    /** \return This texture's blending mode
     */
    blend_mode get_blend_mode() const;

    /// Returns this texture's filtering algorithm.
    /** \return This texture's filtering algorithm
     */
    material::filter get_filter_mode() const;

    /// Checks if this texture is defined as solid color.
    /** \return 'true' if the texutre is defined as solid color, 'false' otherwise
     */
    bool has_solid_color() const;

    /// Returns this texture's color.
    /** \return This texture's color (color::EMPTY if none)
     */
    const color& get_solid_color() const;

    /// Checks if this texture is defined as a gradient.
    /** \return 'true' if the texture is defined a gradient, 'false' otherwise
     */
    bool has_gradient() const;

    /// Returns this texture's gradient.
    /** \return This texture's gradient (Gradient::NONE if none)
     */
    const gradient& get_gradient() const;

    /// Returns this texture's texture coordinates.
    /** \return This texture's texture coordinates
     *   \note The texture coordinates are arranged as a rectangle, which is made
     *         of four points : 1 (top left), 2 (top right), 3 (bottom right) and
     *         4 (bottom left).<br>
     *         The returned array is composed like this :
     *         (x1, y1, x2, y2, x3, y3, x4, y4).
     */
    std::array<float, 8> get_tex_coord() const;

    /// Checks if this texture's dimensions are affected by texture coordinates.
    /** \return 'true' if this texture's dimensions are affected by texture
     *           coordinates
     */
    bool get_tex_coord_modifies_rect() const;

    /// Checks if this texture is defined as a texture file.
    /** \return 'true' if the texture is defined a texture file, 'false' otherwise
     */
    bool has_texture_file() const;

    /// Returns this textures's texture file.
    /** \return This textures's texture file (empty string if none).
     */
    const std::string& get_texture_file() const;

    /// Returns this textures's vertex color.
    /** \param uiIndex The vertex index (0 to 3 included)
     *   \return This textures's vertex color
     *   \note This color is used to filter the texture's colors :
     *         for each pixel, the original color is multiplied
     *         by this vertex color.
     */
    color get_vertex_color(std::size_t ui_index) const;

    /// Checks if this texture is desaturated.
    /** \return 'true' if the texture is desaturated
     *   \note Only available on certain graphic cards (most of modern ones
     *         are capable of this).
     */
    bool is_desaturated() const;

    /// Sets this texture's blending mode.
    /** \param mBlendMode The new blending mode
     */
    void set_blend_mode(blend_mode m_blend_mode);

    /// Sets this texture's blending mode.
    /** \param blend_mode_name The new blending mode
     */
    void set_blend_mode(const std::string& blend_mode_name);

    /// Sets this texture's filtering mode.
    /** \param mFilter The new filtering mode
     */
    void set_filter_mode(material::filter m_filter);

    /// Sets this texture's blending mode.
    /** \param filter_name The new filtering mode
     */
    void set_filter_mode(const std::string& filter_name);

    /// Makes this texture appear without any color.
    /** \param bIsDesaturated 'true' if you want to remove colors
     */
    void set_desaturated(bool b_is_desaturated);

    /// Adds a gradient effect to this texture.
    /** \param mGradient The gradient to add
     *   \note To remove a gradient, call set_gradient(Gradient::NONE).
     */
    void set_gradient(const gradient& m_gradient);

    /// Sets this texture's texture coordinates.
    /** \param texture_rect This texture's texture coordinates
     *   \note The texture coordinates are arranged as a rectangle, which is made
     *         of four points : 1 (top left), 2 (top right), 3 (bottom right) and
     *         4 (bottom left).<br>
     *         The array must be arranged like this : (x1, y1, x3, y3), or (left,
     *         top, right, bottom). Other corners are calculated using these coordinates.
     *   \note This function only allows horizontal/rectangle texture coordinates.
     */
    void set_tex_rect(const std::array<float, 4>& texture_rect);

    /// Sets this texture's texture coordinates.
    /** \param texture_coords This texture's texture coordinates
     *   \note The texture coordinates are arranged as a rectangle, which is made
     *         of four points : 1 (top left), 2 (top right), 3 (bottom right) and
     *         4 (bottom left).<br>
     *         The array must be arranged like this :
     *         (x1, y1, x2, y2, x3, y3, x4, y4).
     *   \note This function allows rotated/deformed texture coordinates.
     */
    void set_tex_coord(const std::array<float, 8>& texture_coords);

    /// Sets whether this texture's dimensions are affected by texture coordinates.
    /** \param bTexCoordModifiesRect 'true' to make dimensions change with tex coords
     */
    void set_tex_coord_modifies_rect(bool b_tex_coord_modifies_rect);

    /// Sets this texture's texture file.
    /** \param file_name The file from which to read data
     *   \note This function takes care of checking that the file can be opened.
     *   \note This function will replace the solid color set by set_solid_color(). If you need
     *         to blend the texture with a color, use set_vertex_color() instead.
     */
    void set_texture(const std::string& file_name);

    /// Reads texture data from a render_target.
    /** \param pRenderTarget The render_target from which to read the data
     *   \note This function will replace the solid color set by set_solid_color(). If you need
     *         to blend the texture with a color, use set_vertex_color() instead.
     */
    void set_texture(std::shared_ptr<render_target> p_render_target);

    /// Sets this texture's color.
    /** \param mColor The color to use
     *   \note This function will replace the texture set by set_texture() with a solid color.
     *         If you need to blend the texture with a color, use set_vertex_color() instead.
     */
    void set_solid_color(const color& m_color);

    /// Directly sets this texture's underlying quad (vertices and material).
    /** \param mQuad The new quad to use
     *   \note The texture's dimensions will be adjusted to fit those
     *         of the provided quad, and same goes for texture coordinates.
     */
    void set_quad(const quad& m_quad);

    /// Sets this texture's vertex color.
    /** \param mColor This textures's new vertex color
     *   \param uiIndex The vertex index (-1: all vertices)
     *   \note This color is used to filter the texture's colors:
     *         for each pixel, the original color is multiplied
     *         by this vertex color.
     */
    void set_vertex_color(
        const color& m_color, std::size_t ui_index = std::numeric_limits<std::size_t>::max());

    /// Creates the associated Lua glue.
    void create_glue() override;

    /// Parses data from a layout_node.
    /** \param mNode The layout node
     */
    void parse_layout(const layout_node& m_node) override;

    /// Registers this region class to the provided Lua state
    static void register_on_lua(sol::state& m_lua);

    static constexpr const char* class_name = "Texture";

private:
    void parse_attributes_(const layout_node& m_node) override;
    void parse_tex_coords_node_(const layout_node& m_node);
    void parse_gradient_node_(const layout_node& m_node);

    void update_dimensions_from_tex_coord_();
    void update_borders_() override;

    using content      = std::variant<color, std::string, gradient>;
    content m_content_ = color::white;

    blend_mode       m_blend_mode_              = blend_mode::blend;
    material::filter m_filter_                  = material::filter::none;
    bool             b_is_desaturated_          = false;
    bool             b_tex_coord_modifies_rect_ = false;

    renderer& m_renderer_;
    quad      m_quad_;
};

} // namespace lxgui::gui

#endif
