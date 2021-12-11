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
