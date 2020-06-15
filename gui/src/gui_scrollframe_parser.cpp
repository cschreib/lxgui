#include "lxgui/gui_scrollframe.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_manager.hpp"

#include <lxgui/xml_document.hpp>

namespace lxgui {
namespace gui
{
void scroll_frame::parse_block(xml::block* pBlock)
{
    frame::parse_block(pBlock);

    parse_scroll_child_block_(pBlock);
}

void scroll_frame::parse_scroll_child_block_(xml::block* pBlock)
{
    xml::block* pScrollChildBlock = pBlock->get_block("ScrollChild");
    if (pScrollChildBlock)
    {
        xml::block* pChildBlock = pScrollChildBlock->first();
        std::unique_ptr<frame> pScrollChild = pManager_->create_frame(pChildBlock->get_name());
        if (pScrollChild)
        {
            try
            {
                pScrollChild->set_addon(pManager_->get_current_addon());
                pScrollChild->set_parent(this);
                pScrollChild->parse_block(pChildBlock);
                pScrollChild->notify_loaded();

                xml::block* pAnchors = pChildBlock->get_block("anchors");
                if (pAnchors)
                {
                    gui::out << gui::warning << pAnchors->get_location() << " : "
                        << "Scroll child's anchors are ignored." << std::endl;
                }

                if (!pChildBlock->get_block("Size"))
                {
                    gui::out << gui::warning << pChildBlock->get_location() << " : "
                        "Scroll child needs its size to be defined in a Size block." << std::endl;
                }

                set_scroll_child(std::move(pScrollChild));
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
