#include "lxgui/gui_focusframe.hpp"

#include <lxgui/utils_string.hpp>
#include <lxgui/utils_layout_node.hpp>

namespace lxgui {
namespace gui
{
void focus_frame::parse_attributes_(const utils::layout_node& mNode)
{
    frame::parse_attributes_(mNode);

    if (mNode.has_attribute("autoFocus"))
        enable_auto_focus(mNode.get_attribute_value<bool>("autoFocus"));
}
}
}
