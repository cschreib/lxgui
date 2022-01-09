#ifndef LXGUI_INPUT_SOURCE_HPP
#define LXGUI_INPUT_SOURCE_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/input_keys.hpp"
#include "lxgui/gui_vector2.hpp"

#include "lxgui/utils.hpp"
#include "lxgui/utils_string.hpp"

#include <string>
#include <vector>
#include <array>

namespace lxgui {
namespace input
{
    /// The base class for input source implementation
    /** \note The implementation is responsible for generating the
    *         following events:
    *          - MOUSE_MOVED:
    *              float: dx, float: dy, float: x, float: y
    *          - MOUSE_WHEEL:
    *              float: dx
    *          - MOUSE_PRESSED, MOUSE_RELEASED, MOUSE_DOUBLE_CLICKED:
    *              uint32: button, float: x, float: y
    *          - KEY_PRESSED, KEY_RELEASED:
    *              uint32: button
    *          - TEXT_ENTERED:
    *              uint32: character
    *          - WINDOW_RESIZED:
    *              uint32: width, uint32: height
    */
    class source : public gui::event_emitter
    {
    public :

        struct key_state
        {
            std::array<bool, KEY_NUMBER> lKeyState = {};
        };

        struct mouse_state
        {
            std::array<bool, MOUSE_BUTTON_NUMBER> lButtonState = {};
            gui::vector2f mPosition;
            float fWheel = 0.0f;
        };

        /// Constructor.
        source() = default;

        /// Destructor.
        virtual ~source() = default;

        // Non-copiable, non-movable
        source(const source&) = delete;
        source(source&&) = delete;
        source& operator=(const source&) = delete;
        source& operator=(source&&) = delete;

        /// Returns the keyboard state of this input source.
        const key_state& get_key_state() const;

        /// Returns the mouse state of this input source.
        const mouse_state& get_mouse_state() const;

        /// Get the window size (in pixels)
        /** \return The window size
        */
        const gui::vector2ui& get_window_dimensions() const;

        /// Sets the double click maximum time.
        /** \param dDoubleClickTime Maximum amount of time between two clicks in a double click
        */
        void set_doubleclick_time(double dDoubleClickTime);

        /// Returns the double click maximum time.
        /** \return The double click maximum time
        */
        double get_doubleclick_time() const;

        /// Retrieve a copy of the clipboard content.
        /** \return A copy of the clipboard content (empty string if clipboard is empty).
        */
        virtual utils::ustring get_clipboard_content() = 0;

        /// Replace the content of the clipboard.
        /** \param sContent The new clipboard content
        */
        virtual void set_clipboard_content(const utils::ustring& sContent) = 0;

        /// Sets the mouse cursor to a given image on disk.
        /** \param sFileName The cursor image
        *   \param mHotSpot The pixel position of the tip of the pointer in the image
        *   \note Use reset_mouse_cursor() to set the cursor back to the default.
        */
        virtual void set_mouse_cursor(const std::string& sFileName, const gui::vector2i& mHotSpot) = 0;

        /// Sets the mouse cursor back to the default (arrow).
        virtual void reset_mouse_cursor() = 0;

        /// Return the interface scaling factor suggested by the operating system.
        /** \return The interface scaling factor suggested by the operating system
        *   \note This is implementation-dependent; not all input implementations are able
        *         to produce this hint, in which case the function always returns 1.
        *         Consequently, it is recommended to not rely blindly on this hint, and
        *         to offer a way for the user to change the scaling factor. But this can be used
        *         for a good default value.
        */
        virtual float get_interface_scaling_factor_hint() const;

    protected:

        key_state   mKeyboard_;
        mouse_state mMouse_;

        gui::vector2ui mWindowDimensions_;

        double dDoubleClickTime_ = 0.25;
    };
}
}

#endif
