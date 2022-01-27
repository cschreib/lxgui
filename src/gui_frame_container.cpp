#include "lxgui/gui_frame_container.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/gui_factory.hpp"

#include <lxgui/utils_std.hpp>

namespace lxgui {
namespace gui
{

frame_container::frame_container(factory& mFactory, registry& mRegistry, frame_renderer* pRenderer) :
    mFactory_(mFactory), mRegistry_(mRegistry), pRenderer_(pRenderer)
{
}

utils::observer_ptr<frame> frame_container::create_root_frame_(
    const uiobject_core_attributes& mAttr)
{
    auto pNewFrame = mFactory_.create_frame(mRegistry_, pRenderer_, mAttr);
    if (!pNewFrame)
        return nullptr;

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

    // NB: the iterator is not removed yet; it will be removed later in garbage_collect().
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

void frame_container::clear_frames_()
{
    lRootFrameList_.clear();
}

}
}
