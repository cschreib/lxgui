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
    /** \param mBlock   The owner pointer control block
     *   \param mManager The GUI manager
     */
    explicit root(utils::control_block& mBlock, manager& mManager);

    /// Destructor.
    ~root() override;

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
    /** \param bEnable 'true' to enable, 'false' to disable
     *   \see toggle_caching()
     */
    void enable_caching(bool bEnable);

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
    /** \param fDelta The time elapsed since the last call
     */
    void update(float fDelta);

    /// Tells this object that the global interface scaling factor has changed.
    void notify_scaling_factor_updated();

    /// Notifies the root that it should update the hovered frame.
    void notify_hovered_frame_dirty();

    /// Returns the currently hovered frame, if any.
    /** \return The currently hovered frame, if any.
     */
    const utils::observer_ptr<frame>& get_hovered_frame() {
        return pHoveredFrame_;
    }

    /// Returns the currently hovered frame, if any.
    /** \return The currently hovered frame, if any.
     */
    utils::observer_ptr<const frame> get_hovered_frame() const {
        return pHoveredFrame_;
    }

    /// Check if a given frame is being hovered.
    /** \return 'true' if hovered, 'false' otherwise
     */
    bool is_hovered(const frame& mFrame) const {
        return pHoveredFrame_.get() == &mFrame;
    }

    /// Returns the currently dragged frame, if any.
    /** \return The currently dragged frame, if any.
     */
    const utils::observer_ptr<frame>& get_dragged_frame() {
        return pDraggedFrame_;
    }

    /// Returns the currently dragged frame, if any.
    /** \return The currently dragged frame, if any.
     */
    utils::observer_ptr<const frame> get_dragged_frame() const {
        return pDraggedFrame_;
    }

    /// Check if a given frame is being dragged.
    /** \return 'true' if dragged, 'false' otherwise
     */
    bool is_dragged(const frame& mFrame) const {
        return pDraggedFrame_.get() == &mFrame;
    }

    /// Start manually moving a region with the mouse.
    /** \param pObj        The object to move
     *   \param pAnchor     The reference anchor
     *   \param mConstraint The constraint axis if any
     *   \param mApplyConstraintFunc Optional function to implement further constraints
     *   \note Movement is handled by the root, you don't need to do anything except
     *         calling stop_moving() when you are done.
     */
    void start_moving(
        utils::observer_ptr<region> pObj,
        anchor*                     pAnchor              = nullptr,
        constraint                  mConstraint          = constraint::NONE,
        std::function<void()>       mApplyConstraintFunc = nullptr);

    /// Stops movement for the current object.
    /** \note Does nothing if no object is being moved
     */
    void stop_moving();

    /// Checks if the given object is allowed to move.
    /** \param mObj The object to check
     *   \return 'true' if the given object is allowed to move
     */
    bool is_moving(const region& mObj) const;

    /// Starts manually resizing a region with the mouse.
    /** \param pObj   The object to resize
     *   \param mPoint The sizing point
     *   \note Resizing is handled by the root, you don't need to do anything except
     *         calling stop_sizing() when you are done.
     */
    void start_sizing(utils::observer_ptr<region> pObj, anchor_point mPoint);

    /// Stops sizing for the current object.
    /** \note Does nothing if no object is being resized
     */
    void stop_sizing();

    /// Checks if the given object is allowed to be resized.
    /** \param mObj The object to check
     *   \return 'true' if the given object is allowed to be resized
     */
    bool is_sizing(const region& mObj) const;

    /// Sets whether keyboard input should be focussed.
    /** \param pReceiver The frame that requires focus
     *   \note This function will forward all keyboard events to the new receiver.
     *         This is usefull to implement an edit box: the user can type letters using keys
     *         that can be bound to special actions in the game, and these should be prevented
     *         from happening. This can be achieved by calling this function and using the
     *         edit box as second argument, which will ensure that input events are only sent
     *         to the edit box exclusively.
     */
    void request_focus(utils::observer_ptr<frame> pReceiver);

    /// Give up focus of keyboard input.
    /** \param mReceiver The event receiver that releases focus
     */
    void release_focus(const frame& mReceiver);

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
        return mManager_;
    }

    /// Returns the manager instance associated with this root.
    /** \return The manager instance associated with this root
     */
    const manager& get_manager() const {
        return mManager_;
    }

    /// Returns the UI object registry, which keeps track of all objects in the UI.
    /** \return The registry object
     */
    registry& get_registry() {
        return mObjectRegistry_;
    }

    /// Returns the UI object registry, which keeps track of all objects in the UI.
    /** \return The registry object
     */
    const registry& get_registry() const {
        return mObjectRegistry_;
    }

    /// Returns the keybinder object, which enables binding global actions to key presses.
    /** \return The keybinder object
     */
    keybinder& get_keybinder() {
        return mKeybinder_;
    }

    /// Returns the keybinder object, which enables binding global actions to key presses.
    /** \return The keybinder object
     */
    const keybinder& get_keybinder() const {
        return mKeybinder_;
    }

private:
    void create_caching_render_target_();
    void create_strata_cache_render_target_(strata& mStrata);

    void clear_hovered_frame_();
    void update_hovered_frame_();
    void set_hovered_frame_(
        utils::observer_ptr<frame> pFrame, const vector2f& mMousePos = vector2f::ZERO);

    void on_window_resized_(const vector2ui& mDimensions);
    void on_mouse_moved_(const vector2f& mMovement, const vector2f& mMousePos);
    void on_mouse_wheel_(float fWheelScroll, const vector2f& mMousePos);
    void on_drag_start_(input::mouse_button mButton, const vector2f& mMousePos);
    void on_drag_stop_(input::mouse_button mButton, const vector2f& mMousePos);
    void on_text_entered_(std::uint32_t uiChar);
    void on_key_state_changed_(input::key mKey, bool bIsDown);
    void on_mouse_button_state_changed_(
        input::mouse_button mButton, bool bIsDown, bool bIsDoubleClick, const vector2f& mMousePos);

    manager&                 mManager_;
    renderer&                mRenderer_;
    registry                 mObjectRegistry_;
    keybinder                mKeybinder_;
    input::world_dispatcher& mWorldInputDispatcher_;

    // Rendering
    vector2ui mScreenDimensions_;

    bool bEnableCaching_ = false;

    std::shared_ptr<render_target> pRenderTarget_;
    quad                           mScreenQuad_;

    // IO
    std::vector<utils::scoped_connection> lConnections_;

    // Mouse IO
    utils::observer_ptr<frame> pHoveredFrame_ = nullptr;
    utils::observer_ptr<frame> pDraggedFrame_ = nullptr;

    utils::observer_ptr<region> pMovedObject_ = nullptr;
    utils::observer_ptr<region> pSizedObject_ = nullptr;
    vector2f                    mMouseMovement_;

    anchor*               pMovedAnchor_ = nullptr;
    vector2f              mMovementStartPosition_;
    constraint            mConstraint_ = constraint::NONE;
    std::function<void()> mApplyConstraintFunc_;

    vector2f mResizeStart_;
    bool     bResizeWidth_      = false;
    bool     bResizeHeight_     = false;
    bool     bResizeFromRight_  = false;
    bool     bResizeFromBottom_ = false;

    // Keyboard IO
    std::vector<utils::observer_ptr<frame>> lFocusStack_;
};

} // namespace lxgui::gui

#endif
