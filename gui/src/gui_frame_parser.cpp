#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_backdrop.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_manager.hpp"

#include <lxgui/xml_document.hpp>
#include <lxgui/utils_string.hpp>

namespace lxgui {
namespace gui
{
void frame::parse_all_blocks_before_children_(xml::block* pBlock)
{
    parse_attributes_(pBlock);

    parse_size_block_(pBlock);
    parse_resize_bounds_block_(pBlock);
    parse_anchor_block_(pBlock);
    parse_title_region_block_(pBlock);
    parse_backdrop_block_(pBlock);
    parse_hit_rect_insets_block_(pBlock);

    parse_layers_block_(pBlock);
}

void frame::parse_block(xml::block* pBlock)
{
    parse_all_blocks_before_children_(pBlock);
    parse_frames_block_(pBlock);
    parse_scripts_block_(pBlock);
}

void frame::parse_attributes_(xml::block* pBlock)
{
    if (pBlock->is_provided("hidden") || !bInherits_)
        set_shown(!utils::string_to_bool(pBlock->get_attribute("hidden")));

    if ((pBlock->is_provided("setAllPoints") || !bInherits_) &&
        (utils::string_to_bool(pBlock->get_attribute("setAllPoints"))))
        set_all_points("$parent");

    if (pBlock->is_provided("alpha") || !bInherits_)
        set_alpha(utils::string_to_float(pBlock->get_attribute("alpha")));
    if (pBlock->is_provided("topLevel") || !bInherits_)
        set_top_level(utils::string_to_bool(pBlock->get_attribute("topLevel")));
    if (pBlock->is_provided("movable") || !bInherits_)
        set_movable(utils::string_to_bool(pBlock->get_attribute("movable")));
    if (pBlock->is_provided("resizable") || !bInherits_)
        set_resizable(utils::string_to_bool(pBlock->get_attribute("resizable")));
    if (pBlock->is_provided("frameStrata") || !bInherits_)
        set_frame_strata(pBlock->get_attribute("frameStrata"));
    if (pBlock->is_provided("frameLevel"))
    {
        if (!bVirtual_)
        {
            std::string sFrameLevel = pBlock->get_attribute("frameLevel");
            if (sFrameLevel != "PARENT")
                set_level(utils::string_to_int(sFrameLevel));
        }
        else
        {
            gui::out << gui::warning << pBlock->get_location() << " : "
                << "\"frameLevel\" is not allowed for virtual widgets. Ignored." << std::endl;
        }
    }
    if (pBlock->is_provided("enableMouse") || !bInherits_)
        enable_mouse(utils::string_to_bool(pBlock->get_attribute("enableMouse")));
    if (pBlock->is_provided("enableMouseWheel") || !bInherits_)
        enable_mouse_wheel(utils::string_to_bool(pBlock->get_attribute("enableMouseWheel")));
    if (pBlock->is_provided("enableKeyboard") || !bInherits_)
        enable_keyboard(utils::string_to_bool(pBlock->get_attribute("enableKeyboard")));
    if (pBlock->is_provided("clampedToScreen") || !bInherits_)
        set_clamped_to_screen(utils::string_to_bool(pBlock->get_attribute("clampedToScreen")));
}

void frame::parse_resize_bounds_block_(xml::block* pBlock)
{
    xml::block* pResizeBoundsBlock = pBlock->get_block("ResizeBounds");
    if (pResizeBoundsBlock)
    {
        xml::block* pMinBlock = pResizeBoundsBlock->get_block("Min");
        if (pMinBlock)
        {
            xml::block* pDimBlock = pMinBlock->get_radio_block();
            if (pDimBlock->get_name() == "AbsDimension")
            {
                set_min_resize(
                    utils::string_to_float(pDimBlock->get_attribute("x")),
                    utils::string_to_float(pDimBlock->get_attribute("y"))
                );
            }
            else if (pDimBlock->get_name() == "RelDimension")
            {
                gui::out << gui::warning << pDimBlock->get_location() << " : "
                    << "\"RelDimension\" for ResizeBounds:Min is not yet supported. Skipped." << std::endl;
            }
        }

        xml::block* pMaxBlock = pResizeBoundsBlock->get_block("Max");
        if (pMaxBlock)
        {
            xml::block* pDimBlock = pMaxBlock->get_radio_block();
            if (pDimBlock->get_name() == "AbsDimension")
            {
                set_max_resize(
                    utils::string_to_float(pDimBlock->get_attribute("x")),
                    utils::string_to_float(pDimBlock->get_attribute("y"))
                );
            }
            else if (pDimBlock->get_name() == "RelDimension")
            {
                gui::out << gui::warning << pDimBlock->get_location() << " : "
                    << "\"RelDimension\" for ResizeBounds:Max is not yet supported. Skipped." << std::endl;
            }
        }
    }
}

void frame::parse_title_region_block_(xml::block* pBlock)
{
    xml::block* pTitleRegionBlock = pBlock->get_block("TitleRegion");
    if (pTitleRegionBlock)
    {
        create_title_region();

        if (pTitleRegion_)
            pTitleRegion_->parse_block(pTitleRegionBlock);
    }
}

void frame::parse_backdrop_block_(xml::block* pBlock)
{
    xml::block* pBackdropBlock = pBlock->get_block("Backdrop");
    if (pBackdropBlock)
    {
        std::unique_ptr<backdrop> pBackdrop(new backdrop(this));

        pBackdrop->set_background(pManager_->parse_file_name(
            pBackdropBlock->get_attribute("bgFile")
        ));
        pBackdrop->set_edge(pManager_->parse_file_name(
            pBackdropBlock->get_attribute("edgeFile")
        ));

        pBackdrop->set_background_tilling(utils::string_to_bool(pBackdropBlock->get_attribute("tile")));

        xml::block* pBGInsetsBlock = pBackdropBlock->get_block("BackgroundInsets");
        if (pBGInsetsBlock)
        {
            xml::block* pInsetBlock = pBGInsetsBlock->get_radio_block();
            if (pInsetBlock->get_name() == "AbsInset")
            {
                pBackdrop->set_background_insets(
                    utils::string_to_float(pInsetBlock->get_attribute("left")),
                    utils::string_to_float(pInsetBlock->get_attribute("right")),
                    utils::string_to_float(pInsetBlock->get_attribute("top")),
                    utils::string_to_float(pInsetBlock->get_attribute("bottom"))
                );
            }
            else
            {
                gui::out << gui::warning << pInsetBlock->get_location() << " : "
                    << "RelInset for Backdrop:BackgroundInsets is not yet supported (" << sName_ << ")." << std::endl;
            }
        }

        xml::block* pEdgeInsetsBlock = pBackdropBlock->get_block("EdgeInsets");
        if (pEdgeInsetsBlock)
        {
            xml::block* pInsetBlock = pEdgeInsetsBlock->get_radio_block();
            if (pInsetBlock->get_name() == "AbsInset")
            {
                pBackdrop->set_edge_insets(
                    utils::string_to_float(pInsetBlock->get_attribute("left")),
                    utils::string_to_float(pInsetBlock->get_attribute("right")),
                    utils::string_to_float(pInsetBlock->get_attribute("top")),
                    utils::string_to_float(pInsetBlock->get_attribute("bottom"))
                );
            }
            else
            {
                gui::out << gui::warning << pInsetBlock->get_location() << " : "
                    << "RelInset for Backdrop:EdgeInsets is not yet supported (" << sName_ << ")." << std::endl;
            }
        }

        xml::block* pColorBlock = pBackdropBlock->get_block("BackgroundColor");
        if (pColorBlock)
            pBackdrop->set_background_color(parse_color_block_(pColorBlock));

        pColorBlock = pBackdropBlock->get_block("EdgeColor");
        if (pColorBlock)
            pBackdrop->set_edge_color(parse_color_block_(pColorBlock));

        xml::block* pEdgeSizeBlock = pBackdropBlock->get_block("EdgeSize");
        if (pEdgeSizeBlock)
        {
            xml::block* pSizeBlock = pEdgeSizeBlock->get_radio_block();
            if (pSizeBlock->get_name() == "AbsValue")
            {
                pBackdrop->set_edge_size(utils::string_to_float(pSizeBlock->get_attribute("x")));
            }
            else
            {
                gui::out << gui::warning << pSizeBlock->get_location() << " : "
                    << "RelValue for Backdrop:EdgeSize is not yet supported (" << sName_ << ")." << std::endl;
            }
        }

        xml::block* pTileSizeBlock = pBackdropBlock->get_block("TileSize");
        if (pTileSizeBlock)
        {
            xml::block* pTileBlock = pTileSizeBlock->get_radio_block();
            if (pTileBlock->get_name() == "AbsValue")
                pBackdrop->set_tile_size(utils::string_to_float(pTileBlock->get_attribute("x")));
            else
            {
                gui::out << gui::warning << pTileBlock->get_location() << " : "
                    << "RelValue for Backdrop:TileSize is not yet supported (" << sName_ << ")." << std::endl;
            }
        }

        set_backdrop(std::move(pBackdrop));
    }
}

void frame::parse_hit_rect_insets_block_(xml::block* pBlock)
{
    xml::block* pHitRectBlock = pBlock->get_block("HitRectInsets");
    if (pHitRectBlock)
    {
        xml::block* pInsetBlock = pHitRectBlock->get_radio_block();
        if (pInsetBlock->get_name() == "AbsInset")
        {
            set_abs_hit_rect_insets(
                utils::string_to_float(pInsetBlock->get_attribute("left")),
                utils::string_to_float(pInsetBlock->get_attribute("right")),
                utils::string_to_float(pInsetBlock->get_attribute("top")),
                utils::string_to_float(pInsetBlock->get_attribute("bottom"))
            );
        }
        else if (pInsetBlock->get_name() == "RelInset")
        {
            set_rel_hit_rect_insets(
                utils::string_to_float(pInsetBlock->get_attribute("left")),
                utils::string_to_float(pInsetBlock->get_attribute("right")),
                utils::string_to_float(pInsetBlock->get_attribute("top")),
                utils::string_to_float(pInsetBlock->get_attribute("bottom"))
            );
        }
    }
}

void frame::parse_layers_block_(xml::block* pBlock)
{
    xml::block* pLayersBlock = pBlock->get_block("Layers");
    if (pLayersBlock)
    {
        for (auto* pLayerBlock : pLayersBlock->blocks())
        {
            std::string sLevel = pLayerBlock->get_attribute("level");
            for (auto* pRegionBlock : pLayerBlock->blocks())
            {
                auto pRegion = pManager_->create_layered_region(pRegionBlock->get_name());
                if (!pRegion)
                    continue;

                try
                {
                    pRegion->set_parent(this);
                    pRegion->set_draw_layer(sLevel);
                    pRegion->parse_block(pRegionBlock);
                    add_region(std::move(pRegion));
                }
                catch (const exception& e)
                {
                    gui::out << gui::error << e.get_description() << std::endl;
                }
                catch (...)
                {
                    throw;
                }
            }
        }
    }
}

void frame::parse_frames_block_(xml::block* pBlock)
{
    xml::block* pFramesBlock = pBlock->get_block("Frames");
    if (pFramesBlock)
    {
        for (auto* pElemBlock : pFramesBlock->blocks())
        {
            try
            {
                auto mAttr = pManager_->parse_core_attributes(pElemBlock, this);

                frame* pFrame = create_child(mAttr.sFrameType, mAttr.sName, mAttr.lInheritance);
                if (!pFrame)
                    continue;

                pFrame->set_addon(pManager_->get_current_addon());
                pFrame->parse_block(pElemBlock);
                pFrame->notify_loaded();
            }
            catch (const exception& e)
            {
                gui::out << gui::error << e.get_description() << std::endl;
            }
        }
    }
}

void frame::parse_scripts_block_(xml::block* pBlock)
{
    xml::block* pScriptsBlock = pBlock->get_block("Scripts");
    if (pScriptsBlock)
    {
        for (auto* pScriptBlock : pScriptsBlock->blocks())
        {
            bool bOverride = utils::string_to_bool(pScriptBlock->get_attribute("override"));
            if (bOverride)
            {
                set_script(
                    pScriptBlock->get_name(), pScriptBlock->get_value(),
                    script_info{pScriptBlock->get_file(), pScriptBlock->get_line_nbr()}
                );
            }
            else
            {
                add_script(
                    pScriptBlock->get_name(), pScriptBlock->get_value(),
                    script_info{pScriptBlock->get_file(), pScriptBlock->get_line_nbr()}
                );
            }
        }
    }
}
}
}
