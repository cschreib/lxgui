#include "lxgui/gui_frame_renderer.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/utils_range.hpp"

namespace lxgui::gui {

// For debugging only
std::size_t count_frames(const std::array<strata, 8>& strata_list) {
    std::size_t count = 0;
    for (std::size_t strata_id = 0; strata_id < strata_list.size(); ++strata_id) {
        for (const auto& level_obj : utils::range::value(strata_list[strata_id].level_list)) {
            count += level_obj.frame_list.size();
        }
    }

    return count;
}

// For debugging only
void print_frames(const std::array<strata, 8>& strata_list) {
    for (std::size_t strata_id = 0; strata_id < strata_list.size(); ++strata_id) {
        if (strata_list[strata_id].level_list.empty())
            continue;
        gui::out << "strata[" << strata_id << "]" << std::endl;
        for (const auto& level_obj : strata_list[strata_id].level_list) {
            if (level_obj.second.frame_list.empty())
                continue;
            gui::out << "  level[" << level_obj.first << "]" << std::endl;
            for (const auto& obj : level_obj.second.frame_list)
                gui::out << "    " << obj.get() << " " << obj->get_name() << std::endl;
        }
    }
}

void frame_renderer::notify_strata_needs_redraw_(strata& strata) {
    strata.redraw_flag = true;
}

void frame_renderer::notify_strata_needs_redraw(frame_strata strata_id) {
    notify_strata_needs_redraw_(strata_list_[static_cast<std::size_t>(strata_id)]);
}

void frame_renderer::notify_rendered_frame(const utils::observer_ptr<frame>& obj, bool rendered) {
    if (!obj)
        return;

    const auto frame_strata = obj->get_effective_frame_strata();
    auto&      strata       = strata_list_[static_cast<std::size_t>(frame_strata)];

    if (rendered)
        add_to_strata_list_(strata, obj);
    else
        remove_from_strata_list_(strata, obj);

    notify_strata_needs_redraw_(strata);
}

void frame_renderer::notify_frame_strata_changed(
    const utils::observer_ptr<frame>& obj, frame_strata old_strata_id, frame_strata new_strata_id) {

    if (old_strata_id == frame_strata::parent || new_strata_id == frame_strata::parent) {
        throw gui::exception("gui::frame_renderer", "cannot use PARENT strata for renderer");
    }

    auto& old_strata = strata_list_[static_cast<std::size_t>(old_strata_id)];
    auto& new_strata = strata_list_[static_cast<std::size_t>(new_strata_id)];

    remove_from_strata_list_(old_strata, obj);
    add_to_strata_list_(new_strata, obj);

    notify_strata_needs_redraw_(old_strata);
    notify_strata_needs_redraw_(new_strata);
}

void frame_renderer::notify_frame_level_changed(
    const utils::observer_ptr<frame>& obj, int old_level, int new_level) {

    const auto strata_id  = obj->get_effective_frame_strata();
    auto&      strata_obj = strata_list_[static_cast<std::size_t>(strata_id)];
    auto&      level_list = strata_obj.level_list;

    if (auto iter_old = level_list.find(old_level); iter_old != level_list.end()) {
        remove_from_level_list_(iter_old->second, obj);

        if (iter_old->second.frame_list.empty())
            strata_obj.level_list.erase(iter_old);
    }

    auto iter_new = level_list.find(new_level);
    if (iter_new == level_list.end()) {
        iter_new = level_list.insert(std::make_pair(new_level, level{})).first;
    }

    add_to_level_list_(iter_new->second, obj);

    strata_list_updated_ = true;
    notify_strata_needs_redraw_(strata_obj);
}

utils::observer_ptr<const frame>
frame_renderer::find_topmost_frame(const std::function<bool(const frame&)>& predicate) const {
    // Iterate through the frames in reverse order from rendering (frame on top goes first)
    for (const auto& strata_obj : utils::range::reverse(strata_list_)) {
        for (const auto& level_obj : utils::range::reverse_value(strata_obj.level_list)) {
            for (const auto& obj : utils::range::reverse(level_obj.frame_list)) {
                if (const frame* raw_ptr = obj.get()) {
                    if (raw_ptr->is_visible()) {
                        if (auto topmost = raw_ptr->find_topmost_frame(predicate))
                            return topmost;
                    }
                }
            }
        }
    }

    return nullptr;
}

int frame_renderer::get_highest_level(frame_strata strata_id) const {
    const auto& strata_obj = strata_list_[static_cast<std::size_t>(strata_id)];
    if (!strata_obj.level_list.empty())
        return strata_obj.level_list.rbegin()->first;

    return 0;
}

void frame_renderer::add_to_strata_list_(
    strata& strata_obj, const utils::observer_ptr<frame>& obj) {

    int  new_level = obj->get_level();
    auto iter_new  = strata_obj.level_list.find(new_level);
    if (iter_new == strata_obj.level_list.end()) {
        iter_new = strata_obj.level_list.insert(std::make_pair(new_level, level{})).first;
    }

    add_to_level_list_(iter_new->second, obj);

    strata_list_updated_ = true;
    notify_strata_needs_redraw_(strata_obj);
}

void frame_renderer::remove_from_strata_list_(
    strata& strata_obj, const utils::observer_ptr<frame>& obj) {

    auto iter = strata_obj.level_list.find(obj->get_level());
    if (iter == strata_obj.level_list.end()) {
        throw gui::exception("gui::frame_renderer", "frame not found in this strata and level");
    }

    remove_from_level_list_(iter->second, obj);

    if (iter->second.frame_list.empty())
        strata_obj.level_list.erase(iter);

    strata_list_updated_ = true;
    notify_strata_needs_redraw_(strata_obj);
}

void frame_renderer::add_to_level_list_(level& level_obj, const utils::observer_ptr<frame>& obj) {

    level_obj.frame_list.push_back(obj);
}

void frame_renderer::remove_from_level_list_(
    level& level_obj, const utils::observer_ptr<frame>& obj) {

    auto iter = std::find(level_obj.frame_list.begin(), level_obj.frame_list.end(), obj);
    if (iter == level_obj.frame_list.end()) {
        throw gui::exception("gui::frame_renderer", "frame not found in this strata and level");
    }

    level_obj.frame_list.erase(iter);
}

void frame_renderer::render_strata_(const strata& strata_obj) const {
    for (const auto& level_obj : utils::range::value(strata_obj.level_list)) {
        for (const auto& obj : level_obj.frame_list) {
            obj->render();
        }
    }
}

void frame_renderer::clear_strata_list_() {
    for (auto& strata_obj : strata_list_) {
        strata_obj.level_list.clear();
        strata_obj.target      = nullptr;
        strata_obj.redraw_flag = true;
    }

    strata_list_updated_ = true;
}

bool frame_renderer::has_strata_list_changed_() const {
    return strata_list_updated_;
}

void frame_renderer::reset_strata_list_changed_flag_() {
    strata_list_updated_ = false;
}

} // namespace lxgui::gui
