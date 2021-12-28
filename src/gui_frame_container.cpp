#include "lxgui/gui_frame_container.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/gui_virtual_registry.hpp"
#include "lxgui/gui_factory.hpp"
#include "lxgui/input.hpp"

#include <lxgui/utils_std.hpp>

namespace lxgui {
namespace gui
{

frame_container::frame_container(manager& mManager) : mManager_(mManager)
{
}

utils::observer_ptr<frame> frame_container::create_root_frame_(
    registry& mRegistry, const std::string& sClassName, const std::string& sName,
    bool bVirtual, const std::vector<utils::observer_ptr<const uiobject>>& lInheritance)
{
    auto pNewFrame = get_manager().get_factory().create_frame(
        mRegistry, sClassName, bVirtual, sName, nullptr);

    if (!pNewFrame)
        return nullptr;

    for (const auto& pObj : lInheritance)
    {
        if (!pNewFrame->is_object_type(pObj->get_object_type()))
        {
            gui::out << gui::warning << "gui::frame_container : "
                << "\"" << pNewFrame->get_name() << "\" (" << pNewFrame->get_object_type()
                << ") cannot inherit from \"" << pObj->get_name() << "\" (" << pObj->get_object_type()
                << "). Inheritance skipped." << std::endl;
            continue;
        }

        // Inherit from the other frame
        pNewFrame->copy_from(*pObj);
    }

    return add_root_frame(std::move(pNewFrame));
}

utils::observer_ptr<frame> frame_container::add_root_frame(utils::owner_ptr<frame> pFrame)
{
    utils::observer_ptr<frame> pAddedFrame = pFrame;
    lRootFrameList_.push_back(std::move(pFrame));
    return pAddedFrame;
}

utils::owner_ptr<frame> frame_container::remove_root_frame(const utils::observer_ptr<frame>& pFrame)
{
    frame* pFrameRaw = pFrame.get();
    if (!pFrameRaw)
        return nullptr;

    auto mIter = utils::find_if(lRootFrameList_, [&](const auto& pObj)
    {
        return pObj.get() == pFrameRaw;
    });

    if (mIter == lRootFrameList_.end())
        return nullptr;

    // NB: the iterator is not removed yet; it will be removed later in update().
    return std::move(*mIter);
}

frame_container::root_frame_list_view frame_container::get_root_frames()
{
    return root_frame_list_view(lRootFrameList_);
}

frame_container::const_root_frame_list_view frame_container::get_root_frames() const
{
    return const_root_frame_list_view(lRootFrameList_);
}

void frame_container::garbage_collect()
{
    auto mIterRemove = std::remove_if(lRootFrameList_.begin(), lRootFrameList_.end(),
        [](auto& pObj) { return pObj == nullptr; });

    lRootFrameList_.erase(mIterRemove, lRootFrameList_.end());
}

registry& frame_container::get_registry() { return get_manager().get_registry(); }

const registry& frame_container::get_registry() const { return get_manager().get_registry(); }

registry& frame_container::get_virtual_registry() { return get_manager().get_virtual_registry(); }

const registry& frame_container::get_virtual_registry() const { return get_manager().get_virtual_registry(); }

}
}
