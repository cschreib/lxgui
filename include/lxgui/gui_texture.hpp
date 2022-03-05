#ifndef LXGUI_GUI_TEXTURE_HPP
#define LXGUI_GUI_TEXTURE_HPP

#include "lxgui/gui_gradient.hpp"
#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

#include <limits>
#include <variant>

namespace lxgui::gui {

class renderer;

/**
 * \brief A layered_region that can draw images and colored rectangles.
 * \details This object contains either a texture taken from a file,
 * or a plain color (possibly with a different color on each corner).
 */
class texture : public layered_region {
    using base = layered_region;

public:
    enum class blend_mode { none, blend, key, add, mod };

    /// Constructor.
    explicit texture(utils::control_block& block, manager& mgr, const region_core_attributes& attr);

    /**
     * \brief Prints all relevant information about this region in a string.
     * \param tab The offset to give to all lines
     * \return All relevant information about this region
     */
    std::string serialize(const std::string& tab) const override;

    /// Renders this region on the current render target.
    void render() const override;

    /**
     * \brief Copies a region's parameters into this texture (inheritance).
     * \param obj The region to copy
     */
    void copy_from(const region& obj) override;

    /**
     * \brief Returns this texture's blending mode.
     * \return This texture's blending mode
     */
    blend_mode get_blend_mode() const;

    /**
     * \brief Returns this texture's filtering algorithm.
     * \return This texture's filtering algorithm
     */
    material::filter get_filter_mode() const;

    /**
     * \brief Checks if this texture is defined as solid color.
     * \return 'true' if the texture is defined as solid color, 'false' otherwise
     */
    bool has_solid_color() const;

    /**
     * \brief Returns this texture's color.
     * \return This texture's color (color::EMPTY if none)
     */
    const color& get_solid_color() const;

    /**
     * \brief Checks if this texture is defined as a gradient.
     * \return 'true' if the texture is defined a gradient, 'false' otherwise
     */
    bool has_gradient() const;

    /**
     * \brief Returns this texture's gradient.
     * \return This texture's gradient (Gradient::NONE if none)
     */
    const gradient& get_gradient() const;

    /**
     * \brief Returns this texture's texture coordinates.
     * \return This texture's texture coordinates
     * \note The texture coordinates are arranged as a rectangle, which is made
     * of four points: 1 (top left), 2 (top right), 3 (bottom right) and
     * 4 (bottom left). The returned array is composed like this:
     * `(x1, y1, x2, y2, x3, y3, x4, y4)`.
     */
    std::array<float, 8> get_tex_coord() const;

    /**
     * \brief Checks if this texture can stretch to match the region dimensions
     * \return 'true' if this texture can stretch to match the region dimensions
     */
    bool get_texture_stretching() const;

    /**
     * \brief Checks if this texture is defined as a texture file.
     * \return 'true' if the texture is defined a texture file, 'false' otherwise
     */
    bool has_texture_file() const;

    /**
     * \brief Returns this texture's texture file.
     * \return This texture's texture file (empty string if none).
     */
    const std::string& get_texture_file() const;

    /**
     * \brief Returns this texture's vertex color.
     * \param index The vertex index (0 to 3 included)
     * \return This texture's vertex color
     * \note This color is used to filter the texture's colors:
     * for each pixel, the original color is multiplied by this vertex color.
     */
    color get_vertex_color(std::size_t index) const;

    /**
     * \brief Checks if this texture is desaturated.
     * \return 'true' if the texture is desaturated
     * \note Only available on certain graphic cards (most of modern ones
     * are capable of this).
     */
    bool is_desaturated() const;

    /**
     * \brief Sets this texture's blending mode.
     * \param mode The new blending mode
     */
    void set_blend_mode(blend_mode mode);

    /**
     * \brief Sets this texture's filtering mode.
     * \param filt The new filtering mode
     */
    void set_filter_mode(material::filter filt);

    /**
     * \brief Makes this texture appear without any color.
     * \param is_desaturated 'true' if you want to remove colors
     */
    void set_desaturated(bool is_desaturated);

