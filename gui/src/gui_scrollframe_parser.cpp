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
        try
        {
            auto mAttr = pManager_->parse_core_attributes(pChildBlock, this);

            frame* pScrollChild = create_child(mAttr.sFrameType, mAttr.sName,
                    mAttr.lInheritance);

            if (!pScrollChild)
                return;

            this->set_scroll_child(remove_child(pScrollChild));

            pScrollChild->set_addon(pManager_->get_current_addon());
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
        }
        catch (const exception& e)
        {
            gui::out << gui::error << e.get_description() << std::endl;
        }
    }
}
}
}
