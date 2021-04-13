#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_manager.hpp"

#include <lxgui/xml_document.hpp>

namespace lxgui {
namespace gui
{
void font_string::parse_block(xml::block* pBlock)
{
    layered_region::parse_block(pBlock);

    xml::block* pColorBlock = pBlock->get_block("Color");
    if (pColorBlock)
        set_text_color(parse_color_block_(pColorBlock));

    parse_shadow_block_(pBlock);
}

void font_string::parse_attributes_(xml::block* pBlock)
{
    layered_region::parse_attributes_(pBlock);

    set_font(
        pManager_->parse_file_name(pBlock->get_attribute("font")),
        utils::string_to_float(pBlock->get_attribute("fontHeight"))
    );

    if (pBlock->is_provided("text") || !bInherits_)
        set_text(utils::utf8_to_unicode(pBlock->get_attribute("text")));
    if (pBlock->is_provided("nonspacewrap") || !bInherits_)
        set_non_space_wrap(utils::string_to_bool(pBlock->get_attribute("nonspacewrap")));
    if (pBlock->is_provided("spacing") || !bInherits_)
        set_spacing(utils::string_to_float(pBlock->get_attribute("spacing")));

    if (pBlock->is_provided("outline") || !bInherits_)
    {
        const std::string& sOutline = pBlock->get_attribute("outline");
        if (sOutline == "NORMAL" || sOutline == "THICK")
            set_outlined(true);
        else if (sOutline == "NONE")
            set_outlined(false);
        else
        {
            gui::out << gui::warning << pBlock->get_location() <<  " : "
                << "Unknown outline type for " << sName_ << " : \""
                << sOutline << "\"." << std::endl;
        }
    }

    if (pBlock->is_provided("justifyH") || !bInherits_)
    {
        const std::string& sJustifyH = pBlock->get_attribute("justifyH");
        if (sJustifyH == "LEFT")
            set_justify_h(text::alignment::LEFT);
        else if (sJustifyH == "CENTER")
            set_justify_h(text::alignment::CENTER);
        else if (sJustifyH == "RIGHT")
            set_justify_h(text::alignment::RIGHT);
        else
        {
            gui::out << gui::warning << pBlock->get_location() <<  " : "
                << "Unknown horizontal justify behavior for " << sName_
                << " : \"" << sJustifyH << "\"." << std::endl;
        }
    }

    if (pBlock->is_provided("justifyV") || !bInherits_)
    {
        const std::string& sJustifyV = pBlock->get_attribute("justifyV");
        if (sJustifyV == "TOP")
            set_justify_v(text::vertical_alignment::TOP);
        else if (sJustifyV == "MIDDLE")
            set_justify_v(text::vertical_alignment::MIDDLE);
        else if (sJustifyV == "BOTTOM")
            set_justify_v(text::vertical_alignment::BOTTOM);
        else
        {
            gui::out << gui::warning << pBlock->get_location() <<  " : "
                << "Unknown vertical justify behavior for " << sName_
                << " : \"" << sJustifyV << "\"." << std::endl;
        }
    }
}

void font_string::parse_shadow_block_(xml::block* pBlock)
{
    xml::block* pShadowBlock = pBlock->get_block("Shadow");
    if (pShadowBlock)
    {
        set_shadow(true);
        xml::block* pColorBlock = pShadowBlock->get_block("Color");
        if (pColorBlock)
            set_shadow_color(parse_color_block_(pColorBlock));

        xml::block* pOffsetBlock = pShadowBlock->get_block("Offset");
        if (pOffsetBlock)
        {
            set_shadow_offsets(
                utils::string_to_float(pOffsetBlock->get_attribute("x")),
                utils::string_to_float(pOffsetBlock->get_attribute("y"))
            );
        }
    }
}
}
}