    /**
     * \brief Adds a gradient effect to this texture.
     * \param g The gradient to add
     * \note To remove a gradient, call set_gradient(Gradient::NONE).
     */
    void set_gradient(const gradient& g);

    /**
     * \brief Sets this texture's texture coordinates.
     * \param texture_rect This texture's texture coordinates
     * \note The texture coordinates are arranged as a rectangle, which is made
     * of four points: 1 (top left), 2 (top right), 3 (bottom right) and
     * 4 (bottom left). The array must be arranged like this: `(x1, y1, x3, y3)`, or
     * `(left, top, right, bottom)`. Other corners are calculated using these coordinates.
     * \note This function only allows horizontal/rectangle texture coordinates.
     */
    void set_tex_rect(const std::array<float, 4>& texture_rect);

    /**
     * \brief Sets this texture's texture coordinates.
     * \param texture_coords This texture's texture coordinates
     * \note The texture coordinates are arranged as a rectangle, which is made
     * of four points: 1 (top left), 2 (top right), 3 (bottom right) and
     * 4 (bottom left). The array must be arranged like this:
     * `(x1, y1, x2, y2, x3, y3, x4, y4)`.
     * \note This function allows rotated/deformed texture coordinates.
     */
    void set_tex_coord(const std::array<float, 8>& texture_coords);

    /**
     * \brief Sets whether this texture can stretch to match the region dimensions.
     * \param texture_stretching 'true' to allow texture stretching change with tex coords
     */
    void set_texture_stretching(bool texture_stretching);

    /**
     * \brief Sets this texture's texture file.
     * \param file_name The file from which to read data
     * \note This function takes care of checking that the file can be opened.
     * \note This function will replace the solid color set by set_solid_color(). If you need
     * to blend the texture with a color, use set_vertex_color() instead.
     */
    void set_texture(const std::string& file_name);

    /**
     * \brief Reads texture data from a render_target.
     * \param target The render_target from which to read the data
     * \note This function will replace the solid color set by set_solid_color(). If you need
     * to blend the texture with a color, use set_vertex_color() instead.
     */
    void set_texture(std::shared_ptr<render_target> target);

    /**
     * \brief Sets this texture's color.
     * \param c The color to use
     * \note This function will replace the texture set by set_texture() with a solid color.
     * If you need to blend the texture with a color, use set_vertex_color() instead.
     */
    void set_solid_color(const color& c);

    /**
     * \brief Directly sets this texture's underlying quad (vertices and material).
     * \param q The new quad to use
     * \note The texture's dimensions will be adjusted to fit those
     * of the provided quad, and same goes for texture coordinates.
     */
    void set_quad(const quad& q);

    /**
     * \brief Sets this texture's vertex color.
     * \param c This texture's new vertex color
     * \param index The vertex index (-1: all vertices)
     * \note This color is used to filter the texture's colors:
     * for each pixel, the original color is multiplied
     * by this vertex color.
     */
    void
    set_vertex_color(const color& c, std::size_t index = std::numeric_limits<std::size_t>::max());

    /**
     * \brief Parses data from a layout_node.
     * \param node The layout node
     */
    void parse_layout(const layout_node& node) override;

    /// Registers this region class to the provided Lua state
    static void register_on_lua(sol::state& lua);

    static constexpr const char* class_name = "Texture";

private:
    void parse_attributes_(const layout_node& node) override;
    void parse_tex_coords_node_(const layout_node& node);
    void parse_gradient_node_(const layout_node& node);

    void update_dimensions_from_tex_coord_();
    void update_borders_() override;

    using content    = std::variant<color, std::string, gradient>;
    content content_ = color::white;

    blend_mode       blend_mode_                    = blend_mode::blend;
    material::filter filter_                        = material::filter::none;
    bool             is_desaturated_                = false;
    bool             is_texture_stretching_enabled_ = true;

    renderer& renderer_;
    quad      quad_;
};

} // namespace lxgui::gui

#endif
