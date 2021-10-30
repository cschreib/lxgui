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
        auto pTexture = create_checked_texture_();
        pTexture->parse_block(pSpecialBlock);
        if (pSpecialBlock->is_provided("layer"))
            pTexture->set_draw_layer(pSpecialBlock->get_attribute("layer"));

        set_checked_texture(pTexture.get());
        add_region(std::move(pTexture));
    }

    pSpecialBlock = pBlock->get_block("DisabledCheckedTexture");
    if (pSpecialBlock)
    {
        auto pTexture = create_disabled_checked_texture_();
        pTexture->parse_block(pSpecialBlock);
        if (pSpecialBlock->is_provided("layer"))
            pTexture->set_draw_layer(pSpecialBlock->get_attribute("layer"));

        set_disabled_checked_texture(pTexture.get());
        add_region(std::move(pTexture));
    }
}
}
}
