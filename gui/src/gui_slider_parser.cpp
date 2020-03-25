#include "lxgui/gui_slider.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/xml_document.hpp>
#include <lxgui/utils_string.hpp>

namespace gui
{
void slider::parse_block(xml::block* pBlock)
{
    frame::parse_block(pBlock);

    if (pBlock->is_provided("valueStep") || !bInherits_)
        set_value_step(utils::string_to_float(pBlock->get_attribute("valueStep")));
    if (pBlock->is_provided("minValue") || !bInherits_)
        set_min_value(utils::string_to_float(pBlock->get_attribute("minValue")));
    if (pBlock->is_provided("maxValue") || !bInherits_)
        set_max_value(utils::string_to_float(pBlock->get_attribute("maxValue")));
    if (pBlock->is_provided("defaultValue") || !bInherits_)
        set_value(utils::string_to_float(pBlock->get_attribute("defaultValue")));
    if (pBlock->is_provided("drawLayer") || !bInherits_)
        set_thumb_draw_layer(pBlock->get_attribute("drawLayer"));

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
                "Unknown Slider orientation : \""+sOrientation+"\". Expecting either :\n"
                "\"HORIZONTAL\" or \"VERTICAL\". Attribute ignored." << std::endl;
        }
    }

    xml::block* pThumbBlock = pBlock->get_block("ThumbTexture");
    if (pThumbBlock)
    {
        create_thumb_texture_();

        pThumbTexture_->parse_block(pThumbBlock);
        pThumbTexture_->clear_all_points();
        pThumbTexture_->set_point(anchor(
            pThumbTexture_, anchor_point::CENTER, "$parent",
            get_orientation() == orientation::HORIZONTAL ? anchor_point::LEFT : anchor_point::TOP
        ));
    }
}
}
