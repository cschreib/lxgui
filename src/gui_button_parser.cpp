#include "lxgui/gui_button.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_localizer.hpp"

#include <lxgui/xml_document.hpp>
#include <lxgui/utils_string.hpp>

namespace lxgui {
namespace gui
{
void button::parse_attributes_(xml::block* pBlock)
{
    frame::parse_attributes_(pBlock);

    if ((pBlock->is_provided("text") || !pBlock->is_provided("inherits")))
    {
        set_text(utils::utf8_to_unicode(
            get_manager().get_localizer().localize(pBlock->get_attribute("text"))));
    }
}

void button::parse_all_blocks_before_children_(xml::block* pBlock)
{
    frame::parse_all_blocks_before_children_(pBlock);

    xml::block* pSpecialBlock;
    pSpecialBlock = pBlock->get_block("NormalTexture");
    if (pSpecialBlock)
    {
        std::string sLayer = "ARTWORK";
        if (pSpecialBlock->is_provided("layer"))
            sLayer = pSpecialBlock->get_attribute("layer");

        auto pTexture = parse_region_(pSpecialBlock, sLayer, "Texture");
        pTexture->set_special();
        set_normal_texture(utils::static_pointer_cast<texture>(pTexture));
    }

    pSpecialBlock = pBlock->get_block("PushedTexture");
    if (pSpecialBlock)
    {
        std::string sLayer = "ARTWORK";
        if (pSpecialBlock->is_provided("layer"))
            sLayer = pSpecialBlock->get_attribute("layer");

        auto pTexture = parse_region_(pSpecialBlock, sLayer, "Texture");
        pTexture->set_special();
        set_pushed_texture(utils::static_pointer_cast<texture>(pTexture));
    }

    pSpecialBlock = pBlock->get_block("DisabledTexture");
    if (pSpecialBlock)
    {
        std::string sLayer = "ARTWORK";
        if (pSpecialBlock->is_provided("layer"))
            sLayer = pSpecialBlock->get_attribute("layer");

        auto pTexture = parse_region_(pSpecialBlock, sLayer, "Texture");
        pTexture->set_special();
        set_disabled_texture(utils::static_pointer_cast<texture>(pTexture));
    }

    pSpecialBlock = pBlock->get_block("HighlightTexture");
    if (pSpecialBlock)
    {
        std::string sLayer = "ARTWORK";
        if (pSpecialBlock->is_provided("layer"))
            sLayer = pSpecialBlock->get_attribute("layer");

        auto pTexture = parse_region_(pSpecialBlock, sLayer, "Texture");
        pTexture->set_special();
        set_highlight_texture(utils::static_pointer_cast<texture>(pTexture));
    }


    pSpecialBlock = pBlock->get_block("NormalText");
    if (pSpecialBlock)
    {
        std::string sLayer = "ARTWORK";
        if (pSpecialBlock->is_provided("layer"))
            sLayer = pSpecialBlock->get_attribute("layer");

        auto pFontString = parse_region_(pSpecialBlock, sLayer, "FontString");
        pFontString->set_special();
        set_normal_text(utils::static_pointer_cast<font_string>(pFontString));
    }

    pSpecialBlock = pBlock->get_block("HighlightText");
    if (pSpecialBlock)
    {
        std::string sLayer = "ARTWORK";
        if (pSpecialBlock->is_provided("layer"))
            sLayer = pSpecialBlock->get_attribute("layer");

        auto pFontString = parse_region_(pSpecialBlock, sLayer, "FontString");
        pFontString->set_special();
        set_highlight_text(utils::static_pointer_cast<font_string>(pFontString));
    }

    pSpecialBlock = pBlock->get_block("DisabledText");
    if (pSpecialBlock)
    {
        std::string sLayer = "ARTWORK";
        if (pSpecialBlock->is_provided("layer"))
            sLayer = pSpecialBlock->get_attribute("layer");

        auto pFontString = parse_region_(pSpecialBlock, sLayer, "FontString");
        pFontString->set_special();
        set_disabled_text(utils::static_pointer_cast<font_string>(pFontString));
    }

    xml::block* pOffsetBlock = pBlock->get_block("PushedTextOffset");
    if (pOffsetBlock)
    {
        xml::block* pDimBlock = pOffsetBlock->get_radio_block();
        if (pDimBlock->get_name() == "AbsDimension")
        {
            set_pushed_text_offset(vector2f(
                utils::string_to_float(pDimBlock->get_attribute("x")),
                utils::string_to_float(pDimBlock->get_attribute("y"))
            ));
        }
        else if (pDimBlock->get_name() == "RelDimension")
        {
            gui::out << gui::warning << pDimBlock->get_location() << " : "
                << "RelDimension for button:PushedTextOffset is not yet supported ("+sName_+")." << std::endl;
        }
    }
}
}
}
