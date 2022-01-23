#ifndef LXGUI_INPUT_WORLD_DISPATCHER_HPP
#define LXGUI_INPUT_WORLD_DISPATCHER_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/gui_vector2.hpp"
#include "lxgui/utils_signal.hpp"

namespace lxgui {

namespace input
{
    /// Generates input events for the world, after filtering by the UI.
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
