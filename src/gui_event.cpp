#include "lxgui/gui_event.hpp"

namespace lxgui {
namespace gui
{

event::event(const std::string& sName) :
    sName_(sName)
{
}

void event::set_name(const std::string& sName)
{
    sName_ = sName;
}

const std::string& event::get_name() const
{
    return sName_;
}


}
}
