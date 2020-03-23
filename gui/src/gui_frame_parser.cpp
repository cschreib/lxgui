#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_backdrop.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/xml_document.hpp>
#include <lxgui/utils_string.hpp>

namespace gui
{
void frame::parse_block(xml::block* pBlock)
{
    parse_attributes_(pBlock);

    parse_size_block_(pBlock);
    parse_resize_bounds_block_(pBlock);
    parse_anchor_block_(pBlock);
    parse_title_region_block_(pBlock);
    parse_backdrop_block_(pBlock);
    parse_hit_rect_insets_block_(pBlock);

    parse_layers_block_(pBlock);
    parse_frames_block_(pBlock);
    parse_scripts_block_(pBlock);
}

void frame::parse_attributes_(xml::block* pBlock)
{
    std::string sParent = pBlock->get_attribute("parent");
    bool bVirtual = utils::string_to_bool(pBlock->get_attribute("virtual"));

    if (!sParent.empty())
    {
        if (!pParent_)
        {
            uiobject* pParent = pManager_->get_uiobject_by_name(sParent);
            std::string sName = pBlock->get_attribute("name");
            if (!pManager_->check_uiobject_name(sName))
            {
                throw exception(pBlock->get_location(),
                    "Cannot create an uiobject with an incorrect name. Skipped."
                );
            }

            if (!utils::has_no_content(sName))
            {
                if (pParent)
                {
                    set_parent(pParent);
                    if (bVirtual || pParent->is_virtual())
                        set_virtual();
                    set_name(sName);
                }
                else
                {
                    if (bVirtual)
                        set_virtual();
                    set_name(sName);

                    gui::out << gui::warning << pBlock->get_location() << " : "
                        << "Cannot find \"" << get_name() << "\"'s parent : \"" << sParent << "\". "
                        "No parent given to that widget." << std::endl;
                }
            }
            else
            {
                throw exception(pBlock->get_location(),
                    "Cannot create an uiobject with a blank name. Skipped."
                );
            }
        }
        else
        {
            if (bVirtual || pParent_->is_virtual())
                set_virtual();

            set_name(pBlock->get_attribute("name"));

            gui::out << gui::warning << pBlock->get_location() << " : "
                << "Cannot use the \"parent\" attribute on \"" << get_name() << "\", "
                "because it is a nested uiobject. Attribute ignored." << std::endl;
        }
    }
    else
    {
        if (bVirtual  || (pParent_ && pParent_->is_virtual()))
            set_virtual();

        set_name(pBlock->get_attribute("name"));
    }

    if (pManager_->get_uiobject_by_name(sName_))
    {
        throw exception(pBlock->get_location(),
            std::string(bVirtual ? "A virtual" : "An")+" object with the name \""+
            sName_+"\" already exists. Skipped."
        );
    }

    pManager_->add_uiobject(this);

    create_glue();

    if (pParentFrame_)
    {
        pParentFrame_->add_child(this);
        set_level(pParentFrame_->get_frame_level() + 1);
    }
    else
        set_level(0);

    std::string sInheritance = pBlock->get_attribute("inherits");
    if (!utils::has_no_content(sInheritance))
    {
        for (auto sParent : utils::cut(sInheritance, ","))
        {
            utils::trim(sParent, ' ');
            uiobject* pObj = pManager_->get_uiobject_by_name(sParent, true);
            if (pObj)
            {
                if (is_object_type(pObj->get_object_type()))
                {
                    // Inherit from the other frame
                    copy_from(pObj);
                }
                else
                {
                    gui::out << gui::warning << pBlock->get_location() << " : "
                        << "\"" << sName_ << "\" (" << "gui::" << lType_.back() << ") cannot inherit "
                        << "from \"" << sParent << "\" (" << pObj->get_object_type()
                        << "). Inheritance skipped." << std::endl;
                }
            }
            else
            {
                bool bNonVirtual = false;
                if (pManager_->get_uiobject_by_name(sParent))
                    bNonVirtual = true;

                gui::out << gui::warning << pBlock->get_location() << " : "
                    << "Cannot find inherited object \"" << sParent << "\""
                    << std::string(bNonVirtual ? " (object is not virtual)" : "")
                    << ". Inheritance skipped." << std::endl;
            }
        }
    }

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
                    utils::string_to_int(pDimBlock->get_attribute("x")),
                    utils::string_to_int(pDimBlock->get_attribute("y"))
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
                    utils::string_to_int(pDimBlock->get_attribute("x")),
                    utils::string_to_int(pDimBlock->get_attribute("y"))
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

        pBackdrop->set_backgrond_tilling(utils::string_to_bool(pBackdropBlock->get_attribute("tile")));

        xml::block* pBGInsetsBlock = pBackdropBlock->get_block("BackgroundInsets");
        if (pBGInsetsBlock)
        {
            xml::block* pInsetBlock = pBGInsetsBlock->get_radio_block();
            if (pInsetBlock->get_name() == "AbsInset")
            {
                pBackdrop->set_background_insets(
                    utils::string_to_int(pInsetBlock->get_attribute("left")),
                    utils::string_to_int(pInsetBlock->get_attribute("right")),
                    utils::string_to_int(pInsetBlock->get_attribute("top")),
                    utils::string_to_int(pInsetBlock->get_attribute("bottom"))
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
                    utils::string_to_int(pInsetBlock->get_attribute("left")),
                    utils::string_to_int(pInsetBlock->get_attribute("right")),
                    utils::string_to_int(pInsetBlock->get_attribute("top")),
                    utils::string_to_int(pInsetBlock->get_attribute("bottom"))
                );
            }
            else
            {
                gui::out << gui::warning << pInsetBlock->get_location() << " : "
                    << "RelInset for Backdrop:BackgroundInsets is not yet supported (" << sName_ << ")." << std::endl;
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
                pBackdrop->set_edge_size(utils::string_to_uint(pSizeBlock->get_attribute("x")));
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
                pBackdrop->set_tile_size(utils::string_to_uint(pTileBlock->get_attribute("x")));
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
                utils::string_to_int(pInsetBlock->get_attribute("left")),
                utils::string_to_int(pInsetBlock->get_attribute("right")),
                utils::string_to_int(pInsetBlock->get_attribute("top")),
                utils::string_to_int(pInsetBlock->get_attribute("bottom"))
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
                layered_region* pRegion = pManager_->create_layered_region(pRegionBlock->get_name());
                if (pRegion)
                {
                    try
                    {
                        pRegion->set_parent(this);
                        pRegion->set_draw_layer(sLevel);
                        pRegion->parse_block(pRegionBlock);
                    }
                    catch (const exception& e)
                    {
                        delete pRegion;
                        gui::out << gui::error << e.get_description();
                    }
                    catch (...)
                    {
                        delete pRegion;
                        throw;
                    }
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
            frame* pFrame = pManager_->create_frame(pElemBlock->get_name());
            if (pFrame)
            {
                try
                {
                    pFrame->set_addon(pManager_->get_current_addon());
                    pFrame->set_parent(this);
                    pFrame->parse_block(pElemBlock);
                    pFrame->notify_loaded();
                }
                catch (const exception& e)
                {
                    delete pFrame;
                    gui::out << gui::error << e.get_description();
                }
                catch (...)
                {
                    delete pFrame;
                    throw;
                }
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
            define_script(
                pScriptBlock->get_name(), pScriptBlock->get_value(),
                pScriptBlock->get_file(), pScriptBlock->get_line_nbr()
            );
        }
    }
}
}
