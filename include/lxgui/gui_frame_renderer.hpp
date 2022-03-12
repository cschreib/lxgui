#ifndef LXGUI_GUI_FRAME_RENDERER_HPP
#define LXGUI_GUI_FRAME_RENDERER_HPP

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

    // Non-copiable, non-movable
    frame_renderer(const frame_renderer&) = delete;
    frame_renderer(frame_renderer&&)      = delete;
    frame_renderer& operator=(const frame_renderer&) = delete;
    frame_renderer& operator=(frame_renderer&&) = delete;

    /// Tells this renderer that one of its region requires redraw.
    virtual void notify_strata_needs_redraw(frame_strata strata_id);

    /**
     * \brief Tells this renderer that it should (or not) render another frame.
     * \param obj The frame to render
     * \param rendered 'true' if this renderer needs to render that new object
     */
    virtual void notify_rendered_frame(const utils::observer_ptr<frame>& obj, bool rendered);

    /**
     * \brief Tells this renderer that a frame has changed strata.
     * \param obj The frame which has changed
     * \param old_strata_id The old frame strata
     * \param new_strata_id The new frame strata
     */
    virtual void notify_frame_strata_changed(
        const utils::observer_ptr<frame>& obj,
        frame_strata                      old_strata_id,
        frame_strata                      new_strata_id);

    /**
     * \brief Tells this renderer that a frame has changed level.
     * \param obj The frame which has changed
     * \param old_level The old frame level
     * \param new_level The new frame level
     */
    virtual void
    notify_frame_level_changed(const utils::observer_ptr<frame>& obj, int old_level, int new_level);

    /**
     * \brief Returns the width and height of of this renderer's main render target (e.g., screen).
     * \return The render target dimensions
     */
    virtual vector2f get_target_dimensions() const = 0;

    /**
     * \brief Find the top-most frame matching the provided predicate
     * \param predicate A function returning 'true' if the frame can be selected
     * \return The topmost frame, or nullptr if none
     */
    utils::observer_ptr<const frame>
    find_topmost_frame(const std::function<bool(const frame&)>& predicate) const;

    /**
     * \brief Find the top-most frame matching the provided predicate
     * \param predicate A function returning 'true' if the frame can be selected
     * \return The topmost frame, or nullptr if none
     */
    utils::observer_ptr<frame>
    find_topmost_frame(const std::function<bool(const frame&)>& predicate) {
        return utils::const_pointer_cast<frame>(
            const_cast<const frame_renderer*>(this)->find_topmost_frame(predicate));
    }

    /**
     * \brief Returns the highest level on the provided strata.
     * \param strata_id The strata to inspect
     * \return The highest level on the provided strata
     */
    int get_highest_level(frame_strata strata_id) const;

protected:
    void add_to_strata_list_(strata& strata_obj, const utils::observer_ptr<frame>& obj);
    void remove_from_strata_list_(strata& strata_obj, const utils::observer_ptr<frame>& obj);
    void add_to_level_list_(level& level_obj, const utils::observer_ptr<frame>& obj);
    void remove_from_level_list_(level& level_obj, const utils::observer_ptr<frame>& obj);
    void clear_strata_list_();
    bool has_strata_list_changed_() const;
    void reset_strata_list_changed_flag_();
    void notify_strata_needs_redraw_(strata& strata_obj);

    void render_strata_(const strata& strata_obj) const;

    std::array<strata, 9> strata_list_;
    bool                  strata_list_updated_ = false;
};

} // namespace lxgui::gui

#endif
