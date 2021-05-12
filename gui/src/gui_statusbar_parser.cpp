#include "lxgui/gui_statusbar.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/xml_document.hpp>
#include <lxgui/utils_string.hpp>

namespace lxgui {
namespace gui
{
void status_bar::parse_attributes_(xml::block* pBlock)
{
    frame::parse_attributes_(pBlock);

    if (pBlock->is_provided("minValue") || !bInherits_)
        set_min_value(utils::string_to_float(pBlock->get_attribute("minValue")));
    if (pBlock->is_provided("maxValue") || !bInherits_)
        set_max_value(utils::string_to_float(pBlock->get_attribute("maxValue")));
    if (pBlock->is_provided("defaultValue") || !bInherits_)
        set_value(utils::string_to_float(pBlock->get_attribute("defaultValue")));
    if (pBlock->is_provided("drawLayer") || !bInherits_)
        set_bar_draw_layer(pBlock->get_attribute("drawLayer"));

    if (pBlock->is_provided("orientation") || !bInherits_)
    {
        std::string sOrientation = pBlock->get_attribute("orientation");
        if (sOrientation == "HORIZONTAL")
            set_orientation(orientation::HORIZONTAL);
        else if (sOrientation == "VERTICAL")
            set_orientation(orientation::VERTICAL);
        else
        {
            gui::out << gui::warning << pBlock->get_location() << " : "
                "Unknown StatusBar orientation : \""+sOrientation+"\". Expecting either :\n"
                "\"HORIZONTAL\" or \"VERTICAL\". Attribute ignored." << std::endl;
        }
    }

    if (pBlock->is_provided("reversed") || !bInherits_)
        set_reversed(utils::string_to_bool(pBlock->get_attribute("reversed")));
}

void status_bar::parse_all_blocks_before_children_(xml::block* pBlock)
{
    frame::parse_all_blocks_before_children_(pBlock);

    xml::block* pBarBlock = pBlock->get_radio_block();
    if (pBarBlock)
    {
        if (pBarBlock->get_name() == "BarTexture")
        {
            std::unique_ptr<texture> pBarTexture = create_bar_texture_();
            pBarTexture->parse_block(pBarBlock);
            set_bar_texture(pBarTexture.get());
            add_region(std::move(pBarTexture));
        }
        else
            set_bar_color(parse_color_block_(pBarBlock));
    }
}
}
}
