#include "lxgui/utils_maths.hpp"

#include <algorithm>
#include <cmath>

namespace lxgui::utils {

float round(float value, float unit, rounding_method method) {
    switch (method) {
    case rounding_method::nearest: return std::round(value / unit) * unit;
    case rounding_method::nearest_not_zero:
        if (value > 0.0f)
            return std::max(1.0f, std::round(value / unit) * unit);
        else if (value < 0.0f)
            return std::min(-1.0f, std::round(value / unit) * unit);
        else
            return 0.0f;
    case rounding_method::up: return std::ceil(value / unit) * unit;
    case rounding_method::down: return std::floor(value / unit) * unit;
    default: return std::round(value / unit) * unit;
    }
}

} // namespace lxgui::utils
