#ifndef LXGUI_GUI_GRADIENT_HPP
#define LXGUI_GUI_GRADIENT_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_color.hpp"

namespace lxgui {
namespace gui
{
    /// Represents color gradients
    class gradient
    {
    public :

        enum class orientation
        {
            HORIZONTAL,
            VERTICAL
        };

        /// Default constructor.
        /** \note Makes an empty gradient.
        */
        gradient() = default;

        /// Constructor.
        /** \param mOrientation This gradient's orientation
        *   \param mMinColor    This gradient's min color
        *   \param mMaxColor    This gradient's max color
        */
        gradient(orientation mOrientation, const color& mMinColor, const color& mMaxColor);

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

    private :

        orientation mOrientation_ = orientation::HORIZONTAL;
        color       mMinColor_, mMaxColor_;

    };
}
}

#endif
