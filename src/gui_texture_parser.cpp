#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_layoutnode.hpp"

namespace lxgui {
namespace gui
{
void texture::parse_layout(const layout_node& mNode)
{
    layered_region::parse_layout(mNode);

    parse_tex_coords_node_(mNode);

    if (const layout_node* pColorNode = mNode.try_get_child("Color"))
        set_solid_color(parse_color_node_(*pColorNode));

    parse_gradient_node_(mNode);
}

void texture::parse_attributes_(const layout_node& mNode)
{
    layered_region::parse_attributes_(mNode);

    if (const layout_attribute* pAttr = mNode.try_get_attribute("filter"))
        set_filter_mode(pAttr->get_value<std::string>());

    if (const layout_attribute* pAttr = mNode.try_get_attribute("file"))
        set_texture(pAttr->get_value<std::string>());
}

void texture::parse_tex_coords_node_(const layout_node& mNode)
{
    if (const layout_node* pTexCoordsNode = mNode.try_get_child("TexCoords"))
    {
        set_tex_rect({
            pTexCoordsNode->get_attribute_value_or<float>("left", 0.0f),
            pTexCoordsNode->get_attribute_value_or<float>("top", 0.0f),
            pTexCoordsNode->get_attribute_value_or<float>("right", 1.0f),
            pTexCoordsNode->get_attribute_value_or<float>("bottom", 1.0f)});
    }
}

void texture::parse_gradient_node_(const layout_node& mNode)
{
    if (const layout_node* pGradientNode = mNode.try_get_child("Gradient"))
    {
        std::string sOrientation = pGradientNode->get_attribute_value_or<std::string>(
            "orientation", "HORIZONTAL");

        gradient::orientation mOrient;
        if (sOrientation == "HORIZONTAL")
            mOrient = gradient::orientation::HORIZONTAL;
        else if (sOrientation == "VERTICAL")
            mOrient = gradient::orientation::VERTICAL;
        else
        {
            gui::out << gui::warning << pGradientNode->get_location() << " : "
                "Unknown gradient orientation for "+sName_+" : \""+sOrientation+"\". "
                "No gradient will be shown for this texture." << std::endl;
            return;
        }

        const layout_node* pMinColorNode = pGradientNode->try_get_child("MinColor");
        if (!pMinColorNode)
        {
            gui::out << gui::warning << mNode.get_location() << " : "
                "Gradient requires MinColor child node." << std::endl;
            return;
        }

        const layout_node* pMaxColorNode = pGradientNode->try_get_child("MaxColor");
        if (!pMaxColorNode)
        {
            gui::out << gui::warning << mNode.get_location() << " : "
                "Gradient requires MaxColor child node." << std::endl;
            return;
        }

        set_gradient(gradient(mOrient,
            parse_color_node_(*pMinColorNode),
            parse_color_node_(*pMaxColorNode)
        ));
    }
}
}
}
