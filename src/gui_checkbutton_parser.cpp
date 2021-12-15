#include "lxgui/gui_checkbutton.hpp"

#include <lxgui/utils_layout_node.hpp>
#include "lxgui/gui_texture.hpp"

namespace lxgui {
namespace gui
{
void check_button::parse_all_nodes_before_children_(const utils::layout_node& mNode)
{
    button::parse_all_nodes_before_children_(mNode);

    if (const utils::layout_node* pSpecialBlock = mNode.try_get_child("CheckedTexture"))
    {
        std::string sLayer = pSpecialBlock->get_attribute_value_or<std::string>("layer", "ARTWORK");

        utils::layout_node mDefaulted = *pSpecialBlock;
        mDefaulted.get_or_set_attribute_value("name", "$parentCheckedTexture");
        mDefaulted.get_or_set_attribute_value("setAllPoints", "true");

        auto pTexture = parse_region_(mDefaulted, sLayer, "Texture");
        if (pTexture)
        {
            pTexture->set_special();
            set_checked_texture(utils::static_pointer_cast<texture>(pTexture));
        }
    }

    if (const utils::layout_node* pSpecialBlock = mNode.try_get_child("DisabledCheckedTexture"))
    {
        std::string sLayer = pSpecialBlock->get_attribute_value_or<std::string>("layer", "ARTWORK");

        utils::layout_node mDefaulted = *pSpecialBlock;
        mDefaulted.get_or_set_attribute_value("name", "$parentDisabledCheckedTexture");
        mDefaulted.get_or_set_attribute_value("setAllPoints", "true");

        auto pTexture = parse_region_(mDefaulted, sLayer, "Texture");
        if (pTexture)
        {
            pTexture->set_special();
            set_disabled_checked_texture(utils::static_pointer_cast<texture>(pTexture));
        }
    }
}
}
}
