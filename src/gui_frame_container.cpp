#include "lxgui/gui_frame_container.hpp"

#include "lxgui/gui_factory.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/utils_std.hpp"

namespace lxgui::gui {

frame_container::frame_container(
    factory& m_factory, registry& m_registry, frame_renderer* p_renderer) :
    m_factory_(m_factory), m_registry_(m_registry), p_renderer_(p_renderer) {}

utils::observer_ptr<frame>
frame_container::create_root_frame_(const region_core_attributes& m_attr) {
    auto p_new_frame = m_factory_.create_frame(m_registry_, p_renderer_, m_attr);
    if (!p_new_frame)
        return nullptr;

    return add_root_frame(std::move(p_new_frame));
}

utils::observer_ptr<frame> frame_container::add_root_frame(utils::owner_ptr<frame> p_frame) {
    utils::observer_ptr<frame> p_added_frame = p_frame;
    l_root_frame_list_.push_back(std::move(p_frame));
    return p_added_frame;
}

utils::owner_ptr<frame>
frame_container::remove_root_frame(const utils::observer_ptr<frame>& p_frame) {
    frame* p_frame_raw = p_frame.get();
    if (!p_frame_raw)
        return nullptr;

    auto m_iter = utils::find_if(
        l_root_frame_list_, [&](const auto& p_obj) { return p_obj.get() == p_frame_raw; });

    if (m_iter == l_root_frame_list_.end())
        return nullptr;

    // NB: the iterator is not removed yet; it will be removed later in garbage_collect().
    return std::move(*m_iter);
}

frame_container::root_frame_list_view frame_container::get_root_frames() {
    return root_frame_list_view(l_root_frame_list_);
}

frame_container::const_root_frame_list_view frame_container::get_root_frames() const {
    return const_root_frame_list_view(l_root_frame_list_);
}

void frame_container::garbage_collect() {
    auto m_iter_remove =
        std::remove_if(l_root_frame_list_.begin(), l_root_frame_list_.end(), [](auto& p_obj) {
            return p_obj == nullptr;
        });

    l_root_frame_list_.erase(m_iter_remove, l_root_frame_list_.end());
}

void frame_container::clear_frames_() {
    l_root_frame_list_.clear();
}

} // namespace lxgui::gui
