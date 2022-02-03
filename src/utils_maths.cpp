#include "lxgui/utils_maths.hpp"

#include <cmath>
#include <algorithm>

namespace lxgui {
namespace utils
{

float round(float fValue, float fUnit, rounding_method mMethod)
{
    switch (mMethod)
    {
    case rounding_method::NEAREST:
        return std::round(fValue/fUnit)*fUnit;
    case rounding_method::NEAREST_NOT_ZERO:
        if (fValue > 0.0f)
            return std::max(1.0f, std::round(fValue/fUnit)*fUnit);
        else if (fValue < 0.0f)
            return std::min(-1.0f, std::round(fValue/fUnit)*fUnit);
        else
            return 0.0f;
    case rounding_method::UP:
        return std::ceil(fValue/fUnit)*fUnit;
    case rounding_method::DOWN:
        return std::floor(fValue/fUnit)*fUnit;
    default:
        return std::round(fValue/fUnit)*fUnit;
    }
}

}
}
