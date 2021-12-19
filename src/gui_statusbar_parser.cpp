#include "lxgui/gui_statusbar.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_layout_node.hpp>

namespace lxgui {
namespace gui
{

void status_bar::parse_attributes_(const utils::layout_node& mNode)
{
    frame::parse_attributes_(mNode);

    if (const utils::layout_node* pAttr = mNode.try_get_attribute("minValue"))
        set_min_value(pAttr->get_value<float>());
    if (const utils::layout_node* pAttr = mNode.try_get_attribute("maxValue"))
        set_max_value(pAttr->get_value<float>());
    if (const utils::layout_node* pAttr = mNode.try_get_attribute("defaultValue"))
        set_value(pAttr->get_value<float>());
    if (const utils::layout_node* pAttr = mNode.try_get_attribute("drawLayer"))
        set_bar_draw_layer(pAttr->get_value<std::string>());

    if (const utils::layout_node* pAttr = mNode.try_get_attribute("orientation"))
    {
        std::string sOrientation = pAttr->get_value<std::string>();
        if (sOrientation == "HORIZONTAL")
            set_orientation(orientation::HORIZONTAL);
        else if (sOrientation == "VERTICAL")
            set_orientation(orientation::VERTICAL);
        else
        {
            gui::out << gui::warning << mNode.get_location() << " : "
                "Unknown StatusBar orientation : \""+sOrientation+"\". Expecting either :\n"
                "\"HORIZONTAL\" or \"VERTICAL\". Attribute ignored." << std::endl;
        }
    }

    if (const utils::layout_node* pAttr = mNode.try_get_attribute("reversed"))
        set_reversed(pAttr->get_value<bool>());
}

void status_bar::parse_all_nodes_before_children_(const utils::layout_node& mNode)
{
    frame::parse_all_nodes_before_children_(mNode);

    const utils::layout_node* pTextureNode = mNode.try_get_child("BarTexture");
    const utils::layout_node* pColorNode = mNode.try_get_child("BarColor");
    if (pColorNode && pTextureNode)
    {
        gui::out << gui::warning << mNode.get_location() << " : "
            "StatusBar can only contain one of BarTexture or BarColor, but not both. "
            "BarColor ignored." << std::endl;
    }

    if (pTextureNode)
    {
        utils::layout_node mDefaulted = *pTextureNode;
        mDefaulted.get_or_set_attribute_value("name", "$parentBarTexture");

        auto pBarTexture = parse_region_(mDefaulted, "ARTWORK", "Texture");
        if (!pBarTexture)
            return;

        pBarTexture->set_special();
        set_bar_texture(utils::static_pointer_cast<texture>(pBarTexture));
    }
    else if (pColorNode)
    {
        set_bar_color(parse_color_node_(*pColorNode));
    }
}

}
}