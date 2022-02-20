#ifndef LXGUI_GUI_BACKDROP_HPP
#define LXGUI_GUI_BACKDROP_HPP

#include "lxgui/gui_bounds2.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/gui_vertexcache.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

#include <string>

namespace lxgui::gui {

class frame;

/// Draws borders and background of a frame.
class backdrop {
public:
    /// Constructor.
    /** \param parent The frame it is linked to
     */
    explicit backdrop(frame& parent);

    /// Non-copiable
    backdrop(const backdrop&) = delete;

    /// Non-movable
    backdrop(backdrop&&) = delete;

    /// Non-copiable
    backdrop& operator=(const backdrop&) = delete;

    /// Non-movable
    backdrop& operator=(backdrop&&) = delete;

    /// Copies a backdrop's parameters into this one (inheritance).
    /** \param other The backdrop to copy
     */
    void copy_from(const backdrop& other);

    /// Sets the background texture.
    /** \param background_file The background texture
     */
    void set_background(const std::string& background_file);

    /// Returns this backdrop's background file.
    /** \return This backdrop's background file
     */
    const std::string& get_background_file() const;

    /// Sets the background color.
    /** \param c The background color
     *   \note This color can be used to tint the background texture if any
     *         or simply render a plain color background.
     */
    void set_background_color(const color& c);

    /// Returns the background color.
    /** \return The background color
     */
    color get_background_color() const;

    /// Enables tilling for the background texture.
    /** \param is_tilling 'true' to enable tilling
     */
    void set_background_tilling(bool is_tilling);

    /// Checks if tilling is enabled for the background texture.
    /** \return 'true' if tilling is enabled for the background texture
     */
    bool is_background_tilling() const;

    /// Sets the appearent tile size.
    /** \param tile_size The new tile size
     *   \note Tile will be scaled by tile_size/background_texture_size.
     */
    void set_tile_size(float tile_size);

    /// Returns this backdrop's tile size.
    /** \return This backdrop's tile size
     */
    float get_tile_size() const;

    /// Sets insets for the background texture.
    /** \param insets The insets array
     */
    void set_background_insets(const bounds2f& insets);

    /// Returns this backdrop's background insets.
    /** \return This backdrop's background insets
     */
    const bounds2f& get_background_insets() const;

    /// Sets insets for the edge texture.
    /** \param insets The insets array
     */
    void set_edge_insets(const bounds2f& insets);

    /// Returns this backdrop's edge insets.
    /** \return This backdrop's edge insets
     */
    const bounds2f& get_edge_insets() const;

    /// Sets the edge/corner texture.
    /** \param edge_file The edge/corner texture
     *   \note This texture's width must be 8 times greater than its
     *         height.<br><br>
     *         texture parts are interpreted as :<br>
     *         - [  0, 1/8] : left edge
     *         - [1/8, 1/4] : right edge
     *         - [1/4, 3/8] : top edge (rotated 90 degrees ccw)
     *         - [3/8, 1/2] : bottom edge (rotated 90 degrees ccw)
     *         - [1/2, 5/8] : top-left corner
     *         - [5/8, 3/4] : top-right corner
     *         - [3/4, 7/8] : bottom-left corner
     *         - [7/8,   1] : bottom-right corner
     */
    void set_edge(const std::string& edge_file);

    /// Returns this backdrop's edge file.
    /** \return This backdrop's edge file
     */
    const std::string& get_edge_file() const;

    /// Sets the edge color.
    /** \param c The edge color
     *   \note This color can be used to tint the edge texture if any
     *         or simply render a plain color edge.
     */
    void set_edge_color(const color& c);

    /// Returns the edge color.
    /** \return The edge color
     */
    color get_edge_color() const;

    /// Sets the appearent edge size.
    /** \param edge_size The new edge size
     *   \note Edges will be scaled by edge_size/edge_texture_height.
     */
    void set_edge_size(float edge_size);

    /// Returns this backdrop's edge size.
    /** \return This backdrop's edge size
     */
    float get_edge_size() const;

    /// Sets the color to be multiplied to all drawn vertices.
    /** \param c The new vertex color
     */
    void set_vertex_color(const color& c);

    /// Renders this backdrop on the current render target.
    void render() const;

    /// Tells this backdrop that its parent frame has changed dimensions.
    void notify_borders_updated() const;

private:
    void update_cache_() const;
    void update_background_(color c) const;
    void update_edge_(color c) const;

    frame& parent_;

    std::string               background_file_;
    color                     background_color_ = color::empty;
    std::shared_ptr<material> background_texture_;
    bool                      is_background_tilling_ = false;
    float                     tile_size_             = 0.0f;
    float                     original_tile_size_    = 0.0f;
    bounds2f                  background_insets_;

    std::string               edge_file_;
    color                     edge_color_ = color::empty;
    std::shared_ptr<material> edge_texture_;
    bounds2f                  edge_insets_;
    float                     edge_size_          = 0.0f;
    float                     original_edge_size_ = 0.0f;

    color vertex_color_ = color::white;

    mutable bool  is_cache_dirty_ = true;
    mutable float cache_alpha_    = std::numeric_limits<float>::quiet_NaN();
    mutable std::vector<std::array<vertex, 4>> background_quads_;
    mutable std::shared_ptr<vertex_cache>      background_cache_;
    mutable std::vector<std::array<vertex, 4>> edge_quads_;
    mutable std::shared_ptr<vertex_cache>      edge_cache_;
};

} // namespace lxgui::gui

#endif
