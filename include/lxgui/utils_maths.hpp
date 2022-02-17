#ifndef LXGUI_UTILS_MATHS_HPP
#define LXGUI_UTILS_MATHS_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

namespace lxgui::utils {

/// Rounding method for points to pixels conversions.
enum class rounding_method {
    /// Equivalent to round()
    nearest,
    /// Equivalent to round() but only returns 0 if input is exactly 0
    nearest_not_zero,
    /// Equivalent to ceil()
    up,
    /// Equivalent to floor()
    down
};

/// Round a floating point value to a specific unit and using a specific rounding method.
/** \param value  The value to round
 *   \param fUnit   The rounding unit (e.g., if set to 2, the output must be a multiple of 2)
 *   \param mMethod The rounding method
 *   \return The rounded value
 */
float round(float value, float unit, rounding_method m_method);

} // namespace lxgui::utils

#endif
