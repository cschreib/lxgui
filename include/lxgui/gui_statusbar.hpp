#ifndef LXGUI_GUI_STATUSBAR_HPP
#define LXGUI_GUI_STATUSBAR_HPP

#include "lxgui/gui_color.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

namespace lxgui::gui {

class texture;

/// A #frame representing a variable-length bar.
/** This frame has three main properties: a minimum value, a
 *   maximum value, and a current value that must be contained
 *   between the minimum and maximum values. The frame will
 *   render a textured bar that will either be full, empty, or
 *   anything in between depending on the current value.
 *
 *   This can be used to display health bars, or progress bars.
 *
 *   __Events.__ Hard-coded events available to all status bars,
 *   in addition to those from #frame:
 *
 *   - `OnValueChanged`: Triggered whenever the value represented by
 *   the status bar changes. This is triggered by status_bar::set_value.
 *   This can also be triggered by status_bar::set_min_max_values if
 *   the previous value would not satisfy the new constraints.
 */
class status_bar : public frame {
    using base = frame;

public:
    enum class orientation { horizontal, vertical };

    /// Constructor.
    explicit status_bar(utils::control_block& m_block, manager& m_manager);

    /// Prints all relevant information about this region in a string.
    /** \param tab The offset to give to all lines
     *   \return All relevant information about this region
     */
    std::string serialize(const std::string& tab) const override;

    /// Returns 'true' if this status_bar can use a script.
    /** \param script_name The name of the script
     *   \note This method can be overriden if needed.
     */
    bool can_use_script(const std::string& script_name) const override;

    /// Copies a region's parameters into this status_bar (inheritance).
    /** \param mObj The region to copy
     */
    void copy_from(const region& m_obj) override;

    /// Sets this status_bar's minimum value.
    /** \param fMin The minimum value
     */
    void set_min_value(float f_min);

    /// Sets this status_bar's maximum value.
    /** \param fMax The maximum value
     */
    void set_max_value(float f_max);

    /// Sets this status_bar's value range.
    /** \param fMin The minimum value
     *   \param fMax The maximum value
     */
    void set_min_max_values(float f_min, float f_max);

    /// Sets this status_bar's value.
    /** \param fValue The value
     */
    void set_value(float f_value);

    /// Sets the draw layer of this status_bar's bar texture.
    /** \param mBarLayer The layer
     */
    void set_bar_draw_layer(layer m_bar_layer);

    /// Sets the draw layer of this status_bar's bar texture.
    /** \param bar_layer_name The layer
     */
    void set_bar_draw_layer(const std::string& bar_layer_name);

    /// Sets this status_bar's bar texture.
    /** \param pBarTexture The bar texture
     */
    void set_bar_texture(utils::observer_ptr<texture> p_bar_texture);

    /// Sets this status_bar's bar color.
    /** \param mBarColor The bar color
     */
    void set_bar_color(const color& m_bar_color);

    /// Sets this status_bar's orientation.
    /** \param mOrientation The orientation
     */
    void set_orientation(orientation m_orientation);

    /// Sets this status_bar's orientation.
    /** \param orientation_name The orientation ("VERTICAL" or "HORIZONTAL")
     */
    void set_orientation(const std::string& orientation_name);

    /// Reverses this status_bar.
    /** \param bReversed 'true' to reverse it
     *   \note By default, if the status bar is oriented horizontally
     *         (vertically), if will grow from left to right (bottom to top).
     *         You can use this function to reverse the growth, that is
     *         make it grow from right to left.
     */
    void set_reversed(bool b_reversed);

    /// Returns this status_bar's minimum value.
    /** \return This status_bar's minimum value
     */
    float get_min_value() const;

    /// Returns this status_bar's maximum value.
    /** \return This status_bar's maximum value
     */
    float get_max_value() const;

    /// Returns this status_bar's value.
    /** \return This status_bar's value
     */
    float get_value() const;

    /// Returns the draw layer of status_bar's bar texture.
    /** \return The draw layer of status_bar's bar texture
     */
    layer get_bar_draw_layer() const;

    /// Returns this status_bar's bar texture.
    /** \return This status_bar's bar texture
     */
    const utils::observer_ptr<texture>& get_bar_texture() {
        return p_bar_texture_;
    }

    /// Returns this status_bar's bar texture.
    /** \return This status_bar's bar texture
     */
    utils::observer_ptr<const texture> get_bar_texture() const {
        return p_bar_texture_;
    }

    /// Returns this status_bar's bar color.
    /** \return This status_bar's bar color
     */
    const color& get_bar_color() const;

    /// Returns this status_bar's orientation.
    /** \return This status_bar's orientation
     */
    orientation get_orientation() const;

    /// Checks if this status_bar is reversed.
    /** \return 'true' if it is the case
     */
    bool is_reversed() const;

    /// Returns this region's Lua glue.
    void create_glue() override;

    /// Updates this region's logic.
    /** \param fDelta Time spent since last update
     *   \note Triggered callbacks could destroy the frame. If you need
     *         to use the frame again after calling this function, use
     *         the helper class alive_checker.
     */
    void update(float f_delta) override;

    /// Registers this region class to the provided Lua state
    static void register_on_lua(sol::state& m_lua);

    static constexpr const char* class_name = "StatusBar";

protected:
    void create_bar_texture_();
    void notify_bar_texture_needs_update_();

    void parse_attributes_(const layout_node& m_node) override;
    void parse_all_nodes_before_children_(const layout_node& m_node) override;

    bool b_update_bar_texture_ = false;

    orientation m_orientation_ = orientation::horizontal;
    bool        b_reversed_    = false;

    float f_value_     = 0.0f;
    float f_min_value_ = 0.0f;
    float f_max_value_ = 1.0f;

    color                        m_bar_color_         = color::white;
    layer                        m_bar_layer_         = layer::artwork;
    utils::observer_ptr<texture> p_bar_texture_       = nullptr;
    std::array<float, 4>         initial_text_coords_ = {0.0f, 0.0f, 1.0f, 1.0f};
};

} // namespace lxgui::gui

#endif
