#include "lxgui/gui_event.hpp"

namespace lxgui {
namespace gui
{
event::event(const std::string& sName, bool bOncePerFrame) :
    sName_(sName), bOncePerFrame_(bOncePerFrame)
{
}

void event::set_name(const std::string& sName)
{
    sName_ = sName;
}

void event::set_once_per_frame(bool bOncePerFrame)
{
    bOncePerFrame_ = bOncePerFrame;
}

void event::add(const utils::variant& mValue)
{
    lArgList_.push_back(mValue);
}

const utils::variant& event::get(std::size_t uiIndex) const
{
    return lArgList_[uiIndex];
}

utils::variant& event::get(std::size_t uiIndex)
{
    return lArgList_[uiIndex];
}

std::size_t event::get_num_param() const
{
    return lArgList_.size();
}

const std::string& event::get_name() const
{
    return sName_;
}

bool event::is_once_per_frame() const
{
    return bOncePerFrame_;
}
}
}
