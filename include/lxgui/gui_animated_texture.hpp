#ifndef LXGUI_GUI_ANIMATED_TEXTURE_HPP
#define LXGUI_GUI_ANIMATED_TEXTURE_HPP

#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

namespace lxgui::gui {

class renderer;

/**
 * \brief A layered_region that can draw animated sequences.
 * \details This object contains an animated texture taken from a file.
 */
class animated_texture : public layered_region {
    using base = layered_region;

public:
    /// Constructor.
    explicit animated_texture(
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
     * \brief Updates this region's logic.
     * \param delta Time spent since last update
     */
    void update(float delta) override;

    /**
     * \brief Copies a region's parameters into this texture (inheritance).
     * \param obj The region to copy
     */
    void copy_from(const region& obj) override;

    /**
     * \brief Returns this animated_texture's animation speed (frame per second).
     * \return This animated_texture's animation speed
     */
    float get_speed() const;

    /**
     * \brief Returns this animated_texture's state (0: begin, 1: end).
     * \return This animated_texture's state
     */
    float get_state() const;

    /**
     * \brief Check if this animated_texture is paused
     * \return 'true' if paused, 'false' otherwise
     */
    float is_paused() const;

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
     * \brief Set this animated_texture's animation speed (frame per second).
     * \param speed The new animation speed
     */
    void set_speed(float speed);

    /**
     * \brief Returns this animated_texture's state (0: begin, 1: end).
     * \param state The new state
     */
    void set_state(float state);

    /**
     * \brief Check if this animated_texture is paused
     * \return 'true' if paused, 'false' otherwise
     */
    void set_paused(bool is_paused);

    /**
     * \brief Sets this texture's texture file.
     * \param file_name The file from which to read data
     * \note This function takes care of checking that the file can be opened.
     * \note This function will replace the solid color set by set_solid_color(). If you need
     * to blend the texture with a color, use set_vertex_color() instead.
     */
    void set_texture(const std::string& file_name);

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

    static constexpr const char* class_name = "AnimatedTexture";

private:
    void parse_attributes_(const layout_node& node) override;
    void parse_tex_coords_node_(const layout_node& node);
    void parse_gradient_node_(const layout_node& node);

    void update_tex_coords_();
    void update_borders_() override;

    std::string file_;

    float speed_     = 1.0f;
    float state_     = 0.0f;
    bool  is_paused_ = false;

    renderer& renderer_;
    quad      quad_;
};

} // namespace lxgui::gui

#endif
