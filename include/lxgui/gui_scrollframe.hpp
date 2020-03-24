#ifndef GUI_SCROLLFRAME_HPP
#define GUI_SCROLLFRAME_HPP

#include <lxgui/utils.hpp>
#include "lxgui/gui_frame.hpp"

namespace gui
{
    class texture;

    /// A frame with scrollable content
    /** This widget has a special child frame, the scroll child.
    *   The scroll child is rendered on a render_target, which is
    *   then rendered on the screen.
    */
    class scroll_frame : public frame
    {
    public :

        /// Constructor.
        explicit scroll_frame(manager* pManager);

        /// Destructor.
        virtual ~scroll_frame();

        /// updates this widget's logic.
        virtual void update(float fDelta);

        /// Copies an uiobject's parameters into this scroll_frame (inheritance).
        /** \param pObj The uiobject to copy
        */
        virtual void copy_from(uiobject* pObj);

        /// Returns 'true' if this scroll_frame can use a script.
        /** \param sScriptName The name of the script
        *   \note This method can be overriden if needed.
        */
        virtual bool can_use_script(const std::string& sScriptName) const;

        /// Calls a script.
        /** \param sScriptName The name of the script
        *   \param pEvent      Stores scripts arguments
        */
        virtual void on(const std::string& sScriptName, event* pEvent = nullptr);

        /// Sets this scroll_frame's scroll child.
        /** \param pFrame The scroll child
        *   \note Creates the render_target and the associated sprite.
        */
        void set_scroll_child(frame* pFrame);

        /// Returns this scroll_frame's scroll child.
        /** \return This scroll_frame's scroll child
        */
        frame* get_scroll_child();

        /// Sets the horizontal offset of the scroll child.
        /** \param iHorizontalScroll The horizontal offset
        */
        void set_horizontal_scroll(int iHorizontalScroll);

        /// Returns the horizontal offset of the scroll child.
        /** \return The horizontal offset of the scroll child
        */
        int get_horizontal_scroll() const;

        /// Returns the maximum horizontal offset of the scroll child.
        /** \return The maximum horizontal offset of the scroll child
        */
        int get_horizontal_scroll_range() const;

        /// Sets the vertical offset of the scroll child.
        /** \param iVerticalScroll The vertical offset
        */
        void set_vertical_scroll(int iVerticalScroll);

        /// Returns the vertical offset of the scroll child.
        /** \return The vertical offset of the scroll child
        */
        int get_vertical_scroll() const;

        /// Returns the maximum vertical offset of the scroll child.
        /** \return The maximum vertical offset of the scroll child
        */
        int get_vertical_scroll_range() const;

        /// Checks if the provided coordinates are in the scroll_frame.
        /** \param iX           The horizontal coordinate
        *   \param iY           The vertical coordinate
        *   \return 'true' if the provided coordinates are in the scroll_frame
        *   \note The scroll_frame version of this function also checks if the
        *         mouse is over the scroll texture (which means this function
        *         ignores positive hit rect insets).
        *   \note For scroll children to receive input, the scroll_frame must be
        *         keyboard/mouse/wheel enabled.
        */
        virtual bool is_in_frame(int iX, int iY) const;

        /// Tells this scroll_frame it is being overed by the mouse.
        /** \param bMouseInFrame 'true' if the mouse is above this scroll_frame
        *   \param iX            The horizontal mouse coordinate
        *   \param iY            The vertical mouse coordinate
        */
        virtual void notify_mouse_in_frame(bool bMouseInFrame, int iX, int iY);

        /// Tells this widget that a manually rendered widget requires redraw.
        virtual void fire_redraw() const;

        /// Tells this scroll_frame that at least one of its children has modified its strata or level.
        /** \param pChild The child that has changed its strata (can also be a child of this child)
        *   \note If pChild is the scroll child, it only rebuilds its internal strata list.
        *   \note If this scroll_frame has no parent, it calls manager::fire_build_strata_list(). Else it
        *         notifies its parent.
        */
        virtual void notify_child_strata_changed(frame* pChild);

        /// Returns this widget's Lua glue.
        virtual void create_glue();

        /// Parses data from an xml::block.
        /** \param pBlock The scroll_frame's xml::block
        */
        virtual void parse_block(xml::block* pBlock);

        /// Registers this widget to the provided lua::state
        static void register_glue(lua::state* pLua);

        static constexpr const char* CLASS_NAME = "ScrollFrame";

    protected :

        virtual void parse_scroll_child_block_(xml::block* pBlock);

        virtual void notify_manually_rendered_object_(uiobject* pObject, bool bManuallyRendered);

        void add_to_scroll_child_list_(frame* pChild);
        void remove_from_scroll_child_list_(frame* pChild);

        void update_scroll_range_();
        void update_scroll_child_input_();
        void rebuild_scroll_render_target_();
        void rebuild_scroll_strata_list_();
        void render_scroll_strata_list_();

        int iHorizontalScroll_;
        int iHorizontalScrollRange_;
        int iVerticalScroll_;
        int iVerticalScrollRange_;

        frame* pScrollChild_;

        mutable bool bRebuildScrollRenderTarget_;
        mutable bool bRedrawScrollRenderTarget_;
        mutable bool bUpdateScrollRange_;
        utils::refptr<render_target> pScrollRenderTarget_;

        texture* pScrollTexture_;

        mutable bool                   bRebuildScrollStrataList_;
        std::map<uint, frame*>         lScrollChildList_;
        std::map<frame_strata, strata> lScrollStrataList_;

        bool   bMouseInScrollTexture_;
        frame* pOveredScrollChild_;
    };

    /** \cond NOT_REMOVE_FROM_DOC
    */

    class lua_scroll_frame : public lua_frame
    {
    public :

        lua_scroll_frame(lua_State* pLua);

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
        static Lunar<lua_scroll_frame>::RegType methods[];

    protected :

        scroll_frame* pScrollFrameParent_;
    };

    /** \endcond
    */
}

#endif
