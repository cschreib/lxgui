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

void event::add(const utils::any& mValue)
{
    lArgList_.push_back(mValue);
}

const utils::any* event::get(uint uiIndex) const
{
    return &lArgList_[uiIndex];
}

uint event::get_num_param() const
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

utils::any& event::operator [] (uint uiIndex)
{
    return lArgList_[uiIndex];
}

const utils::any& event::operator [] (uint uiIndex) const
{
    return lArgList_[uiIndex];
}
}
}
