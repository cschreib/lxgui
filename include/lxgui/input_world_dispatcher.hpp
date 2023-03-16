#ifndef LXGUI_INPUT_WORLD_DISPATCHER_HPP
#define LXGUI_INPUT_WORLD_DISPATCHER_HPP

#include "lxgui/gui_vector2.hpp"
#include "lxgui/input_signals.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_signal.hpp"

namespace lxgui::input {

/**
 * \brief Generates input events for the world, after filtering by the UI.
 * \details The implementation is responsible for generating the
 * following events:
 *  - @ref on_mouse_moved
 *  - @ref on_mouse_wheel
 *  - @ref on_mouse_pressed
 *  - @ref on_mouse_released
 *  - @ref on_mouse_double_clicked
 *  - @ref on_mouse_drag_start
 *  - @ref on_mouse_drag_stop
 *  - @ref on_key_pressed
 *  - @ref on_key_pressed_repeat
 *  - @ref on_key_released
 *  - @ref on_text_entered
 *
 * These events will only trigger if not captured by any UI element,
 * and are therefore suitable for "world" input (i.e., input for the game
 * elements rendered below the UI). These events are triggered by @ref gui::root,
 * which takes care of filtering global inputs from the @ref input::dispatcher.
 */
class world_dispatcher : public signals {
public:
    /// Default constructor.
    world_dispatcher() = default;

    // Non-movable, non-copiable
    world_dispatcher(const world_dispatcher&) = delete;
    world_dispatcher(world_dispatcher&&)      = delete;
    world_dispatcher& operator=(const world_dispatcher&) = delete;
    world_dispatcher& operator=(world_dispatcher&&) = delete;
};

} // namespace lxgui::input

#endif
