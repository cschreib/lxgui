#include "lxgui/gui_editbox.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/xml_document.hpp>
#include <lxgui/utils_string.hpp>

namespace lxgui {
namespace gui
{
void edit_box::parse_block(xml::block* pBlock)
{
    focus_frame::parse_block(pBlock);

    parse_text_insets_block_(pBlock);
    parse_font_string_block_(pBlock);

    xml::block* pHighlightBlock = pBlock->get_block("HighlightColor");
    if (pHighlightBlock)
        set_highlight_color(parse_color_block_(pHighlightBlock));

    if ((pBlock->is_provided("letters") || !pBlock->is_provided("inherits")))
        set_max_letters(utils::string_to_uint(pBlock->get_attribute("letters")));

    if ((pBlock->is_provided("blinkSpeed") || !pBlock->is_provided("inherits")))
        set_blink_speed(utils::string_to_float(pBlock->get_attribute("blinkSpeed")));

    if ((pBlock->is_provided("numeric") || !pBlock->is_provided("inherits")))
        set_numeric_only(utils::string_to_bool(pBlock->get_attribute("numeric")));

    if ((pBlock->is_provided("positive") || !pBlock->is_provided("positive")))
        set_positive_only(utils::string_to_bool(pBlock->get_attribute("positive")));

    if ((pBlock->is_provided("integer") || !pBlock->is_provided("integer")))
        set_integer_only(utils::string_to_bool(pBlock->get_attribute("integer")));

    if ((pBlock->is_provided("password") || !pBlock->is_provided("inherits")))
        enable_password_mode(utils::string_to_bool(pBlock->get_attribute("password")));

    if ((pBlock->is_provided("multiLine") || !pBlock->is_provided("inherits")))
        set_multi_line(utils::string_to_bool(pBlock->get_attribute("multiLine")));

    if ((pBlock->is_provided("historyLines") || !pBlock->is_provided("inherits")))
        set_max_history_lines(utils::string_to_uint(pBlock->get_attribute("historyLines")));

    if ((pBlock->is_provided("ignoreArrows") || !pBlock->is_provided("inherits")))
        set_arrows_ignored(utils::string_to_bool(pBlock->get_attribute("ignoreArrows")));
}

void edit_box::parse_font_string_block_(xml::block* pBlock)
{
    xml::block* pFontStringBlock = pBlock->get_block("FontString");
    if (pFontStringBlock)
    {
        std::unique_ptr<font_string> pFontString = create_font_string_();
        pFontString->parse_block(pFontStringBlock);

        xml::block* pErrorBlock = pFontStringBlock->get_block("Anchors");
        if (pErrorBlock)
        {
            gui::out << gui::warning << pErrorBlock->get_location() << " : "
                << "edit_box : font_string's anchors are ignored." << std::endl;
        }

        pErrorBlock = pFontStringBlock->get_block("Size");
        if (pErrorBlock)
        {
            gui::out << gui::warning << pErrorBlock->get_location() << " : "
                << "edit_box : font_string's Size is ignored." << std::endl;
        }

        set_font_string(pFontString.get());
        add_region(std::move(pFontString));
    }
}

void edit_box::parse_text_insets_block_(xml::block* pBlock)
{
    xml::block* pTextInsetsBlock = pBlock->get_block("TextInsets");
    if (pTextInsetsBlock)
    {
        set_text_insets(
            utils::string_to_int(pTextInsetsBlock->get_attribute("left")),
            utils::string_to_int(pTextInsetsBlock->get_attribute("right")),
            utils::string_to_int(pTextInsetsBlock->get_attribute("top")),
            utils::string_to_int(pTextInsetsBlock->get_attribute("bottom"))
        );
    }
}
}
}
