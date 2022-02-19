#ifndef LXGUI_GUI_GRADIENT_HPP
#define LXGUI_GUI_GRADIENT_HPP

#include "lxgui/gui_color.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

namespace lxgui::gui {

/// Represents color gradients
class gradient {
public:
    enum class orientation { horizontal, vertical };

    /// Default constructor.
    /** \note Makes an empty gradient.
     */
    gradient() = default;

    /// Constructor.
    /** \param orientation This gradient's orientation
     *   \param min_color    This gradient's min color
     *   \param max_color    This gradient's max color
     */
    gradient(orientation orientation, const color& min_color, const color& max_color);

    /// Returns the gradient's min colors.
    /** \return The gradient's min colors
     *   \note In horizontal mode, this is the left color, and
     *         in vertical mode this is the top one.
     */
    const color& get_min_color() const;

    /// Returns the gradient's max colors.
    /** \return The gradient's max colors
     *   \note In horizontal mode, this is the right color, and
     *         in vertical mode this is the bottom one.
     */
    const color& get_max_color() const;

    /// Returns the gradient's orientation.
    /** \return The gradient's orientation
     */
    orientation get_orientation() const;

private:
    orientation orientation_ = orientation::horizontal;
    color       min_color_, max_color_;
};

} // namespace lxgui::gui

#endif
