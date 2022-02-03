#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_layoutnode.hpp"

namespace lxgui {
namespace gui
{
void font_string::parse_layout(const layout_node& mNode)
{
    layered_region::parse_layout(mNode);

    if (const layout_node* pColorNode = mNode.try_get_child("Color"))
        set_text_color(parse_color_node_(*pColorNode));

    parse_shadow_node_(mNode);
}

void font_string::parse_attributes_(const layout_node& mNode)
{
    layered_region::parse_attributes_(mNode);

    set_font(
        mNode.get_attribute_value_or<std::string>("font", ""),
        mNode.get_attribute_value_or<float>("fontHeight", 0.0f)
    );

    if (const layout_attribute* pAttr = mNode.try_get_attribute("text"))
    {
        set_text(utils::utf8_to_unicode(
            get_manager().get_localizer().localize(pAttr->get_value<std::string>())));
    }

    if (const layout_attribute* pAttr = mNode.try_get_attribute("nonspacewrap"))
        set_non_space_wrap(pAttr->get_value<bool>());

    if (const layout_attribute* pAttr = mNode.try_get_attribute("spacing"))
        set_spacing(pAttr->get_value<float>());

    if (const layout_attribute* pAttr = mNode.try_get_attribute("lineSpacing"))
        set_line_spacing(pAttr->get_value<float>());

    if (const layout_attribute* pAttr = mNode.try_get_attribute("outline"))
    {
        const std::string& sOutline = pAttr->get_value<std::string>();
        if (sOutline == "NORMAL" || sOutline == "THICK")
            set_outlined(true);
        else if (sOutline == "NONE")
            set_outlined(false);
        else
        {
            gui::out << gui::warning << mNode.get_location() <<  " : "
                << "Unknown outline type for " << sName_ << " : \""
                << sOutline << "\"." << std::endl;
        }
    }

    if (const layout_attribute* pAttr = mNode.try_get_attribute("alignX"))
    {
        const std::string& sAlignX = pAttr->get_value<std::string>();
        if (sAlignX == "LEFT")
            set_alignment_x(alignment_x::LEFT);
        else if (sAlignX == "CENTER")
            set_alignment_x(alignment_x::CENTER);
        else if (sAlignX == "RIGHT")
            set_alignment_x(alignment_x::RIGHT);
        else
        {
            gui::out << gui::warning << mNode.get_location() <<  " : "
                << "Unknown horizontal alignment behavior for " << sName_
                << " : \"" << sAlignX << "\"." << std::endl;
        }
    }

    if (const layout_attribute* pAttr = mNode.try_get_attribute("alignY"))
    {
        const std::string& sAlignY = pAttr->get_value<std::string>();
        if (sAlignY == "TOP")
            set_alignment_y(alignment_y::TOP);
        else if (sAlignY == "MIDDLE")
            set_alignment_y(alignment_y::MIDDLE);
        else if (sAlignY == "BOTTOM")
            set_alignment_y(alignment_y::BOTTOM);
        else
        {
            gui::out << gui::warning << mNode.get_location() <<  " : "
                << "Unknown vertical alignment behavior for " << sName_
                << " : \"" << sAlignY << "\"." << std::endl;
        }
    }
}

void font_string::parse_shadow_node_(const layout_node& mNode)
{
    if (const layout_node* pShadowNode = mNode.try_get_child("Shadow"))
    {
        set_shadow(true);

        if (const layout_node* pColorNode = pShadowNode->try_get_child("Color"))
            set_shadow_color(parse_color_node_(*pColorNode));


        if (const layout_node* pOffsetNode = pShadowNode->try_get_child("Offset"))
        {
            set_shadow_offset(vector2f(
                pOffsetNode->get_attribute_value_or<float>("x", 0.0),
                pOffsetNode->get_attribute_value_or<float>("y", 0.0)
            ));
        }
    }
}
}
}
