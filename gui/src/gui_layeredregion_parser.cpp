#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_manager.hpp"

#include <lxgui/xml_document.hpp>
#include <lxgui/utils_string.hpp>

namespace lxgui {
namespace gui
{
void layered_region::parse_block(xml::block* pBlock)
{
    parse_attributes_(pBlock);

    parse_size_block_(pBlock);
    parse_anchor_block_(pBlock);
}

void layered_region::parse_attributes_(xml::block* pBlock)
{
    if ((pBlock->is_provided("hidden") || !bInherits_) &&
        (utils::string_to_bool(pBlock->get_attribute("hidden"))))
        hide();

    if ((pBlock->is_provided("setAllPoints") || !bInherits_) &&
        (utils::string_to_bool(pBlock->get_attribute("setAllPoints"))))
        set_all_points("$parent");
}
}
}
