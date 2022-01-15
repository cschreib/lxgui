#include "lxgui/gui_editbox.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_layoutnode.hpp"
#include "lxgui/gui_parser_common.hpp"

namespace lxgui {
namespace gui
{
void edit_box::parse_attributes_(const layout_node& mNode)
{
    base::parse_attributes_(mNode);

    if (const layout_attribute* pAttr = mNode.try_get_attribute("letters"))
        set_max_letters(pAttr->get_value<std::size_t>());

    if (const layout_attribute* pAttr = mNode.try_get_attribute("blinkSpeed"))
        set_blink_speed(pAttr->get_value<float>());

    if (const layout_attribute* pAttr = mNode.try_get_attribute("numeric"))
        set_numeric_only(pAttr->get_value<bool>());

    if (const layout_attribute* pAttr = mNode.try_get_attribute("positive"))
        set_positive_only(pAttr->get_value<bool>());

    if (const layout_attribute* pAttr = mNode.try_get_attribute("integer"))
        set_integer_only(pAttr->get_value<bool>());

    if (const layout_attribute* pAttr = mNode.try_get_attribute("password"))
        enable_password_mode(pAttr->get_value<bool>());

    if (const layout_attribute* pAttr = mNode.try_get_attribute("multiLine"))
        set_multi_line(pAttr->get_value<bool>());

    if (const layout_attribute* pAttr = mNode.try_get_attribute("historyLines"))
        set_max_history_lines(pAttr->get_value<std::size_t>());

    if (const layout_attribute* pAttr = mNode.try_get_attribute("ignoreArrows"))
        set_arrows_ignored(pAttr->get_value<bool>());
}

void edit_box::parse_all_nodes_before_children_(const layout_node& mNode)
{
    base::parse_all_nodes_before_children_(mNode);

    parse_text_insets_node_(mNode);
    parse_font_string_node_(mNode);

    if (const layout_node* pHighlightNode = mNode.try_get_child("HighlightColor"))
        set_highlight_color(parse_color_node_(*pHighlightNode));
}

void edit_box::parse_font_string_node_(const layout_node& mNode)
{
    if (const layout_node* pFontStringNode = mNode.try_get_child("FontString"))
    {
        layout_node mDefaulted = *pFontStringNode;
        mDefaulted.get_or_set_attribute_value("name", "$parentFontString");

        auto pFontString = parse_region_(mDefaulted, "ARTWORK", "FontString");
        if (pFontString)
        {
            pFontString->set_special();
            set_font_string(utils::static_pointer_cast<font_string>(pFontString));
        }

        if (const layout_node* pErrorNode = mDefaulted.try_get_child("Anchors"))
        {
            gui::out << gui::warning << pErrorNode->get_location() << " : "
                << "edit_box : font_string's anchors will be ignored." << std::endl;
        }

        if (const layout_node* pErrorNode = mDefaulted.try_get_child("Size"))
        {
            gui::out << gui::warning << pErrorNode->get_location() << " : "
                << "edit_box : font_string's Size will be ignored." << std::endl;
        }

        warn_for_not_accessed_node(mDefaulted);
        pFontStringNode->bypass_access_check();
    }
}

void edit_box::parse_text_insets_node_(const layout_node& mNode)
{
    if (const layout_node* pTextInsetsNode = mNode.try_get_child("TextInsets"))
    {
        set_text_insets(bounds2f(
            pTextInsetsNode->get_attribute_value_or<float>("left", 0.0f),
            pTextInsetsNode->get_attribute_value_or<float>("right", 0.0f),
            pTextInsetsNode->get_attribute_value_or<float>("top", 0.0f),
            pTextInsetsNode->get_attribute_value_or<float>("bottom", 0.0f)
        ));
    }
}
}
}
