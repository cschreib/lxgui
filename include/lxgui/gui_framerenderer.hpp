#ifndef LXGUI_GUI_RENDERER_HPP
#define LXGUI_GUI_RENDERER_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include <lxgui/utils_observer.hpp>
#include "lxgui/gui_strata.hpp"

namespace lxgui {
namespace gui
{
    class frame;
    class font;
    class color;

    /// Abstract class for layering and rendering frames.
    class frame_renderer
    {
    public :

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

        /// Tells this renderer that one of its widget requires redraw.
        virtual void notify_strata_needs_redraw(frame_strata mStrata);

        /// Tells this renderer that it should (or not) render another frame.
        /** \param pFrame    The frame to render
        *   \param bRendered 'true' if this renderer needs to render that new object
        */
        virtual void notify_rendered_frame(const utils::observer_ptr<frame>& pFrame, bool bRendered);

        /// Tells this renderer that a frame has changed strata.
        /** \param pFrame The frame which has changed
        *   \param mOldStrata The old frame strata
        *   \param mNewStrata The new frame strata
        */
        virtual void notify_frame_strata_changed(const utils::observer_ptr<frame>& pFrame,
            frame_strata mOldStrata, frame_strata mNewStrata);

        /// Tells this renderer that a frame has changed level.
        /** \param pFrame The frame which has changed
        *   \param iOldLevel The old frame level
        *   \param iNewLevel The new frame level
        */
        virtual void notify_frame_level_changed(const utils::observer_ptr<frame>& pFrame,
            int iOldLevel, int iNewLevel);

        /// Returns the width and height of of this renderer's main render target (e.g., screen).
        /** \return The render target dimensions
        */
        virtual vector2f get_target_dimensions() const = 0;

        /// Find the top-most frame at the provided coordinates
        /** \param mPosition The coordinates to look at
        *   \return The hovered frame, or nullptr if none
        */
        utils::observer_ptr<frame> find_hovered_frame(const vector2f& mPosition);

        /// Returns the highest level on the provided strata.
        /** \param mframe_strata The strata to inspect
        *   \return The highest level on the provided strata
        */
        int get_highest_level(frame_strata mframe_strata) const;

    protected :

        void add_to_strata_list_(strata& mStrata, const utils::observer_ptr<frame>& pFrame);
        void remove_from_strata_list_(strata& mStrata, const utils::observer_ptr<frame>& pFrame);
        void add_to_level_list_(level& mLevel, const utils::observer_ptr<frame>& pFrame);
        void remove_from_level_list_(level& mLevel, const utils::observer_ptr<frame>& pFrame);
        void clear_strata_list_();
        bool has_strata_list_changed_() const;
        void reset_strata_list_changed_flag_();
        void notify_strata_needs_redraw_(strata& mStrata);

        void render_strata_(const strata& mStrata) const;

        std::array<strata,8> lStrataList_;
        bool                 bStrataListUpdated_ = false;
    };
}
}

#endif
