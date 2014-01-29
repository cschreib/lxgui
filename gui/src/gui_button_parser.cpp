#include "lxgui/gui_button.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/xml_document.hpp>
#include <lxgui/utils_string.hpp>

namespace gui
{
void button::parse_block(xml::block* pBlock)
{
    frame::parse_block(pBlock);

    xml::block* pSpecialBlock;
    pSpecialBlock = pBlock->get_block("NormalTexture");
    if (pSpecialBlock)
    {
        texture* pTexture = create_normal_texture_();
        pTexture->parse_block(pSpecialBlock);
        if (pSpecialBlock->is_provided("layer"))
            pTexture->set_draw_layer(pSpecialBlock->get_attribute("layer"));
    }

    pSpecialBlock = pBlock->get_block("PushedTexture");
    if (pSpecialBlock)
    {
        texture* pTexture = create_pushed_texture_();
        pTexture->parse_block(pSpecialBlock);
        if (pSpecialBlock->is_provided("layer"))
            pTexture->set_draw_layer(pSpecialBlock->get_attribute("layer"));
    }

    pSpecialBlock = pBlock->get_block("DisabledTexture");
    if (pSpecialBlock)
    {
        texture* pTexture = create_disabled_texture_();
        pTexture->parse_block(pSpecialBlock);
        if (pSpecialBlock->is_provided("layer"))
            pTexture->set_draw_layer(pSpecialBlock->get_attribute("layer"));
    }

    pSpecialBlock = pBlock->get_block("HighlightTexture");
    if (pSpecialBlock)
    {
        texture* pTexture = create_highlight_texture_();
        pTexture->parse_block(pSpecialBlock);
        if (pSpecialBlock->is_provided("layer"))
            pTexture->set_draw_layer(pSpecialBlock->get_attribute("layer"));
    }


    pSpecialBlock = pBlock->get_block("NormalText");
    if (pSpecialBlock)
    {
        font_string* pFontString = create_normal_text_();
        pFontString->parse_block(pSpecialBlock);
        if (pSpecialBlock->is_provided("layer"))
            pFontString->set_draw_layer(pSpecialBlock->get_attribute("layer"));
    }

    pSpecialBlock = pBlock->get_block("HighlightText");
    if (pSpecialBlock)
    {
        font_string* pFontString = create_highlight_text_();
        pFontString->parse_block(pSpecialBlock);
        if (pSpecialBlock->is_provided("layer"))
            pFontString->set_draw_layer(pSpecialBlock->get_attribute("layer"));
    }

    pSpecialBlock = pBlock->get_block("DisabledText");
    if (pSpecialBlock)
    {
        font_string* pFontString = create_disabled_text_();
        pFontString->parse_block(pSpecialBlock);
        if (pSpecialBlock->is_provided("layer"))
            pFontString->set_draw_layer(pSpecialBlock->get_attribute("layer"));
    }

    xml::block* pOffsetBlock = pBlock->get_block("PushedTextOffset");
    if (pOffsetBlock)
    {
        xml::block* pDimBlock = pOffsetBlock->get_radio_block();
        if (pDimBlock->get_name() == "AbsDimension")
        {
            set_pushed_text_offset(vector2i(
                (int)utils::string_to_int(pDimBlock->get_attribute("x")),
                (int)utils::string_to_int(pDimBlock->get_attribute("y"))
            ));
        }
        else if (pDimBlock->get_name() == "RelDimension")
        {
            gui::out << gui::warning << pDimBlock->get_location() << " : "
                << "RelDimension for button:PushedTextOffset is not yet supported ("+sName_+")." << std::endl;
        }
    }

    if ((pBlock->is_provided("text") || !pBlock->is_provided("inherits")))
        set_text(pBlock->get_attribute("text"));
}
}
