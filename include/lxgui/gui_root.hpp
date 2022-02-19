#ifndef LXGUI_GUI_UIROOT_HPP
#define LXGUI_GUI_UIROOT_HPP

#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_frame_container.hpp"
#include "lxgui/gui_framerenderer.hpp"
#include "lxgui/gui_keybinder.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/gui_vector2.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_observer.hpp"
#include "lxgui/utils_signal.hpp"

#include <list>
#include <memory>

namespace lxgui::input {

class world_dispatcher;

}

namespace lxgui::gui {

class region;
class frame;
class manager;
class renderer;

/// Root of the UI object hierarchy.
/** This class contains and owns all "root" frames (frames with no parents)
 *   and is responsible for their lifetime, update, and rendering.
 */
class root :
    public frame_renderer,
    public frame_container,
    public utils::enable_observer_from_this<root> {
public:
    /// Constructor.
    /** \param block   The owner pointer control block
     *   \param mgr The GUI manager
     */
    explicit root(utils::control_block& block, manager& mgr);

    /// Destructor.
    ~root() override;

    // Non-copiable, non-movable
    root(const root&) = delete;
    root(root&&)      = delete;
    root& operator=(const root&) = delete;
    root& operator=(root&&) = delete;

    /// Returns the width and height of this renderer's main render target (e.g., screen).
    /** \return The render target dimensions
     */
    vector2f get_target_dimensions() const override;

    /// Renders the UI into the current render target.
    void render() const;

    /// Enables or disables interface caching.
    /** \param enable 'true' to enable, 'false' to disable
     *   \see toggle_caching()
     */
    void enable_caching(bool enable);

    /// Toggles interface caching.
    /** \note Disabled by default. Enabling this will most likely improve performances,
     *         at the expense of higher GPU memory usage. The UI will be cached into
     *         large render targets, which are only redrawn when the UI changes, rather
     *         than redrawn on each frame.
     */
    void toggle_caching();

    /// Checks if interface caching is enabled.
    /** \return 'true' if interface caching is enabled
     *   \see toggle_caching()
     */
    bool is_caching_enabled() const;

    /// updates this root and its regions.
    /** \param delta The time elapsed since the last call
     */
    void update(float delta);

    /// Tells this object that the global interface scaling factor has changed.
    void notify_scaling_factor_updated();

    /// Notifies the root that it should update the hovered frame.
    void notify_hovered_frame_dirty();

    /// Returns the currently hovered frame, if any.
    /** \return The currently hovered frame, if any.
     */
    const utils::observer_ptr<frame>& get_hovered_frame() {
        return p_hovered_frame_;
    }

    /// Returns the currently hovered frame, if any.
    /** \return The currently hovered frame, if any.
     */
    utils::observer_ptr<const frame> get_hovered_frame() const {
        return p_hovered_frame_;
    }

    /// Check if a given frame is being hovered.
    /** \return 'true' if hovered, 'false' otherwise
     */
    bool is_hovered(const frame& obj) const {
        return p_hovered_frame_.get() == &obj;
    }

    /// Returns the currently dragged frame, if any.
    /** \return The currently dragged frame, if any.
     */
    const utils::observer_ptr<frame>& get_dragged_frame() {
        return p_dragged_frame_;
    }

    /// Returns the currently dragged frame, if any.
    /** \return The currently dragged frame, if any.
     */
    utils::observer_ptr<const frame> get_dragged_frame() const {
        return p_dragged_frame_;
    }

    /// Check if a given frame is being dragged.
    /** \return 'true' if dragged, 'false' otherwise
     */
    bool is_dragged(const frame& obj) const {
        return p_dragged_frame_.get() == &obj;
    }

    /// Start manually moving a region with the mouse.
    /** \param pObj        The object to move
     *   \param pAnchor     The reference anchor
     *   \param constraint The constraint axis if any
     *   \param apply_constraint_func Optional function to implement further constraints
     *   \note Movement is handled by the root, you don't need to do anything except
     *         calling stop_moving() when you are done.
     */
    void start_moving(
        utils::observer_ptr<region> p_obj,
        anchor*                     p_anchor              = nullptr,
        constraint                  constraint            = constraint::none,
        std::function<void()>       apply_constraint_func = nullptr);

    /// Stops movement for the current object.
    /** \note Does nothing if no object is being moved
     */
    void stop_moving();

    /// Checks if the given object is allowed to move.
    /** \param obj The object to check
     *   \return 'true' if the given object is allowed to move
     */
    bool is_moving(const region& obj) const;

    /// Starts manually resizing a region with the mouse.
    /** \param pObj   The object to resize
     *   \param point The sizing point
     *   \note Resizing is handled by the root, you don't need to do anything except
     *         calling stop_sizing() when you are done.
     */
    void start_sizing(utils::observer_ptr<region> p_obj, anchor_point point);

    /// Stops sizing for the current object.
    /** \note Does nothing if no object is being resized
     */
    void stop_sizing();

    /// Checks if the given object is allowed to be resized.
    /** \param obj The object to check
     *   \return 'true' if the given object is allowed to be resized
     */
    bool is_sizing(const region& obj) const;

    /// Sets whether keyboard input should be focussed.
    /** \param pReceiver The frame that requires focus
     *   \note This function will forward all keyboard events to the new receiver.
     *         This is usefull to implement an edit box: the user can type letters using keys
     *         that can be bound to special actions in the game, and these should be prevented
     *         from happening. This can be achieved by calling this function and using the
     *         edit box as second argument, which will ensure that input events are only sent
     *         to the edit box exclusively.
     */
    void request_focus(utils::observer_ptr<frame> p_receiver);

    /// Give up focus of keyboard input.
    /** \param receiver The event receiver that releases focus
     */
    void release_focus(const frame& receiver);

    /// Release all requested focus.
    void clear_focus();

    /// Checks whether keyboard input is focused somewhere, to prevent multiple inputs.
    /** \return 'true' if input is focused
     *   \note See set_focus() for more information.
     */
    bool is_focused() const;

    /// Returns the currently focussed frame (nullptr if none).
    /** \return The currently focussed frame (nullptr if none)
     */
    utils::observer_ptr<const frame> get_focussed_frame() const;

    /// Returns the currently focussed frame (nullptr if none).
    /** \return The currently focussed frame (nullptr if none)
     */
    utils::observer_ptr<frame> get_focussed_frame() {
        return utils::const_pointer_cast<frame>(
            const_cast<const root*>(this)->get_focussed_frame());
    }

    /// Returns the manager instance associated with this root.
    /** \return The manager instance associated with this root
     */
    manager& get_manager() {
        return manager_;
    }

    /// Returns the manager instance associated with this root.
    /** \return The manager instance associated with this root
     */
    const manager& get_manager() const {
        return manager_;
    }

    /// Returns the UI object registry, which keeps track of all objects in the UI.
    /** \return The registry object
     */
    registry& get_registry() {
        return object_registry_;
    }

    /// Returns the UI object registry, which keeps track of all objects in the UI.
    /** \return The registry object
     */
    const registry& get_registry() const {
        return object_registry_;
    }

    /// Returns the keybinder object, which enables binding global actions to key presses.
    /** \return The keybinder object
     */
    keybinder& get_keybinder() {
        return keybinder_;
    }

    /// Returns the keybinder object, which enables binding global actions to key presses.
    /** \return The keybinder object
     */
    const keybinder& get_keybinder() const {
        return keybinder_;
    }

private:
    void create_caching_render_target_();
    void create_strata_cache_render_target_(strata& strata_obj);

    void clear_hovered_frame_();
    void update_hovered_frame_();
    void set_hovered_frame_(
        utils::observer_ptr<frame> p_frame, const vector2f& mouse_pos = vector2f::zero);

    void on_window_resized_(const vector2ui& dimensions);
    void on_mouse_moved_(const vector2f& movement, const vector2f& mouse_pos);
    void on_mouse_wheel_(float wheel_scroll, const vector2f& mouse_pos);
    void on_drag_start_(input::mouse_button button, const vector2f& mouse_pos);
    void on_drag_stop_(input::mouse_button button, const vector2f& mouse_pos);
    void on_text_entered_(std::uint32_t c);
    void on_key_state_changed_(input::key key, bool is_down);
    void on_mouse_button_state_changed_(
        input::mouse_button button, bool is_down, bool is_double_click, const vector2f& mouse_pos);

    manager&                 manager_;
    renderer&                renderer_;
    registry                 object_registry_;
    keybinder                keybinder_;
    input::world_dispatcher& world_input_dispatcher_;

    // Rendering
    vector2ui screen_dimensions_;

    bool caching_enabled_ = false;

    std::shared_ptr<render_target> p_render_target_;
    quad                           screen_quad_;

    // IO
    std::vector<utils::scoped_connection> connections_;

    // Mouse IO
    utils::observer_ptr<frame> p_hovered_frame_ = nullptr;
    utils::observer_ptr<frame> p_dragged_frame_ = nullptr;

    utils::observer_ptr<region> p_moved_object_ = nullptr;
    utils::observer_ptr<region> p_sized_object_ = nullptr;
    vector2f                    mouse_movement_;

    anchor*               p_moved_anchor_ = nullptr;
    vector2f              movement_start_position_;
    constraint            constraint_ = constraint::none;
    std::function<void()> apply_constraint_func_;

    vector2f resize_start_;
    bool     is_resizing_width_       = false;
    bool     is_resizing_height_      = false;
    bool     is_resizing_from_right_  = false;
    bool     is_resizing_from_bottom_ = false;

    // Keyboard IO
    std::vector<utils::observer_ptr<frame>> focus_stack_;
};

} // namespace lxgui::gui

#endif
