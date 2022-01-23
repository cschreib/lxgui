#ifndef LXGUI_INPUT_WORLD_DISPATCHER_HPP
#define LXGUI_INPUT_WORLD_DISPATCHER_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/gui_vector2.hpp"
#include "lxgui/utils_signal.hpp"

namespace lxgui {

namespace input
{
    /// Generates input events for the world, after filtering by the UI.
    /** The implementation is responsible for generating the
    *   following events:
    *    - @ref on_mouse_moved
    *    - @ref on_mouse_wheel
    *    - @ref on_mouse_pressed
    *    - @ref on_mouse_released
    *    - @ref on_mouse_double_clicked
    *    - @ref on_mouse_drag_start
    *    - @ref on_mouse_drag_stop
    *    - @ref on_key_pressed
    *    - @ref on_key_released
    *
    *   These events will only trigger if not captured by any UI element,
    *   and are therefore suitable for "world" input (i.e., input for the game
    *   elements rendered below the UI). These events are triggered by @ref gui::uiroot,
    *   which takes care of filtering global inputs from the @ref input::dispatcher.
    */
    class world_dispatcher
    {
    public :

        /// Default constructor.
        world_dispatcher() = default;

        // Non-movable, non-copiable
        world_dispatcher(const world_dispatcher&) = delete;
        world_dispatcher(world_dispatcher&&) = delete;
        world_dispatcher& operator=(const world_dispatcher&) = delete;
        world_dispatcher& operator=(world_dispatcher&&) = delete;

        /// Signal triggered when the mouse moves
        utils::signal<void(const gui::vector2f&, const gui::vector2f&)> on_mouse_moved;
        /// Signal triggered when the mouse wheel is moved
        utils::signal<void(float, const gui::vector2f&)>                on_mouse_wheel;
        /// Signal triggered when a mouse button is pressed
        utils::signal<void(input::mouse_button, const gui::vector2f&)>  on_mouse_pressed;
        /// Signal triggered when a mouse button is released
        utils::signal<void(input::mouse_button, const gui::vector2f&)>  on_mouse_released;
        /// Signal triggered when a mouse button is double clicked
        utils::signal<void(input::mouse_button, const gui::vector2f&)>  on_mouse_double_clicked;
        /// Signal triggered when the mouse starts a drag operation
        utils::signal<void(input::mouse_button, const gui::vector2f&)>  on_mouse_drag_start;
        /// Signal triggered when the mouse ends a drag operation
        utils::signal<void(input::mouse_button, const gui::vector2f&)>  on_mouse_drag_stop;
        /// Signal triggered when a keyboard key is pressed
        utils::signal<void(input::key)>                                 on_key_pressed;
        /// Signal triggered when a keyboard key is released
        utils::signal<void(input::key)>                                 on_key_released;
    };
}
}


#endif
