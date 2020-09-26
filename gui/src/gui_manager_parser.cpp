#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_eventmanager.hpp"

#include <lxgui/xml_document.hpp>
#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_exception.hpp>

namespace lxgui {
namespace gui
{
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
                // TODO: make that use create_root_frame and create_child
                std::unique_ptr<frame> pFrame = create_frame(pElemBlock->get_name());
                if (!pFrame)
                    continue;

                try
                {
                    pFrame->set_addon(get_current_addon());
                    pFrame->parse_block(pElemBlock);
                    pFrame->notify_loaded();

                    if (pFrame->get_parent())
                        pFrame->get_parent()->add_child(std::move(pFrame));
                    else
                        add_root_frame(std::move(pFrame));
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
