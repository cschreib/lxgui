#ifndef LXGUI_GUI_RENDERER_HPP
#define LXGUI_GUI_RENDERER_HPP

#include "lxgui/gui_strata.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/utils_observer.hpp"

#include <functional>

namespace lxgui::gui {

class frame;
class font;
class color;

/// Abstract class for layering and rendering frames.
class frame_renderer {
public:
    /// Default constructor
    frame_renderer() = default;

    /// Destructor
    virtual ~frame_renderer() = default;

    /// Non-copiable
    frame_renderer(const frame_renderer&) = delete;

    /// Non-movable
    frame_renderer(frame_renderer&&) = delete;

    /// Non-copiable
    frame_renderer& operator=(const frame_renderer&) = delete;

    /// Non-movable
    frame_renderer& operator=(frame_renderer&&) = delete;

    /// Tells this renderer that one of its region requires redraw.
    virtual void notify_strata_needs_redraw(frame_strata m_strata);

    /// Tells this renderer that it should (or not) render another frame.
    /** \param pFrame    The frame to render
     *   \param bRendered 'true' if this renderer needs to render that new object
     */
    virtual void notify_rendered_frame(const utils::observer_ptr<frame>& p_frame, bool b_rendered);

    /// Tells this renderer that a frame has changed strata.
    /** \param pFrame The frame which has changed
     *   \param mOldStrata The old frame strata
     *   \param mNewStrata The new frame strata
     */
    virtual void notify_frame_strata_changed(
        const utils::observer_ptr<frame>& p_frame,
        frame_strata                      m_old_strata,
        frame_strata                      m_new_strata);

    /// Tells this renderer that a frame has changed level.
    /** \param pFrame The frame which has changed
     *   \param old_level The old frame level
     *   \param new_level The new frame level
     */
    virtual void notify_frame_level_changed(
        const utils::observer_ptr<frame>& p_frame, int old_level, int new_level);

    /// Returns the width and height of of this renderer's main render target (e.g., screen).
    /** \return The render target dimensions
     */
    virtual vector2f get_target_dimensions() const = 0;

    /// Find the top-most frame matching the provided predicate
    /** \param mPredicate A function returning 'true' if the frame can be selected
     *   \return The topmost frame, or nullptr if none
     */
    utils::observer_ptr<const frame>
    find_topmost_frame(const std::function<bool(const frame&)>& m_predicate) const;

    /// Find the top-most frame matching the provided predicate
    /** \param mPredicate A function returning 'true' if the frame can be selected
     *   \return The topmost frame, or nullptr if none
     */
    utils::observer_ptr<frame>
    find_topmost_frame(const std::function<bool(const frame&)>& m_predicate) {
        return utils::const_pointer_cast<frame>(
            const_cast<const frame_renderer*>(this)->find_topmost_frame(m_predicate));
    }

    /// Returns the highest level on the provided strata.
    /** \param mframe_strata The strata to inspect
     *   \return The highest level on the provided strata
     */
    int get_highest_level(frame_strata mframe_strata) const;

protected:
    void add_to_strata_list_(strata& m_strata, const utils::observer_ptr<frame>& p_frame);
    void remove_from_strata_list_(strata& m_strata, const utils::observer_ptr<frame>& p_frame);
    void add_to_level_list_(level& m_level, const utils::observer_ptr<frame>& p_frame);
    void remove_from_level_list_(level& m_level, const utils::observer_ptr<frame>& p_frame);
    void clear_strata_list_();
    bool has_strata_list_changed_() const;
    void reset_strata_list_changed_flag_();
    void notify_strata_needs_redraw_(strata& m_strata);

    void render_strata_(const strata& m_strata) const;

    std::array<strata, 8> l_strata_list_;
    bool                  b_strata_list_updated_ = false;
};

} // namespace lxgui::gui

#endif
