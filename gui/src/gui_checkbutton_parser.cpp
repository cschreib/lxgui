#include "lxgui/gui_checkbutton.hpp"

#include <lxgui/xml_document.hpp>
#include "lxgui/gui_texture.hpp"

namespace lxgui {
namespace gui
{
void check_button::parse_all_blocks_before_children_(xml::block* pBlock)
{
    button::parse_all_blocks_before_children_(pBlock);

    xml::block* pSpecialBlock;
    pSpecialBlock = pBlock->get_block("CheckedTexture");
    if (pSpecialBlock)
    {
        std::string sLayer = "ARTWORK";
        if (pSpecialBlock->is_provided("layer"))
            sLayer = pSpecialBlock->get_attribute("layer");

        auto pTexture = parse_region_(pSpecialBlock, sLayer, "Texture");
        pTexture->set_special();
        set_checked_texture(utils::static_pointer_cast<texture>(pTexture));
    }

    pSpecialBlock = pBlock->get_block("DisabledCheckedTexture");
    if (pSpecialBlock)
    {
        std::string sLayer = "ARTWORK";
        if (pSpecialBlock->is_provided("layer"))
            sLayer = pSpecialBlock->get_attribute("layer");

        auto pTexture = parse_region_(pSpecialBlock, sLayer, "Texture");
        pTexture->set_special();
        set_disabled_checked_texture(utils::static_pointer_cast<texture>(pTexture));
    }
}
}
}
