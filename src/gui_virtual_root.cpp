#include "lxgui/gui_virtual_root.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_frame.hpp"

namespace lxgui {
namespace gui
{

virtual_root::virtual_root(manager& mManager, registry& mNonVirtualRegistry) :
    frame_container(mManager.get_factory(), mObjectRegistry_, nullptr),
    mManager_(mManager), mObjectRegistry_(mNonVirtualRegistry)
{
}

virtual_root::~virtual_root()
{
    // Must be done before we destroy the registry
    clear_frames_();
}

}
}
