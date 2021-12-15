#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/gui_parser_common.hpp"

#include <lxgui/xml_document.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/utils_layout_node.hpp>

#include <sol/state.hpp>

namespace lxgui {
namespace gui
{

void set_block(utils::layout_node& mNode, xml::block* pBlock)
{
    mNode.set_location(pBlock->get_location());
    mNode.set_name(pBlock->get_name());
    mNode.set_value(pBlock->get_value());

    for (const auto& mAttr : pBlock->get_attributes())
    {
        if (mAttr.second.bFound)
        {
            auto& mAttrib = mNode.add_attribute();
            mAttrib.set_location(pBlock->get_location());
            mAttrib.set_name(mAttr.second.sName);
            mAttrib.set_value(mAttr.second.sValue);
        }
    }

    for (auto* pElemBlock : pBlock->blocks())
    {
        auto& mChild = mNode.add_child();
        set_block(mChild, pElemBlock);
    }
}

void manager::parse_xml_file_(const std::string& sFile, addon* pAddOn)
{
    utils::layout_node mRoot;
    {
        xml::document mDoc(sFile, "interface/ui.def", gui::out);
        if (!mDoc.check())
            return;

        set_block(mRoot, mDoc.get_main_block());
    }

    for (const auto& mNode : mRoot.get_children())
    {
        if (mNode.get_name() == "Script")
        {
            std::string sScriptFile = pAddOn->sDirectory + "/" + mNode.get_attribute_value<std::string>("file");

            try
            {
                pLua_->do_file(sScriptFile);
            }
            catch (const sol::error& e)
            {
                std::string sError = e.what();

                gui::out << gui::error << sError << std::endl;

                event mEvent("LUA_ERROR");
                mEvent.add(sError);
                fire_event(mEvent);
            }
        }
        else if (mNode.get_name() == "Include")
        {
            this->parse_xml_file_(pAddOn->sDirectory + "/" + mNode.get_attribute_value<std::string>("file"), pAddOn);
        }
        else
        {
            try
            {
                auto mAttr = parse_core_attributes(*this, mNode, nullptr);

                utils::observer_ptr<frame> pFrame;
                if (mAttr.pParent)
                {
                    pFrame = mAttr.pParent->create_child(mAttr.sObjectType, mAttr.sName, mAttr.lInheritance);
                }
                else
                {
                    if (mAttr.bVirtual)
                        pFrame = create_virtual_root_frame(mAttr.sObjectType, mAttr.sName, mAttr.lInheritance);
                    else
                        pFrame = create_root_frame(mAttr.sObjectType, mAttr.sName, mAttr.lInheritance);
                }

                if (!pFrame)
                    continue;

                pFrame->set_addon(get_current_addon());
                pFrame->parse_layout(mNode);
                pFrame->notify_loaded();
            }
            catch (const exception& e)
            {
                gui::out << gui::error << e.get_description() << std::endl;
            }
        }
    }
}

}
}
