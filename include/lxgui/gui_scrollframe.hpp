#ifndef LXGUI_GUI_SCROLLFRAME_HPP
#define LXGUI_GUI_SCROLLFRAME_HPP

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_framerenderer.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

namespace lxgui::gui {

class texture;

/// A #frame with scrollable content.
/** This frame has a special child frame, the "scroll child". The scroll
 *   child is rendered on a separate render target, which is then rendered
 *   on the screen. This allows clipping the content of the scroll child
 *   and only display a portion of it (as if scrolling on a page). The
 *   displayed portion is controlled by the scroll value, which can be
 *   changed in both the vertical and horizontal directions.
 *
 *   By default, the mouse wheel movement will not trigger any scrolling;
 *   this has to be explicitly implemented using the `OnMouseWheel` callback
 *   and the scroll_frame::set_horizontal_scroll function.
 *
 *   __Events.__ Hard-coded events available to all scroll frames,
 *   in addition to those from #frame:
 *
 *   - `OnHorizontalScroll`: Triggered by scroll_frame::set_horizontal_scroll.
 *   - `OnScrollRangeChanged`: Triggered whenever the range of the scroll value
 *   changes. This happens either when the size of the scrollable content
 *   changes, or when the size of the scroll frame changes.
 *   - `OnVerticalScroll`: Triggered by scroll_frame::set_vertical_scroll.
 */
class scroll_frame : public frame, public frame_renderer {
    using base = frame;

public:
    /// Constructor.
    explicit scroll_frame(utils::control_block& m_block, manager& m_manager);

    /// Destructor.
    ~scroll_frame() override;

    /// Updates this region's logic.
    /** \param delta Time spent since last update
     *   \note Triggered callbacks could destroy the frame. If you need
     *         to use the frame again after calling this function, use
     *         the helper class alive_checker.
     */
    void update(float f_delta) override;

    /// Copies a region's parameters into this scroll_frame (inheritance).
    /** \param mObj The region to copy
     */
    void copy_from(const region& m_obj) override;

    /// Returns 'true' if this scroll_frame can use a script.
    /** \param script_name The name of the script
     *   \note This method can be overriden if needed.
     */
    bool can_use_script(const std::string& script_name) const override;

    /// Calls a script.
    /** \param script_name The name of the script
     *   \param mData       Stores scripts arguments
     *   \note Triggered callbacks could destroy the frame. If you need
     *         to use the frame again after calling this function, use
     *         the helper class alive_checker.
     */
    void
    fire_script(const std::string& script_name, const event_data& m_data = event_data{}) override;

    /// Sets this scroll_frame's scroll child.
    /** \param pFrame The scroll child
     *   \note Creates the render target.
     */
    void set_scroll_child(utils::owner_ptr<frame> p_frame);

    /// Returns this scroll_frame's scroll child.
    /** \return This scroll_frame's scroll child
     */
    const utils::observer_ptr<frame>& get_scroll_child() {
        return p_scroll_child_;
    }

    /// Returns this scroll_frame's scroll child.
    /** \return This scroll_frame's scroll child
     */
    utils::observer_ptr<const frame> get_scroll_child() const {
        return p_scroll_child_;
    }

    /// Sets the horizontal offset of the scroll child.
    /** \param fHorizontalScroll The horizontal offset
     */
    void set_horizontal_scroll(float f_horizontal_scroll);

    /// Returns the horizontal offset of the scroll child.
    /** \return The horizontal offset of the scroll child
     */
    float get_horizontal_scroll() const;

    /// Returns the maximum horizontal offset of the scroll child.
    /** \return The maximum horizontal offset of the scroll child
     */
    float get_horizontal_scroll_range() const;

    /// Sets the vertical offset of the scroll child.
    /** \param fVerticalScroll The vertical offset
     */
    void set_vertical_scroll(float f_vertical_scroll);

    /// Returns the vertical offset of the scroll child.
    /** \return The vertical offset of the scroll child
     */
    float get_vertical_scroll() const;

    /// Returns the maximum vertical offset of the scroll child.
    /** \return The maximum vertical offset of the scroll child
     */
    float get_vertical_scroll_range() const;

    /// Find the topmost frame matching the provided predicate
    /** \param mPredicate A function returning 'true' if the frame can be selected
     *   \return The topmost frame, if any, and nullptr otherwise.
     *   \note For most frames, this can either return 'this' or 'nullptr'. For
     *         frames responsible for rendering other frames (such as @ref scroll_frame),
     *         this can return other frames.
     *   \note For scroll children to receive input, the scroll_frame must be
     *         keyboard/mouse/wheel enabled.
     */
    utils::observer_ptr<const frame>
    find_topmost_frame(const std::function<bool(const frame&)>& m_predicate) const override;

    /// Tells this renderer that one of its region requires redraw.
    void notify_strata_needs_redraw(frame_strata m_strata) override;

    /// Tells this renderer that it should (or not) render another frame.
    /** \param pFrame    The frame to render
     *   \param rendered 'true' if this renderer needs to render that new object
     */
    void notify_rendered_frame(const utils::observer_ptr<frame>& p_frame, bool rendered) override;

    /// Returns the width and height of of this renderer's main render target (e.g., screen).
    /** \return The render target dimensions
     */
    vector2f get_target_dimensions() const override;

    /// Tells this region that the global interface scaling factor has changed.
    void notify_scaling_factor_updated() override;

    /// Returns this region's Lua glue.
    void create_glue() override;

    /// Registers this region class to the provided Lua state
    static void register_on_lua(sol::state& m_lua);

    static constexpr const char* class_name = "ScrollFrame";

protected:
    void         parse_all_nodes_before_children_(const layout_node& m_node) override;
    virtual void parse_scroll_child_node_(const layout_node& m_node);

    void update_scroll_range_();
    void rebuild_scroll_render_target_();
    void render_scroll_strata_list_();

    vector2f m_scroll_;
    vector2f m_scroll_range_;

    utils::observer_ptr<frame> p_scroll_child_ = nullptr;

    bool                           rebuild_scroll_render_target_flag_ = false;
    bool                           redraw_scroll_render_target_flag_  = false;
    bool                           update_scroll_range_flag_          = false;
    std::shared_ptr<render_target> p_scroll_render_target_;

    utils::observer_ptr<texture> p_scroll_texture_ = nullptr;
};

} // namespace lxgui::gui

#endif
