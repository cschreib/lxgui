#include "lxgui/gui_uiobject.hpp"

#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_color.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_layoutnode.hpp"

#include <lxgui/utils_std.hpp>

namespace lxgui {
namespace gui
{
color uiobject::parse_color_node_(const layout_node& mNode)
{
    if (const layout_node* pAttr = mNode.try_get_attribute("c"))
    {
        std::string sColor = pAttr->get_value<std::string>();
        if (!sColor.empty() && sColor[0] == '#')
            return color(sColor);
    }

    return color(
        mNode.get_attribute_value_or<float>("r", 0.0f),
        mNode.get_attribute_value_or<float>("g", 0.0f),
        mNode.get_attribute_value_or<float>("b", 0.0f),
        mNode.get_attribute_value_or<float>("a", 1.0f)
    );
}

std::pair<anchor_type, vector2<std::optional<float>>> uiobject::parse_dimension_(
    const layout_node& mNode)
{
    const layout_node* pAbsDimNode = mNode.try_get_child("AbsDimension");
    const layout_node* pRelDimNode = mNode.try_get_child("RelDimension");

    if (pAbsDimNode && pRelDimNode)
    {
        gui::out << gui::warning << mNode.get_location() << " : " <<
            mNode.get_name() << " node can only contain one of AbsDimension or RelDimension, "
            "but not both. RelDimension ignored." << std::endl;
    }

    if (!pAbsDimNode && !pRelDimNode)
    {
        gui::out << gui::warning << mNode.get_location() << " : " <<
            mNode.get_name() << " node must contain one of AbsDimension or RelDimension." << std::endl;
        return {};
    }

    anchor_type mType = anchor_type::ABS;
    const layout_node* pNode = nullptr;
    if (pAbsDimNode)
    {
        mType = anchor_type::ABS;
        pNode = pAbsDimNode;
    }
    else
    {
        mType = anchor_type::REL;
        pNode = pRelDimNode;
    }

    vector2<std::optional<float>> mVec;
    if (const layout_node* pAttr = pNode->try_get_attribute("x"))
        mVec.x = pAttr->get_value<float>();
    if (const layout_node* pAttr = pNode->try_get_attribute("y"))
        mVec.y = pAttr->get_value<float>();

    return std::make_pair(mType, mVec);
}

void uiobject::parse_size_node_(const layout_node& mNode)
{
    if (const layout_node* pSizeBlock = mNode.try_get_child("Size"))
    {
        auto mDimensions = parse_dimension_(*pSizeBlock);
        bool bHasX = mDimensions.second.x.has_value();
        bool bHasY = mDimensions.second.y.has_value();
        if (mDimensions.first == anchor_type::ABS)
        {
            if (bHasX && bHasY)
            {
                set_dimensions(vector2f(
                    mDimensions.second.x.value(),
                    mDimensions.second.y.value()));
            }
            else if (bHasX)
                set_width(mDimensions.second.x.value());
            else if (bHasY)
                set_height(mDimensions.second.y.value());
        }
        else
        {
            if (bHasX && bHasY)
            {
                set_relative_dimensions(vector2f(
                    mDimensions.second.x.value(),
                    mDimensions.second.y.value()));
            }
            else if (bHasX)
                set_relative_width(mDimensions.second.x.value());
            else if (bHasY)
                set_relative_height(mDimensions.second.y.value());
        }
    }
}

void uiobject::parse_anchor_node_(const layout_node& mNode)
{
    if (const layout_node* pAnchorsNode = mNode.try_get_child("Anchors"))
    {
        std::vector<std::string> lFoundPoints;
        for (const auto& mAnchorNode : pAnchorsNode->get_children())
        {
            if (mAnchorNode.get_name() != "Anchor" && mAnchorNode.get_name() != "")
            {
                gui::out << gui::warning << mAnchorNode.get_location() << " : "
                    << "unexpected node '" << mAnchorNode.get_name() << "'; ignored." << std::endl;
                continue;
            }

            std::string sPoint = mAnchorNode.get_attribute_value_or<std::string>(
                "point", "TOPLEFT");
            std::string sParent = mAnchorNode.get_attribute_value_or<std::string>(
                "relativeTo", pParent_ || is_virtual() ? "$parent" : "");
            std::string sRelativePoint = mAnchorNode.get_attribute_value_or<std::string>(
                "relativePoint", sPoint);

            if (utils::find(lFoundPoints, sPoint) != lFoundPoints.end())
            {
                gui::out << gui::warning << mAnchorNode.get_location() << " : "
                    << "anchor point \"" << sPoint << "\" has already been defined "
                    "for \"" << sName_ << "\". anchor skipped." << std::endl;
                continue;
            }

            anchor_data mAnchor(
                anchor::get_anchor_point(sPoint),
                sParent,
                anchor::get_anchor_point(sRelativePoint)
            );

            if (const layout_node* pOffsetNode = mAnchorNode.try_get_child("Offset"))
            {
                auto mDimensions = parse_dimension_(*pOffsetNode);
                mAnchor.mType = mDimensions.first;
                mAnchor.mOffset = vector2f(
                    mDimensions.second.x.value_or(0.0f),
                    mDimensions.second.y.value_or(0.0f)
                );
            }

            set_point(mAnchor);
        }
    }
}
}
}
