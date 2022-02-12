#include "lxgui/utils_maths.hpp"

#include <algorithm>
#include <cmath>

namespace lxgui::utils {

float round(float f_value, float f_unit, rounding_method m_method) {
    switch (m_method) {
    case rounding_method::nearest: return std::round(f_value / f_unit) * f_unit;
    case rounding_method::nearest_not_zero:
        if (f_value > 0.0f)
            return std::max(1.0f, std::round(f_value / f_unit) * f_unit);
        else if (f_value < 0.0f)
            return std::min(-1.0f, std::round(f_value / f_unit) * f_unit);
        else
            return 0.0f;
    case rounding_method::up: return std::ceil(f_value / f_unit) * f_unit;
    case rounding_method::down: return std::floor(f_value / f_unit) * f_unit;
    default: return std::round(f_value / f_unit) * f_unit;
    }
}

} // namespace lxgui::utils
