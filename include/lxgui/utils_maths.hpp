#ifndef LXGUI_UTILS_MATHS_HPP
#define LXGUI_UTILS_MATHS_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

namespace lxgui { namespace utils {

/// Rounding method for points to pixels conversions.
enum class rounding_method {
    /// Equivalent to round()
    NEAREST,
    /// Equivalent to round() but only returns 0 if input is exactly 0
    NEAREST_NOT_ZERO,
    /// Equivalent to ceil()
    UP,
    /// Equivalent to floor()
    DOWN
};

/// Round a floating point value to a specific unit and using a specific rounding method.
/** \param fValue  The value to round
 *   \param fUnit   The rounding unit (e.g., if set to 2, the output must be a multiple of 2)
 *   \param mMethod The rounding method
 *   \return The rounded value
 */
float round(float fValue, float fUnit, rounding_method mMethod);

}} // namespace lxgui::utils

#endif
