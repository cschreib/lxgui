#ifndef LXGUI_GUI_LAYERED_REGION_HPP
#define LXGUI_GUI_LAYERED_REGION_HPP

#include "lxgui/gui_region.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

namespace lxgui::gui {

/// ID of a layer for rendering inside a frame.
enum class layer {
    background  = 0,
    border      = 1,
    artwork     = 2,
    overlay     = 3,
    highlight   = 4,
    specialhigh = 5,
    enum_size
};

/**
 * \brief A #region that can be rendered in a layer.
 * \details Layered regions can display content on the screen (texture,
 * texts, 3D models, ...) and must be contained inside a layer,
 * within a #lxgui::gui::frame object. The frame will then render all
 * its layered regions, sorted by layers.
 *
 * Layered regions cannot themselves react to events; this
 * must be taken care of by the parent frame.
 */
class layered_region : public region {
    using base = region;

public:
    /// Constructor.
    explicit layered_region(
        utils::control_block& block, manager& mgr, const region_core_attributes& attr);

    /**
     * \brief Prints all relevant information about this region in a string.
     * \param tab The offset to give to all lines
     * \return All relevant information about this region
     */
    std::string serialize(const std::string& tab) const override;

    /// Creates the associated Lua glue.
    void create_glue() override;

    /**
     * \brief Removes this region from its parent and return an owning pointer.
     * \return An owning pointer to this region
     */
    utils::owner_ptr<region> release_from_parent() override;

    /**
     * \brief shows this region.
     * \note Its parent must be shown for it to appear on
     * the screen.
     */
    void show() override;

    /**
     * \brief hides this region.
     * \note All its children won't be visible on the screen
     * anymore, even if they are still marked as shown.
     */
    void hide() override;

    /**
     * \brief Checks if this region can be seen on the screen.
     * \return 'true' if this region can be seen on the screen
     */
    bool is_visible() const override;

    /**
     * \brief Returns this layered_region's draw layer.
     * \return this layered_region's draw layer
     */
    layer get_draw_layer() const;

    /**
     * \brief Sets this layered_region's draw layer.
     * \param layer_id The new layer
     */
    virtual void set_draw_layer(layer layer_id);

    /**
     * \brief Notifies the renderer of this region that it needs to be redrawn.
     * \note Automatically called by any shape changing function.
     */
    void notify_renderer_need_redraw() override;

    /**
     * \brief Parses data from a layout_node.
     * \param node The layout node
     */
    void parse_layout(const layout_node& node) override;

    /// Registers this region class to the provided Lua state
    static void register_on_lua(sol::state& lua);

    static constexpr const char* class_name = "LayeredRegion";

protected:
    void parse_attributes_(const layout_node& node) override;

    layer layer_ = layer::artwork;
};

} // namespace lxgui::gui

#endif
