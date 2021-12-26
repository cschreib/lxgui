#include "lxgui/gui_checkbutton.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_parser_common.hpp"

namespace lxgui {
namespace gui
{
void check_button::parse_all_nodes_before_children_(const layout_node& mNode)
{
    button::parse_all_nodes_before_children_(mNode);

    if (const layout_node* pSpecialNode = mNode.try_get_child("CheckedTexture"))
    {
        std::string sLayer = pSpecialNode->get_attribute_value_or<std::string>("layer", "ARTWORK");

        layout_node mDefaulted = *pSpecialNode;
        mDefaulted.get_or_set_attribute_value("name", "$parentCheckedTexture");
        mDefaulted.get_or_set_attribute_value("setAllPoints", "true");

        auto pTexture = parse_region_(mDefaulted, sLayer, "Texture");
        if (pTexture)
        {
            pTexture->set_special();
            set_checked_texture(utils::static_pointer_cast<texture>(pTexture));
        }

        warn_for_not_accessed_node(mDefaulted);
        pSpecialNode->bypass_access_check();
    }

    if (const layout_node* pSpecialNode = mNode.try_get_child("DisabledCheckedTexture"))
    {
        std::string sLayer = pSpecialNode->get_attribute_value_or<std::string>("layer", "ARTWORK");

        layout_node mDefaulted = *pSpecialNode;
        mDefaulted.get_or_set_attribute_value("name", "$parentDisabledCheckedTexture");
        mDefaulted.get_or_set_attribute_value("setAllPoints", "true");

        auto pTexture = parse_region_(mDefaulted, sLayer, "Texture");
        if (pTexture)
        {
            pTexture->set_special();
            set_disabled_checked_texture(utils::static_pointer_cast<texture>(pTexture));
        }

        warn_for_not_accessed_node(mDefaulted);
        pSpecialNode->bypass_access_check();
    }
}
}
}
