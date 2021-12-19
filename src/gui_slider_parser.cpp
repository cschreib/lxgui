#include "lxgui/gui_slider.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_layout_node.hpp>

namespace lxgui {
namespace gui
{
void slider::parse_attributes_(const utils::layout_node& mNode)
{
    frame::parse_attributes_(mNode);

    if (const utils::layout_node* pAttr = mNode.try_get_attribute("valueStep"))
        set_value_step(pAttr->get_value<float>());
    if (const utils::layout_node* pAttr = mNode.try_get_attribute("minValue"))
        set_min_value(pAttr->get_value<float>());
    if (const utils::layout_node* pAttr = mNode.try_get_attribute("maxValue"))
        set_max_value(pAttr->get_value<float>());
    if (const utils::layout_node* pAttr = mNode.try_get_attribute("defaultValue"))
        set_value(pAttr->get_value<float>());
    if (const utils::layout_node* pAttr = mNode.try_get_attribute("drawLayer"))
        set_thumb_draw_layer(pAttr->get_value<std::string>());

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
                "Unknown Slider orientation : \""+sOrientation+"\". Expecting either :\n"
                "\"HORIZONTAL\" or \"VERTICAL\". Attribute ignored." << std::endl;
        }
    }
}

void slider::parse_all_nodes_before_children_(const utils::layout_node& mNode)
{
    frame::parse_all_nodes_before_children_(mNode);

    if (const utils::layout_node* pThumbBlock = mNode.try_get_child("ThumbTexture"))
    {
        utils::layout_node mDefaulted = *pThumbBlock;
        mDefaulted.get_or_set_attribute_value("name", "$parentThumbTexture");

        auto pThumbTexture = parse_region_(mDefaulted, "ARTWORK", "Texture");
        if (!pThumbTexture)
            return;

        pThumbTexture->set_special();
        set_thumb_texture(utils::static_pointer_cast<texture>(pThumbTexture));
    }
}
}
}