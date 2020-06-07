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
                std::unique_ptr<uiobject> pUIObject = create_uiobject(pElemBlock->get_name());
                if (!pUIObject)
                    return;

                try
                {
                    std::unique_ptr<frame> pFrame(dynamic_cast<frame*>(pUIObject.release()));

                    if (pFrame)
                    {
                        pFrame->set_addon(get_current_addon());
                        pFrame->parse_block(pElemBlock);
                        pFrame->notify_loaded();

                        if (pFrame->get_parent())
                            dynamic_cast<frame*>(pFrame->get_parent())->add_child(std::move(pFrame));
                        else
                            add_root_uiobject(std::move(pFrame));
                    }
                    else
                    {
                        // TODO : Allow virtual regions to be created at root
                        gui::out << gui::warning << "gui::manager : " "Creating elements other than Frames at root "
                            "level is forbidden. Skipped." << std::endl;
                    }
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
