#include "lxgui/gui_gradient.hpp"

namespace lxgui {
namespace gui
{
const gradient gradient::NONE = gradient();

gradient::gradient(orientation mOrientation, const color& mMinColor, const color& mMaxColor) :
    bIsEmpty_(false), mOrientation_(mOrientation), mMinColor_(mMinColor), mMaxColor_(mMaxColor)
{
}

const color& gradient::get_min_color() const
{
    return mMinColor_;
}

const color& gradient::get_max_color() const
{
    return mMaxColor_;
}

gradient::orientation gradient::get_orientation() const
{
    return mOrientation_;
}

bool gradient::is_empty() const
{
    return bIsEmpty_;
}
}
}
