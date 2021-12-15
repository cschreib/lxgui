#include "lxgui/gui_button.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_localizer.hpp"

#include <lxgui/utils_layout_node.hpp>
#include <lxgui/utils_string.hpp>

namespace lxgui {
namespace gui
{
void button::parse_attributes_(const utils::layout_node& mNode)
{
    frame::parse_attributes_(mNode);

    if (const utils::layout_node* pAttr = mNode.try_get_attribute("text"))
    {
        set_text(utils::utf8_to_unicode(
            get_manager().get_localizer().localize(pAttr->get_value<std::string>())));
    }
}

void button::parse_all_nodes_before_children_(const utils::layout_node& mNode)
{
    frame::parse_all_nodes_before_children_(mNode);

    if (const utils::layout_node* pSpecialNode = mNode.try_get_child("NormalTexture"))
    {
        auto sLayer = pSpecialNode->get_attribute_value_or<std::string>("layer", "ARTWORK");

        utils::layout_node mDefaulted = *pSpecialNode;
        mDefaulted.get_or_set_attribute_value("name", "$parentNormalTexture");
        mDefaulted.get_or_set_attribute_value("setAllPoints", "true");

        auto pTexture = parse_region_(mDefaulted, sLayer, "Texture");
        if (pTexture)
        {
            pTexture->set_special();
            set_normal_texture(utils::static_pointer_cast<texture>(pTexture));
        }
    }

    if (const utils::layout_node* pSpecialNode = mNode.try_get_child("PushedTexture"))
    {
        auto sLayer = pSpecialNode->get_attribute_value_or<std::string>("layer", "BORDER");

        utils::layout_node mDefaulted = *pSpecialNode;
        mDefaulted.get_or_set_attribute_value("name", "$parentPushedTexture");
        mDefaulted.get_or_set_attribute_value("setAllPoints", "true");

        auto pTexture = parse_region_(mDefaulted, sLayer, "Texture");
        if (pTexture)
        {
            pTexture->set_special();
            set_pushed_texture(utils::static_pointer_cast<texture>(pTexture));
        }
    }

    if (const utils::layout_node* pSpecialNode = mNode.try_get_child("DisabledTexture"))
    {
        auto sLayer = pSpecialNode->get_attribute_value_or<std::string>("layer", "BORDER");

        utils::layout_node mDefaulted = *pSpecialNode;
        mDefaulted.get_or_set_attribute_value("name", "$parentDisabledTexture");
        mDefaulted.get_or_set_attribute_value("setAllPoints", "true");

        auto pTexture = parse_region_(mDefaulted, sLayer, "Texture");
        if (pTexture)
        {
            pTexture->set_special();
            set_disabled_texture(utils::static_pointer_cast<texture>(pTexture));
        }
    }

    if (const utils::layout_node* pSpecialNode = mNode.try_get_child("HighlightTexture"))
    {
        auto sLayer = pSpecialNode->get_attribute_value_or<std::string>("layer", "HIGHLIGHT");

        utils::layout_node mDefaulted = *pSpecialNode;
        mDefaulted.get_or_set_attribute_value("name", "$parentHighlightTexture");
        mDefaulted.get_or_set_attribute_value("setAllPoints", "true");

        auto pTexture = parse_region_(mDefaulted, sLayer, "Texture");
        if (pTexture)
        {
            pTexture->set_special();
            set_highlight_texture(utils::static_pointer_cast<texture>(pTexture));
        }
    }


    if (const utils::layout_node* pSpecialNode = mNode.try_get_child("NormalText"))
    {
        auto sLayer = pSpecialNode->get_attribute_value_or<std::string>("layer", "ARTWORK");

        utils::layout_node mDefaulted = *pSpecialNode;
        mDefaulted.get_or_set_attribute_value("name", "$parentNormalText");
        mDefaulted.get_or_set_attribute_value("setAllPoints", "true");

        auto pFontString = parse_region_(mDefaulted, sLayer, "FontString");
        if (pFontString)
        {
            pFontString->set_special();
            set_normal_text(utils::static_pointer_cast<font_string>(pFontString));
        }
    }

    if (const utils::layout_node* pSpecialNode = mNode.try_get_child("HighlightText"))
    {
        auto sLayer = pSpecialNode->get_attribute_value_or<std::string>("layer", "HIGHLIGHT");

        utils::layout_node mDefaulted = *pSpecialNode;
        mDefaulted.get_or_set_attribute_value("name", "$parentHighlightText");
        mDefaulted.get_or_set_attribute_value("setAllPoints", "true");

        auto pFontString = parse_region_(mDefaulted, sLayer, "FontString");
        if (pFontString)
        {
            pFontString->set_special();
            set_highlight_text(utils::static_pointer_cast<font_string>(pFontString));
        }
    }

    if (const utils::layout_node* pSpecialNode = mNode.try_get_child("DisabledText"))
    {
        auto sLayer = pSpecialNode->get_attribute_value_or<std::string>("layer", "BORDER");

        utils::layout_node mDefaulted = *pSpecialNode;
        mDefaulted.get_or_set_attribute_value("name", "$parentDisabledText");
        mDefaulted.get_or_set_attribute_value("setAllPoints", "true");

        auto pFontString = parse_region_(mDefaulted, sLayer, "FontString");
        if (pFontString)
        {
            pFontString->set_special();
            set_disabled_text(utils::static_pointer_cast<font_string>(pFontString));
        }
    }

    if (const utils::layout_node* pOffsetBlock = mNode.try_get_child("PushedTextOffset"))
    {
        auto mDimensions = parse_dimension_(*pOffsetBlock);
        if (mDimensions.first == anchor_type::ABS)
        {
            set_pushed_text_offset(vector2f(
                mDimensions.second.x.value_or(0.0f),
                mDimensions.second.y.value_or(0.0f)
            ));
        }
        else
        {
            gui::out << gui::warning << pOffsetBlock->get_location() << " : "
                << "RelDimension for Button:PushedTextOffset is not yet supported. Skipped." << std::endl;
        }
    }
}
}
}
