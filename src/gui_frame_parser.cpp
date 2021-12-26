#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_backdrop.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_parser_common.hpp"
#include "lxgui/gui_layoutnode.hpp"

namespace lxgui {
namespace gui
{
void frame::parse_all_nodes_before_children_(const layout_node& mNode)
{
    parse_attributes_(mNode);

    parse_size_node_(mNode);
    parse_resize_bounds_node_(mNode);
    parse_anchor_node_(mNode);
    parse_title_region_node_(mNode);
    parse_backdrop_node_(mNode);
    parse_hit_rect_insets_node_(mNode);

    parse_layers_node_(mNode);
}

void frame::parse_layout(const layout_node& mNode)
{
    parse_all_nodes_before_children_(mNode);
    parse_frames_node_(mNode);
    parse_scripts_node_(mNode);
}

void frame::parse_attributes_(const layout_node& mNode)
{
    if (const layout_attribute* pAttr = mNode.try_get_attribute("hidden"))
        set_shown(!pAttr->get_value<bool>());

    if (mNode.get_attribute_value_or<bool>("setAllPoints", false))
        set_all_points("$parent");

    if (const layout_attribute* pAttr = mNode.try_get_attribute("alpha"))
        set_alpha(pAttr->get_value<float>());
    if (const layout_attribute* pAttr = mNode.try_get_attribute("topLevel"))
        set_top_level(pAttr->get_value<bool>());
    if (const layout_attribute* pAttr = mNode.try_get_attribute("movable"))
        set_movable(pAttr->get_value<bool>());
    if (const layout_attribute* pAttr = mNode.try_get_attribute("resizable"))
        set_resizable(pAttr->get_value<bool>());

    set_frame_strata(mNode.get_attribute_value_or<std::string>("frameStrata", "PARENT"));

    if (const layout_attribute* pAttr = mNode.try_get_attribute("frameLevel"))
    {
        if (!bVirtual_)
        {
            std::string sFrameLevel = pAttr->get_value<std::string>();
            int iLevel = 0;
            if (sFrameLevel != "PARENT" && utils::from_string(sFrameLevel, iLevel))
                set_level(iLevel);
        }
        else
        {
            gui::out << gui::warning << mNode.get_location() << " : "
                << "\"frameLevel\" is not allowed for virtual widgets. Ignored." << std::endl;
        }
    }
    if (const layout_attribute* pAttr = mNode.try_get_attribute("enableMouse"))
        enable_mouse(pAttr->get_value<bool>());
    if (const layout_attribute* pAttr = mNode.try_get_attribute("enableMouseWheel"))
        enable_mouse_wheel(pAttr->get_value<bool>());
    if (const layout_attribute* pAttr = mNode.try_get_attribute("enableKeyboard"))
        enable_keyboard(pAttr->get_value<bool>());
    if (const layout_attribute* pAttr = mNode.try_get_attribute("clampedToScreen"))
        set_clamped_to_screen(pAttr->get_value<bool>());
}

void frame::parse_resize_bounds_node_(const layout_node& mNode)
{
    if (const layout_node* pResizeBoundsNode = mNode.try_get_child("ResizeBounds"))
    {
        if (const layout_node* pMinNode = pResizeBoundsNode->try_get_child("Min"))
        {
            auto mDimensions = parse_dimension_(*pMinNode);
            bool bHasX = mDimensions.second.x.has_value();
            bool bHasY = mDimensions.second.y.has_value();
            if (mDimensions.first == anchor_type::ABS)
            {
                if (bHasX && bHasY)
                {
                    set_min_dimensions(vector2f(
                        mDimensions.second.x.value(),
                        mDimensions.second.y.value()));
                }
                else if (bHasX)
                    set_min_width(mDimensions.second.x.value());
                else if (bHasY)
                    set_min_height(mDimensions.second.y.value());
            }
            else
            {
                gui::out << gui::warning << pMinNode->get_location() << " : "
                    << "\"RelDimension\" for ResizeBounds:Min is not yet supported. Skipped." << std::endl;
            }
        }

        if (const layout_node* pMaxNode = pResizeBoundsNode->try_get_child("Max"))
        {
            auto mDimensions = parse_dimension_(*pMaxNode);
            bool bHasX = mDimensions.second.x.has_value();
            bool bHasY = mDimensions.second.y.has_value();
            if (mDimensions.first == anchor_type::ABS)
            {
                if (bHasX && bHasY)
                {
                    set_max_dimensions(vector2f(
                        mDimensions.second.x.value(),
                        mDimensions.second.y.value()));
                }
                else if (bHasX)
                    set_max_width(mDimensions.second.x.value());
                else if (bHasY)
                    set_max_height(mDimensions.second.y.value());
            }
            else
            {
                gui::out << gui::warning << pMaxNode->get_location() << " : "
                    << "\"RelDimension\" for ResizeBounds:Max is not yet supported. Skipped." << std::endl;
            }
        }
    }
}

void frame::parse_title_region_node_(const layout_node& mNode)
{
    if (const layout_node* pTitleRegionNode = mNode.try_get_child("TitleRegion"))
    {
        create_title_region();

        if (pTitleRegion_)
            pTitleRegion_->parse_layout(*pTitleRegionNode);
    }
}

void frame::parse_backdrop_node_(const layout_node& mNode)
{
    if (const layout_node* pBackdropNode = mNode.try_get_child("Backdrop"))
    {
        std::unique_ptr<backdrop> pBackdrop(new backdrop(*this));

        pBackdrop->set_background(get_manager().parse_file_name(
            pBackdropNode->get_attribute_value_or<std::string>("bgFile", "")));

        pBackdrop->set_edge(get_manager().parse_file_name(
            pBackdropNode->get_attribute_value_or<std::string>("edgeFile", "")));

        pBackdrop->set_background_tilling(pBackdropNode->get_attribute_value_or<bool>("tile", false));

        if (const layout_node* pBGInsetsNode = pBackdropNode->try_get_child("BackgroundInsets"))
        {
            const layout_node* pAbsInsetNode = pBGInsetsNode->try_get_child("AbsInset");
            const layout_node* pRelInsetNode = pBGInsetsNode->try_get_child("RelInset");

            if (pAbsInsetNode && pRelInsetNode)
            {
                gui::out << gui::warning << pBGInsetsNode->get_location() << " : "
                    "BackgroundInsets node can only contain one of AbsInset or RelInset, but not both. "
                    "RelInset ignored." << std::endl;
            }

            if (!pAbsInsetNode && !pRelInsetNode)
            {
                gui::out << gui::warning << pBGInsetsNode->get_location() << " : "
                    "BackgroundInsets node must contain one of AbsInset or RelInset." << std::endl;
                return;
            }

            if (pAbsInsetNode)
            {
                pBackdrop->set_background_insets(bounds2f(
                    pAbsInsetNode->get_attribute_value_or<float>("left", 0.0f),
                    pAbsInsetNode->get_attribute_value_or<float>("right", 0.0f),
                    pAbsInsetNode->get_attribute_value_or<float>("top", 0.0f),
                    pAbsInsetNode->get_attribute_value_or<float>("bottom", 0.0f)
                ));
            }
            else
            {
                gui::out << gui::warning << pRelInsetNode->get_location() << " : "
                    << "RelInset for Backdrop:BackgroundInsets is not yet supported (" << sName_ << ")." << std::endl;
            }
        }

        if (const layout_node* pEdgeInsetsNode = pBackdropNode->try_get_child("EdgeInsets"))
        {
            const layout_node* pAbsInsetNode = pEdgeInsetsNode->try_get_child("AbsInset");
            const layout_node* pRelInsetNode = pEdgeInsetsNode->try_get_child("RelInset");

            if (pAbsInsetNode && pRelInsetNode)
            {
                gui::out << gui::warning << pEdgeInsetsNode->get_location() << " : "
                    "EdgeInsets node can only contain one of AbsInset or RelInset, but not both. "
                    "RelInset ignored." << std::endl;
            }

            if (!pAbsInsetNode && !pRelInsetNode)
            {
                gui::out << gui::warning << pEdgeInsetsNode->get_location() << " : "
                    "EdgeInsets node must contain one of AbsInset or RelInset." << std::endl;
                return;
            }

            if (pAbsInsetNode)
            {
                pBackdrop->set_edge_insets(bounds2f(
                    pAbsInsetNode->get_attribute_value_or<float>("left", 0.0f),
                    pAbsInsetNode->get_attribute_value_or<float>("right", 0.0f),
                    pAbsInsetNode->get_attribute_value_or<float>("top", 0.0f),
                    pAbsInsetNode->get_attribute_value_or<float>("bottom", 0.0f)
                ));
            }
            else
            {
                gui::out << gui::warning << pRelInsetNode->get_location() << " : "
                    << "RelInset for Backdrop:EdgeInsets is not yet supported (" << sName_ << ")." << std::endl;
            }
        }

        if (const layout_node* pColorNode = pBackdropNode->try_get_child("BackgroundColor"))
            pBackdrop->set_background_color(parse_color_node_(*pColorNode));

        if (const layout_node* pColorNode = pBackdropNode->try_get_child("EdgeColor"))
            pBackdrop->set_edge_color(parse_color_node_(*pColorNode));

        if (const layout_node* pEdgeSizeNode = pBackdropNode->try_get_child("EdgeSize"))
        {
            const layout_node* pAbsValueNode = pEdgeSizeNode->try_get_child("AbsValue");
            const layout_node* pRelValueNode = pEdgeSizeNode->try_get_child("RelValue");

            if (pAbsValueNode && pRelValueNode)
            {
                gui::out << gui::warning << pEdgeSizeNode->get_location() << " : "
                    "EdgeSize node can only contain one of AbsValue or RelValue, but not both. "
                    "RelValue ignored." << std::endl;
            }

            if (!pAbsValueNode && !pRelValueNode)
            {
                gui::out << gui::warning << pEdgeSizeNode->get_location() << " : "
                    "EdgeSize node must contain one of AbsValue or RelValue." << std::endl;
                return;
            }

            if (pAbsValueNode)
            {
                pBackdrop->set_edge_size(pAbsValueNode->get_attribute_value_or("x", 0.0f));
            }
            else
            {
                gui::out << gui::warning << pRelValueNode->get_location() << " : "
                    << "RelValue for Backdrop:EdgeSize is not yet supported (" << sName_ << ")." << std::endl;
            }
        }

        if (const layout_node* pTileSizeNode = pBackdropNode->try_get_child("TileSize"))
        {
            const layout_node* pAbsValueNode = pTileSizeNode->try_get_child("AbsValue");
            const layout_node* pRelValueNode = pTileSizeNode->try_get_child("RelValue");

            if (pAbsValueNode && pRelValueNode)
            {
                gui::out << gui::warning << pTileSizeNode->get_location() << " : "
                    "TileSize node can only contain one of AbsValue or RelValue, but not both. "
                    "RelValue ignored." << std::endl;
            }

            if (!pAbsValueNode && !pRelValueNode)
            {
                gui::out << gui::warning << pTileSizeNode->get_location() << " : "
                    "TileSize node must contain one of AbsValue or RelValue." << std::endl;
                return;
            }

            if (pAbsValueNode)
            {
                pBackdrop->set_tile_size(pAbsValueNode->get_attribute_value_or("x", 0.0f));
            }
            else
            {
                gui::out << gui::warning << pRelValueNode->get_location() << " : "
                    << "RelValue for Backdrop:TileSize is not yet supported (" << sName_ << ")." << std::endl;
            }
        }

        set_backdrop(std::move(pBackdrop));
    }
}

void frame::parse_hit_rect_insets_node_(const layout_node& mNode)
{
    if (const layout_node* pHitRectBlock = mNode.try_get_child("HitRectInsets"))
    {
        const layout_node* pAbsInsetNode = pHitRectBlock->try_get_child("AbsInset");
        const layout_node* pRelInsetNode = pHitRectBlock->try_get_child("RelInset");

        if (pAbsInsetNode && pRelInsetNode)
        {
            gui::out << gui::warning << pHitRectBlock->get_location() << " : "
                "HitRectInsets node can only contain one of AbsInset or RelInset, but not both. "
                "RelInset ignored." << std::endl;
        }

        if (!pAbsInsetNode && !pRelInsetNode)
        {
            gui::out << gui::warning << pHitRectBlock->get_location() << " : "
                "HitRectInsets node must contain one of AbsInset or RelInset." << std::endl;
            return;
        }

        if (pAbsInsetNode)
        {
            set_abs_hit_rect_insets(bounds2f(
                pAbsInsetNode->get_attribute_value_or<float>("left", 0.0f),
                pAbsInsetNode->get_attribute_value_or<float>("right", 0.0f),
                pAbsInsetNode->get_attribute_value_or<float>("top", 0.0f),
                pAbsInsetNode->get_attribute_value_or<float>("bottom", 0.0f)
            ));
        }
        else
        {
            set_rel_hit_rect_insets(bounds2f(
                pRelInsetNode->get_attribute_value_or<float>("left", 0.0f),
                pRelInsetNode->get_attribute_value_or<float>("right", 0.0f),
                pRelInsetNode->get_attribute_value_or<float>("top", 0.0f),
                pRelInsetNode->get_attribute_value_or<float>("bottom", 0.0f)
            ));
        }
    }
}

utils::observer_ptr<layered_region> frame::parse_region_(const layout_node& mNode,
    const std::string& sLayer, const std::string& sType)
{
    try
    {
        auto mAttr = parse_core_attributes(get_manager(), mNode, observer_from(this));

        std::string sObjectType = mAttr.sObjectType;
        if (!sType.empty())
            sObjectType = sType;

        auto pRegion = create_region(
            parse_layer_type(sLayer), sObjectType, mAttr.sName, mAttr.lInheritance);

        if (!pRegion)
            return nullptr;

        try
        {
            pRegion->parse_layout(mNode);
            pRegion->notify_loaded();
            return pRegion;
        }
        catch (...)
        {
            pRegion->release_from_parent();
            throw;
        }
    }
    catch (const exception& e)
    {
        gui::out << gui::error << e.get_description() << std::endl;
    }

    return nullptr;
}

void frame::parse_layers_node_(const layout_node& mNode)
{
    if (const layout_node* pLayersNode = mNode.try_get_child("Layers"))
    {
        for (const layout_node& mLayerNode : pLayersNode->get_children())
        {
            if (mLayerNode.get_name() != "Layer" && mLayerNode.get_name() != "")
            {
                gui::out << gui::warning << mLayerNode.get_location() << " : "
                    << "unexpected node '" << mLayerNode.get_name() << "'; ignored." << std::endl;
                continue;
            }

            std::string sLevel = mLayerNode.get_attribute_value_or<std::string>("level", "ARTWORK");
            for (const layout_node& mRegionNode : mLayerNode.get_children())
            {
                parse_region_(mRegionNode, sLevel, "");
            }
        }
    }
}

utils::observer_ptr<frame> frame::parse_child_(const layout_node& mNode,
    const std::string& sType)
{
    try
    {
        auto mAttr = parse_core_attributes(get_manager(), mNode, observer_from(this));

        std::string sObjectType = mAttr.sObjectType;
        if (!sType.empty())
            sObjectType = sType;

        utils::observer_ptr<frame> pFrame = create_child(
            sObjectType, mAttr.sName, mAttr.lInheritance);

        if (!pFrame)
            return nullptr;

        try
        {
            pFrame->set_addon(get_manager().get_current_addon());
            pFrame->parse_layout(mNode);
            pFrame->notify_loaded();
            return pFrame;
        }
        catch (...)
        {
            pFrame->release_from_parent();
            throw;
        }
    }
    catch (const exception& e)
    {
        gui::out << gui::error << e.get_description() << std::endl;
    }

    return nullptr;
}

void frame::parse_frames_node_(const layout_node& mNode)
{
    if (const layout_node* pFramesNode = mNode.try_get_child("Frames"))
    {
        for (const layout_node& mElemNode : pFramesNode->get_children())
        {
            parse_child_(mElemNode, "");
        }
    }
}

void frame::parse_scripts_node_(const layout_node& mNode)
{
    if (const layout_node* pScriptsNode = mNode.try_get_child("Scripts"))
    {
        for (const layout_node& mScriptNode : pScriptsNode->get_children())
        {
            std::string sName = std::string(mScriptNode.get_name());

            const layout_attribute* pNode = &mScriptNode;
            if (const layout_attribute* pRun = mScriptNode.try_get_attribute("run"))
                pNode = pRun;

            std::string sScript = std::string(pNode->get_value());
            script_info mInfo{std::string(pNode->get_filename()),
                    static_cast<uint>(pNode->get_value_line_number())};

            if (mScriptNode.get_attribute_value_or<bool>("override", false))
                set_script(std::move(sName), std::move(sScript), std::move(mInfo));
            else
                add_script(std::move(sName), std::move(sScript), std::move(mInfo));
        }
    }
}
}
}
