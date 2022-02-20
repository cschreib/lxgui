#include "lxgui/gui_frame_container.hpp"

#include "lxgui/gui_factory.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/utils_std.hpp"

namespace lxgui::gui {

frame_container::frame_container(factory& fac, registry& reg, frame_renderer* rdr) :
    factory_(fac), registry_(reg), renderer_(rdr) {}

utils::observer_ptr<frame> frame_container::create_root_frame_(const region_core_attributes& attr) {
    auto new_frame = factory_.create_frame(registry_, renderer_, attr);
    if (!new_frame)
        return nullptr;

    return add_root_frame(std::move(new_frame));
}

utils::observer_ptr<frame> frame_container::add_root_frame(utils::owner_ptr<frame> obj) {
    utils::observer_ptr<frame> added_frame = obj;
    root_frames_.push_back(std::move(obj));
    return added_frame;
}

utils::owner_ptr<frame> frame_container::remove_root_frame(const utils::observer_ptr<frame>& obj) {
    frame* frame_raw = obj.get();
    if (!frame_raw)
        return nullptr;

    auto iter = utils::find_if(
        root_frames_, [&](const auto& root_frame) { return root_frame.get() == frame_raw; });

    if (iter == root_frames_.end())
        return nullptr;

    // NB: the iterator is not removed yet; it will be removed later in garbage_collect().
    return std::move(*iter);
}

frame_container::root_frame_list_view frame_container::get_root_frames() {
    return root_frame_list_view(root_frames_);
}

frame_container::const_root_frame_list_view frame_container::get_root_frames() const {
    return const_root_frame_list_view(root_frames_);
}

void frame_container::garbage_collect() {
    auto iter_remove = std::remove_if(
        root_frames_.begin(), root_frames_.end(), [](auto& obj) { return obj == nullptr; });

    root_frames_.erase(iter_remove, root_frames_.end());
}

void frame_container::clear_frames_() {
    root_frames_.clear();
}

} // namespace lxgui::gui
