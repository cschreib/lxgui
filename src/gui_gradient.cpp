#include "lxgui/gui_gradient.hpp"

namespace lxgui {
namespace gui
{

gradient::gradient(orientation mOrientation, const color& mMinColor, const color& mMaxColor) :
    mOrientation_(mOrientation), mMinColor_(mMinColor), mMaxColor_(mMaxColor)
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

}
}
