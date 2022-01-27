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

    if (const layout_attribute* pAttr = mNode.try_get_attribute("justifyH"))
    {
        const std::string& sJustifyH = pAttr->get_value<std::string>();
        if (sJustifyH == "LEFT")
            set_justify_h(text::alignment::LEFT);
        else if (sJustifyH == "CENTER")
            set_justify_h(text::alignment::CENTER);
        else if (sJustifyH == "RIGHT")
            set_justify_h(text::alignment::RIGHT);
        else
        {
            gui::out << gui::warning << mNode.get_location() <<  " : "
                << "Unknown horizontal justify behavior for " << sName_
                << " : \"" << sJustifyH << "\"." << std::endl;
        }
    }

    if (const layout_attribute* pAttr = mNode.try_get_attribute("justifyV"))
    {
        const std::string& sJustifyV = pAttr->get_value<std::string>();
        if (sJustifyV == "TOP")
            set_justify_v(text::vertical_alignment::TOP);
        else if (sJustifyV == "MIDDLE")
            set_justify_v(text::vertical_alignment::MIDDLE);
        else if (sJustifyV == "BOTTOM")
            set_justify_v(text::vertical_alignment::BOTTOM);
        else
        {
            gui::out << gui::warning << mNode.get_location() <<  " : "
                << "Unknown vertical justify behavior for " << sName_
                << " : \"" << sJustifyV << "\"." << std::endl;
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
