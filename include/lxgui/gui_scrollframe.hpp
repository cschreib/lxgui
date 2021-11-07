#ifndef LXGUI_GUI_SCROLLFRAME_HPP
#define LXGUI_GUI_SCROLLFRAME_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_framerenderer.hpp"

namespace lxgui {
namespace gui
{
    class texture;

    /// A #frame with scrollable content.
    /** This frame has a special child frame, the "scroll child". The scroll
    *   child is rendered on a separate render target, which is then rendered
    *   on the screen. This allows clipping the content of the scroll child
    *   and only display a portion of it (as if scrolling on a page). The
    *   displayed portion is controlled by the scroll value, which can be
    *   changed in both the vertical and horizontal directions.
    *
    *   By default, the mouse wheel movement will not trigger any scrolling;
    *   this has to be explicitly implemented using the `OnMouseWheel` callback
    *   and the scroll_frame::set_horizontal_scroll function.
    *
    *   __Events.__ Hard-coded events available to all scroll frames,
    *   in addition to those from #frame:
    *
    *   - `OnHorizontalScroll`: Triggered by scroll_frame::set_horizontal_scroll.
    *   - `OnScrollRangeChanged`: Triggered whenever the range of the scroll value
    *   changes. This happens either when the size of the scrollable content
    *   changes, or when the size of the scroll frame changes.
    *   - `OnVerticalScroll`: Triggered by scroll_frame::set_vertical_scroll.
    */
    class scroll_frame : public frame, public frame_renderer
    {
    public :

        /// Constructor.
        explicit scroll_frame(manager* pManager);

        /// Destructor.
        ~scroll_frame() override;

        /// Updates this widget's logic.
        /** \param fDelta Time spent since last update
        *   \note Triggered callbacks could destroy the frame. If you need
        *         to use the frame again after calling this function, use
        *         the helper class alive_checker.
        */
        void update(float fDelta) override;

        /// Copies an uiobject's parameters into this scroll_frame (inheritance).
        /** \param pObj The uiobject to copy
        */
        void copy_from(uiobject* pObj) override;

        /// Returns 'true' if this scroll_frame can use a script.
        /** \param sScriptName The name of the script
        *   \note This method can be overriden if needed.
        */
        bool can_use_script(const std::string& sScriptName) const override;

        /// Calls a script.
        /** \param sScriptName The name of the script
        *   \param pEvent      Stores scripts arguments
        *   \note Triggered callbacks could destroy the frame. If you need
        *         to use the frame again after calling this function, use
        *         the helper class alive_checker.
        */
        void on_script(const std::string& sScriptName, event* pEvent = nullptr) override;

        /// Sets this scroll_frame's scroll child.
        /** \param pFrame The scroll child
        *   \note Creates the render target.
        */
        void set_scroll_child(utils::observable_sealed_ptr<frame> pFrame);

        /// Returns this scroll_frame's scroll child.
        /** \return This scroll_frame's scroll child
        */
        frame* get_scroll_child();

        /// Sets the horizontal offset of the scroll child.
        /** \param fHorizontalScroll The horizontal offset
        */
        void set_horizontal_scroll(float fHorizontalScroll);

        /// Returns the horizontal offset of the scroll child.
        /** \return The horizontal offset of the scroll child
        */
        float get_horizontal_scroll() const;

        /// Returns the maximum horizontal offset of the scroll child.
        /** \return The maximum horizontal offset of the scroll child
        */
        float get_horizontal_scroll_range() const;

        /// Sets the vertical offset of the scroll child.
        /** \param fVerticalScroll The vertical offset
        */
        void set_vertical_scroll(float fVerticalScroll);

        /// Returns the vertical offset of the scroll child.
        /** \return The vertical offset of the scroll child
        */
        float get_vertical_scroll() const;

        /// Returns the maximum vertical offset of the scroll child.
        /** \return The maximum vertical offset of the scroll child
        */
        float get_vertical_scroll_range() const;

        /// Checks if the provided coordinates are in the scroll_frame.
        /** \param fX The horizontal coordinate
        *   \param fY The vertical coordinate
        *   \return 'true' if the provided coordinates are in the scroll_frame
        *   \note The scroll_frame version of this function also checks if the
        *         mouse is over the scroll texture (which means this function
        *         ignores positive hit rect insets).
        *   \note For scroll children to receive input, the scroll_frame must be
        *         keyboard/mouse/wheel enabled.
        */
        bool is_in_frame(float fX, float fY) const override;

        /// Tells this scroll_frame it is being overed by the mouse.
        /** \param bMouseInFrame 'true' if the mouse is above this scroll_frame
        *   \param fX            The horizontal mouse coordinate
        *   \param fY            The vertical mouse coordinate
        */
        void notify_mouse_in_frame(bool bMouseInFrame, float fX, float fY) override;

        /// Tells this renderer that one of its widget requires redraw.
        void notify_strata_needs_redraw(frame_strata mStrata) const override;

        /// Tells this renderer that it should (or not) render another frame.
        /** \param pFrame    The frame to render
        *   \param bRendered 'true' if this renderer needs to render that new object
        */
        void notify_rendered_frame(frame* pFrame, bool bRendered) override;

        /// Returns the width of of this renderer's main render target (e.g., screen).
        /** \return The render target width
        */
        float get_target_width() const override;

        /// Returns the height of this renderer's main render target (e.g., screen).
        /** \return The render target height
        */
        float get_target_height() const override;

        /// Tells this widget that the global interface scaling factor has changed.
        void notify_scaling_factor_updated() override;

        /// Returns this widget's Lua glue.
        void create_glue() override;

        /// Registers this widget to the provided lua::state
        static void register_glue(lua::state& mLua);

        static constexpr const char* CLASS_NAME = "ScrollFrame";

    protected :

        void parse_all_blocks_before_children_(xml::block* pBlock) override;
        virtual void parse_scroll_child_block_(xml::block* pBlock);

        void update_scroll_range_();
        void update_scroll_child_input_();
        void rebuild_scroll_render_target_();
        void render_scroll_strata_list_();
        void update_borders_() const override;

        float fHorizontalScroll_ = 0;
        float fHorizontalScrollRange_ = 0;
        float fVerticalScroll_ = 0;
        float fVerticalScrollRange_ = 0;

        frame* pScrollChild_ = nullptr;

        mutable bool bRebuildScrollRenderTarget_ = false;
        mutable bool bRedrawScrollRenderTarget_ = false;
        mutable bool bUpdateScrollRange_ = false;
        std::shared_ptr<render_target> pScrollRenderTarget_;

        texture* pScrollTexture_ = nullptr;

        bool   bMouseInScrollTexture_ = false;
        frame* pHoveredScrollChild_ = nullptr;
    };

    /** \cond NOT_REMOVE_FROM_DOC
    */

    class lua_scroll_frame : public lua_frame
    {
    public :

        explicit lua_scroll_frame(lua_State* pLua);
        scroll_frame* get_object() { return static_cast<scroll_frame*>(pObject_); }

        // Glues
        int _get_horizontal_scroll(lua_State*);
        int _get_horizontal_scroll_range(lua_State*);
        int _get_scroll_child(lua_State*);
        int _get_vertical_scroll(lua_State*);
        int _get_vertical_scroll_range(lua_State*);
        int _set_horizontal_scroll(lua_State*);
        int _set_scroll_child(lua_State*);
        int _set_vertical_scroll(lua_State*);

        static const char className[];
        static const char* classList[];
        static lua::lunar_binding<lua_scroll_frame> methods[];
    };

    /** \endcond
    */
}
}

#endif
