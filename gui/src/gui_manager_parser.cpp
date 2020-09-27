#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_eventmanager.hpp"

#include <lxgui/xml_document.hpp>
#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_exception.hpp>
#include <lxgui/utils_string.hpp>

namespace lxgui {
namespace gui
{
manager::xml_core_attributes manager::parse_core_attributes(xml::block* pBlock, frame* pXMLParent)
{
    manager::xml_core_attributes mAttr;
    mAttr.sFrameType = pBlock->get_name();
    mAttr.sName = pBlock->get_attribute("name");

    if (pXMLParent)
    {
        mAttr.pParent = pXMLParent;

        if (pBlock->is_provided("virtual"))
        {
            gui::out << gui::warning << pBlock->get_location() << " : "
                << "Cannot use the \"virtual\" attribute on \"" << mAttr.sName << "\", "
                "because it is a nested uiobject. Attribute ignored." << std::endl;
        }
        if (pBlock->is_provided("parent"))
        {
            gui::out << gui::warning << pBlock->get_location() << " : "
                << "Cannot use the \"parent\" attribute on \"" << mAttr.sName << "\", "
                "because it is a nested uiobject. Attribute ignored." << std::endl;
        }
    }
    else
    {
        mAttr.bVirtual = utils::string_to_bool(pBlock->get_attribute("virtual"));
        std::string sParent = pBlock->get_attribute("parent");
        mAttr.pParent = get_uiobject_by_name(sParent)->down_cast<frame>();
        if (!sParent.empty() && !mAttr.pParent)
        {
            gui::out << gui::warning << pBlock->get_location() << " : "
                << "Cannot find \"" << mAttr.sName << "\"'s parent : \"" << sParent << "\". "
                "No parent given to that widget." << std::endl;
        }
    }

    mAttr.lInheritance = get_virtual_uiobject_list(pBlock->get_attribute("inherits"));

    return mAttr;
}

void manager::parse_xml_file_(const std::string& sFile, addon* pAddOn)
{
    xml::document mDoc(sFile, "interface/ui.def", gui::out);
    if (mDoc.check())
    {
        for (auto* pElemBlock : mDoc.get_main_block()->blocks())
        {
            if (pElemBlock->get_name() == "Script")
            {
                std::string sScriptFile = pAddOn->sDirectory + "/" + pElemBlock->get_attribute("file");

                try
                {
                    pLua_->do_file(sScriptFile);
                }
                catch (const lua::exception& e)
                {
                    std::string sError = e.get_description();

                    gui::out << gui::error << sError << std::endl;

                    event mEvent("LUA_ERROR");
                    mEvent.add(sError);
                    pEventManager_->fire_event(mEvent);
                }
            }
            else if (pElemBlock->get_name() == "Include")
            {
                this->parse_xml_file_(pAddOn->sDirectory + "/" + pElemBlock->get_attribute("file"), pAddOn);
            }
            else
            {
                try
                {
                    auto mAttr = parse_core_attributes(pElemBlock, nullptr);

                    frame* pFrame = nullptr;
                    if (mAttr.pParent)
                    {
                        pFrame = mAttr.pParent->create_child(mAttr.sFrameType, mAttr.sName, mAttr.lInheritance);
                    }
                    else
                    {
                        if (mAttr.bVirtual)
                            pFrame = create_virtual_root_frame(mAttr.sFrameType, mAttr.sName, mAttr.lInheritance);
                        else
                            pFrame = create_root_frame(mAttr.sFrameType, mAttr.sName, mAttr.lInheritance);
                    }

                    if (!pFrame)
                        continue;

                    pFrame->set_addon(get_current_addon());
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
}
}
}
