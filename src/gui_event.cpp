#include "lxgui/gui_event.hpp"

namespace lxgui {
namespace gui
{

event_data::event_data(std::initializer_list<utils::variant> lData)
{
    for (auto& mElement : lData)
        lArgList_.push_back(std::move(mElement));
}


}
}
