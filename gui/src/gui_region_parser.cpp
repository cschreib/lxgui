#include "lxgui/gui_region.hpp"

#include <lxgui/xml_document.hpp>
#include <lxgui/utils_string.hpp>

namespace lxgui {
namespace gui
{
void region::parse_block(xml::block* pBlock)
{
    parse_attributes_(pBlock);

    parse_size_block_(pBlock);
    parse_anchor_block_(pBlock);
}

void region::parse_attributes_(xml::block* pBlock)
{
    if ((pBlock->is_provided("setAllPoints") || !bInherits_) &&
        (utils::string_to_bool(pBlock->get_attribute("setAllPoints"))))
        set_all_points("$parent");
}
}
}
