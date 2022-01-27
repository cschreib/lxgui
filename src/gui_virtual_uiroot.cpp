#include "lxgui/gui_virtual_uiroot.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_frame.hpp"

namespace lxgui {
namespace gui
{

virtual_uiroot::virtual_uiroot(manager& mManager, registry& mNonVirtualRegistry) :
    frame_container(mManager.get_factory(), mObjectRegistry_, nullptr),
    mManager_(mManager), mObjectRegistry_(mNonVirtualRegistry)
{
}

virtual_uiroot::~virtual_uiroot()
{
    // Must be done before we destroy the registry
    clear_frames_();
}

}
}
