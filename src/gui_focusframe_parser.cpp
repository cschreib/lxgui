#include "lxgui/gui_focusframe.hpp"
#include "lxgui/gui_layoutnode.hpp"

namespace lxgui {
namespace gui
{
void focus_frame::parse_attributes_(const layout_node& mNode)
{
    frame::parse_attributes_(mNode);

    if (mNode.has_attribute("autoFocus"))
        enable_auto_focus(mNode.get_attribute_value<bool>("autoFocus"));
}
}
}
