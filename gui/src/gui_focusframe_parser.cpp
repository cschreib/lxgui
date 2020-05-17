#include "lxgui/gui_focusframe.hpp"

#include <lxgui/xml_document.hpp>
#include <lxgui/utils_string.hpp>

namespace lxgui {
namespace gui
{
void focus_frame::parse_block(xml::block* pBlock)
{
    frame::parse_block(pBlock);

    if ((pBlock->is_provided("autoFocus") || !pBlock->is_provided("inherits")))
        enable_auto_focus(utils::string_to_bool(pBlock->get_attribute("autoFocus")));
}
}
}
