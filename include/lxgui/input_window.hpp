#ifndef LXGUI_INPUT_WINDOW_HPP
#define LXGUI_INPUT_WINDOW_HPP

#include "lxgui/gui_vector2.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_signal.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui::input {

class source;

/// Represents the window in which the UI is displayed
class window {
public:
    /**
     * \brief Initializes this window with a chosen input source.
     * \param src The input source
     */
    explicit window(source& src);

    // Non-copiable, non-movable
    window(const window&) = delete;
    window(window&&)      = delete;
    window& operator=(const window&) = delete;
    window& operator=(window&&) = delete;

    /**
     * \brief Retrieve a copy of the clipboard content.
     * \return A copy of the clipboard content (empty string is clipboard is empty).
     */
    utils::ustring get_clipboard_content();

    /**
     * \brief Replace the content of the clipboard.
     * \param content The new clipboard content
     */
    void set_clipboard_content(const utils::ustring& content);

    /**
     * \brief Sets the mouse cursor to a given image on disk.
     * \param file_name The cursor image
     * \param hot_spot The pixel position of the tip of the pointer in the image
     * \note Use reset_mouse_cursor() to set the cursor back to the default.
     */
    void set_mouse_cursor(const std::string& file_name, const gui::vector2i& hot_spot);

    /// Sets the mouse cursor back to the default (arrow).
    void reset_mouse_cursor();

    /**
     * \brief Return the interface scaling factor suggested by the operating system.
     * \return The interface scaling factor suggested by the operating system
     * \note This is implementation-dependent; not all input implementations are able
     * to produce this hint, in which case the function always returns 1.
     * Consequently, it is recommended to not rely blindly on this hint, and
     * to offer a way for the user to change the scaling factor.
     */
    float get_interface_scaling_factor_hint() const;

    /**
     * \brief Get the window size (in pixels)
     * \return The window size
     */
    const gui::vector2ui& get_dimensions() const;

    /**
     * \brief Returns the input source.
     * \return The input source
     */
    const source& get_source() const;

    /**
     * \brief Returns the input source.
     * \return The input source
     */
    source& get_source();

    /**
     * \brief Signal triggered whenever the window is resized or changes resolution.
     * \details Arguments:
     *  - New size of the window, in pixels
     */
    utils::signal<void(const gui::vector2ui&)> on_window_resized;

private:
    source&                  source_;
    utils::scoped_connection connection_;
};

} // namespace lxgui::input

#endif
