#include "lxgui/gui_framerenderer.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/utils_range.hpp"

namespace lxgui::gui {

// For debugging only
std::size_t count_frames(const std::array<strata, 8>& l_strata_list) {
    std::size_t ui_count = 0;
    for (std::size_t ui_strata = 0; ui_strata < l_strata_list.size(); ++ui_strata) {
        for (const auto& m_level : utils::range::value(l_strata_list[ui_strata].l_level_list)) {
            ui_count += m_level.l_frame_list.size();
        }
    }

    return ui_count;
}

// For debugging only
void print_frames(const std::array<strata, 8>& l_strata_list) {
    for (std::size_t ui_strata = 0; ui_strata < l_strata_list.size(); ++ui_strata) {
        if (l_strata_list[ui_strata].l_level_list.empty())
            continue;
        gui::out << "strata[" << ui_strata << "]" << std::endl;
        for (const auto& m_level : l_strata_list[ui_strata].l_level_list) {
            if (m_level.second.l_frame_list.empty())
                continue;
            gui::out << "  level[" << m_level.first << "]" << std::endl;
            for (const auto& p_frame : m_level.second.l_frame_list)
                gui::out << "    " << p_frame.get() << " " << p_frame->get_name() << std::endl;
        }
    }
}

void frame_renderer::notify_strata_needs_redraw_(strata& m_strata) {
    m_strata.b_redraw = true;
}

void frame_renderer::notify_strata_needs_redraw(frame_strata m_strata) {
    notify_strata_needs_redraw_(l_strata_list_[static_cast<std::size_t>(m_strata)]);
}

void frame_renderer::notify_rendered_frame(
    const utils::observer_ptr<frame>& p_frame, bool b_rendered) {
    if (!p_frame)
        return;

    if (p_frame->get_frame_strata() == frame_strata::parent) {
        throw gui::exception("gui::frame_renderer", "cannot use PARENT strata for renderer");
    }

    const auto m_frame_strata = p_frame->get_frame_strata();
    auto&      m_strata      = l_strata_list_[static_cast<std::size_t>(m_frame_strata)];

    if (b_rendered)
        add_to_strata_list_(m_strata, p_frame);
    else
        remove_from_strata_list_(m_strata, p_frame);

    notify_strata_needs_redraw_(m_strata);
}

void frame_renderer::notify_frame_strata_changed(
    const utils::observer_ptr<frame>& p_frame, frame_strata m_old_strata, frame_strata m_new_strata) {
    if (m_old_strata == frame_strata::parent || m_new_strata == frame_strata::parent) {
        throw gui::exception("gui::frame_renderer", "cannot use PARENT strata for renderer");
    }

    auto& m_old = l_strata_list_[static_cast<std::size_t>(m_old_strata)];
    auto& m_new = l_strata_list_[static_cast<std::size_t>(m_new_strata)];
    remove_from_strata_list_(m_old, p_frame);
    add_to_strata_list_(m_new, p_frame);

    notify_strata_needs_redraw_(m_old);
    notify_strata_needs_redraw_(m_new);
}

void frame_renderer::notify_frame_level_changed(
    const utils::observer_ptr<frame>& p_frame, int i_old_level, int i_new_level) {
    if (p_frame->get_frame_strata() == frame_strata::parent) {
        throw gui::exception("gui::frame_renderer", "cannot use PARENT strata for renderer");
    }

    const auto m_frame_strata = p_frame->get_frame_strata();
    auto&      m_strata      = l_strata_list_[static_cast<std::size_t>(m_frame_strata)];
    auto&      l_level_list   = m_strata.l_level_list;

    auto m_iter_old = l_level_list.find(i_old_level);
    if (m_iter_old != l_level_list.end()) {
        remove_from_level_list_(m_iter_old->second, p_frame);
        if (m_iter_old->second.l_frame_list.empty())
            m_strata.l_level_list.erase(m_iter_old);
    }

    auto m_iter_new = l_level_list.find(i_new_level);
    if (m_iter_new == l_level_list.end()) {
        m_iter_new                 = l_level_list.insert(std::make_pair(i_new_level, level{})).first;
        m_iter_new->second.p_strata = &m_strata;
    }

    add_to_level_list_(m_iter_new->second, p_frame);

    notify_strata_needs_redraw_(m_strata);
}

utils::observer_ptr<const frame>
frame_renderer::find_topmost_frame(const std::function<bool(const frame&)>& m_predicate) const {
    // Iterate through the frames in reverse order from rendering (frame on top goes first)
    for (const auto& m_strata : utils::range::reverse(l_strata_list_)) {
        for (const auto& m_level : utils::range::reverse_value(m_strata.l_level_list)) {
            for (const auto& p_frame : utils::range::reverse(m_level.l_frame_list)) {
                if (const frame* p_raw_ptr = p_frame.get()) {
                    if (p_raw_ptr->is_visible()) {
                        if (auto p_topmost = p_raw_ptr->find_topmost_frame(m_predicate))
                            return p_topmost;
                    }
                }
            }
        }
    }

    return nullptr;
}

int frame_renderer::get_highest_level(frame_strata m_frame_strata) const {
    const auto& m_strata = l_strata_list_[static_cast<std::size_t>(m_frame_strata)];
    if (!m_strata.l_level_list.empty())
        return m_strata.l_level_list.rbegin()->first;

    return 0;
}

void frame_renderer::add_to_strata_list_(
    strata& m_strata, const utils::observer_ptr<frame>& p_frame) {
    int  i_new_level = p_frame->get_level();
    auto m_iter_new  = m_strata.l_level_list.find(i_new_level);
    if (m_iter_new == m_strata.l_level_list.end()) {
        m_iter_new = m_strata.l_level_list.insert(std::make_pair(i_new_level, level{})).first;
        m_iter_new->second.p_strata = &m_strata;
    }

    add_to_level_list_(m_iter_new->second, p_frame);
}

void frame_renderer::remove_from_strata_list_(
    strata& m_strata, const utils::observer_ptr<frame>& p_frame) {
    auto m_iter = m_strata.l_level_list.find(p_frame->get_level());
    if (m_iter == m_strata.l_level_list.end()) {
        throw gui::exception("gui::frame_renderer", "frame not found in this strata and level");
    }

    remove_from_level_list_(m_iter->second, p_frame);

    if (m_iter->second.l_frame_list.empty())
        m_strata.l_level_list.erase(m_iter);
}

void frame_renderer::add_to_level_list_(level& m_level, const utils::observer_ptr<frame>& p_frame) {
    m_level.l_frame_list.push_back(p_frame);
    notify_strata_needs_redraw_(*m_level.p_strata);
    b_strata_list_updated_ = true;
}

void frame_renderer::remove_from_level_list_(
    level& m_level, const utils::observer_ptr<frame>& p_frame) {
    auto m_iter = std::find(m_level.l_frame_list.begin(), m_level.l_frame_list.end(), p_frame);
    if (m_iter == m_level.l_frame_list.end()) {
        throw gui::exception("gui::frame_renderer", "frame not found in this strata and level");
    }

    m_level.l_frame_list.erase(m_iter);
    notify_strata_needs_redraw_(*m_level.p_strata);
    b_strata_list_updated_ = true;
}

void frame_renderer::render_strata_(const strata& m_strata) const {
    for (const auto& m_level : utils::range::value(m_strata.l_level_list)) {
        for (const auto& p_frame : m_level.l_frame_list) {
            p_frame->render();
        }
    }
}

void frame_renderer::clear_strata_list_() {
    for (auto& m_strata : l_strata_list_) {
        m_strata.l_level_list.clear();
        m_strata.p_render_target = nullptr;
        m_strata.b_redraw       = true;
    }

    b_strata_list_updated_ = true;
}

bool frame_renderer::has_strata_list_changed_() const {
    return b_strata_list_updated_;
}

void frame_renderer::reset_strata_list_changed_flag_() {
    b_strata_list_updated_ = false;
}

} // namespace lxgui::gui
