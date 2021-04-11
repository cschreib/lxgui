#include "lxgui/gui_uiobject.hpp"

#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_color.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/xml_document.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_std.hpp>

namespace lxgui {
namespace gui
{
color uiobject::parse_color_block_(xml::block* pBlock)
{
    if (pBlock->is_provided("c"))
    {
        std::string sColor = pBlock->get_attribute("c");
        if (!sColor.empty() && sColor[0] == '#')
            return color(sColor);

    }

    return color(
        utils::string_to_float(pBlock->get_attribute("r")),
        utils::string_to_float(pBlock->get_attribute("g")),
        utils::string_to_float(pBlock->get_attribute("b")),
        utils::string_to_float(pBlock->get_attribute("a"))
    );
}

void uiobject::parse_size_block_(xml::block* pBlock)
{
    xml::block* pSizeBlock = pBlock->get_block("Size");
    if (pSizeBlock)
    {
        xml::block* pDimBlock = pSizeBlock->get_radio_block();
        if (pDimBlock->get_name() == "AbsDimension")
        {
            bool bHasX = pDimBlock->is_provided("x"), bHasY = pDimBlock->is_provided("y");
            if (bHasX && bHasY)
            {
                set_abs_dimensions(
                    utils::string_to_float(pDimBlock->get_attribute("x")),
                    utils::string_to_float(pDimBlock->get_attribute("y"))
                );
            }
            else if (bHasX)
                set_abs_width(utils::string_to_float(pDimBlock->get_attribute("x")));
            else if (bHasY)
                set_abs_height(utils::string_to_float(pDimBlock->get_attribute("y")));
        }
        else if (pDimBlock->get_name() == "RelDimension")
        {
            bool bHasX = pDimBlock->is_provided("x"), bHasY = pDimBlock->is_provided("y");
            if (bHasX && bHasY)
            {
                set_rel_dimensions(
                    utils::string_to_float(pDimBlock->get_attribute("x")),
                    utils::string_to_float(pDimBlock->get_attribute("y"))
                );
            }
            else if (bHasX)
                set_rel_width(utils::string_to_float(pDimBlock->get_attribute("x")));
            else if (bHasY)
                set_rel_height(utils::string_to_float(pDimBlock->get_attribute("y")));
        }
    }
}

void uiobject::parse_anchor_block_(xml::block* pBlock)
{
    xml::block* pAnchorsBlock = pBlock->get_block("Anchors");
    if (pAnchorsBlock)
    {
        std::vector<std::string> lFoundPoints;
        for (auto* pAnchorBlock : pAnchorsBlock->blocks())
        {
            std::string sPoint = pAnchorBlock->get_attribute("point");
            std::string sParent = pAnchorBlock->get_attribute("relativeTo");
            std::string sRelativePoint = pAnchorBlock->get_attribute("relativePoint");

            if (utils::find(lFoundPoints, sPoint) != lFoundPoints.end())
            {
                gui::out << gui::warning << pAnchorsBlock->get_location() << " : "
                    << "anchor point \"" << sPoint << "\" has already been defined "
                    "for \"" << sName_ << "\". anchor skipped." << std::endl;
            }
            else
            {
                if (sRelativePoint.empty())
                    sRelativePoint = sPoint;

                if (utils::has_no_content(sParent))
                {
                    if (pParent_ || is_virtual())
                        sParent = "$parent";
                    else
                        sParent = "";
                }

                anchor mAnchor(
                    this,
                    anchor::get_anchor_point(sPoint),
                    sParent,
                    anchor::get_anchor_point(sRelativePoint)
                );

                xml::block* pOffsetBlock = pAnchorBlock->get_block("Offset");
                if (pOffsetBlock)
                {
                    xml::block* pDimBlock = pOffsetBlock->get_radio_block();
                    if (pDimBlock->get_name() == "AbsDimension")
                    {
                        mAnchor.set_abs_offset(
                            utils::string_to_float(pDimBlock->get_attribute("x")),
                            utils::string_to_float(pDimBlock->get_attribute("y"))
                        );
                    }
                    else if (pDimBlock->get_name() == "RelDimension")
                    {
                        mAnchor.set_rel_offset(
                            utils::string_to_float(pDimBlock->get_attribute("x")),
                            utils::string_to_float(pDimBlock->get_attribute("y"))
                        );
                    }
                }

                set_point(mAnchor);
            }
        }
    }
}
}
}
