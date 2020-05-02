#ifndef GUI_FOCUSFRAME_HPP
#define GUI_FOCUSFRAME_HPP

#include <lxgui/utils.hpp>
#include "lxgui/gui_frame.hpp"

namespace gui
{
    /// Abstract gui widget that can receive and loose focus
    /** A typical example is the edit_box widget.
    */
    class focus_frame : public frame
    {
    public :

        /// Constructor.
        explicit focus_frame(manager* pManager);

        /// Destructor.
        ~focus_frame();

        /// Copies an uiobject's parameters into this focus_frame (inheritance).
        /** \param pObj The uiobject to copy
        */
        void copy_from(uiobject* pObj) override;

        /// Enables automatic focus for this focus_frame.
        /** \param bEnable 'true' to enable auto focus
        *   \note When auto focus is enabled, this focus_frame will be focused
        *         immediately after it is shown.
        */
        void enable_auto_focus(bool bEnable);

        /// Checks if automatic focus is enabled.
        /** \return 'true' if automatic focus is enabled
        */
        bool is_auto_focus_enabled() const;

        /// Asks for focus for this focus_frame.
        /** \param bFocus 'true' to give to focus, 'false' to remove it
        *   \note This does not give focus immediately to the focus_frame.
        *         It is up to the manager class to actually give the focus
        *         through notify_focus().
        */
        void set_focus(bool bFocus);

        /// Notifies this focus_frame it has gained/lost focus.
        /** \param bFocus 'true' if the focus_frame has gained focus
        *   \note This function is called by the manager.
        */
        virtual void notify_focus(bool bFocus);

        /// Returns this widget's Lua glue.
        void create_glue() override;

        /// Parses data from an xml::block.
        /** \param pBlock The edit_box's xml::block
        */
        void parse_block(xml::block* pBlock) override;

        static constexpr const char* CLASS_NAME = "FocusFrame";

    protected :

        bool bFocus_;
        bool bAutoFocus_;
    };

    /** \cond NOT_REMOVE_FROM_DOC
    */

    class lua_focus_frame : public lua_frame
    {
    public :

        explicit lua_focus_frame(lua_State* pLua);

        // Glues
        int _clear_focus(lua_State*);
        int _is_auto_focus(lua_State*);
        int _set_auto_focus(lua_State*);
        int _set_focus(lua_State*);

        static const char  className[];
        static const char* classList[];
        static Lunar<lua_focus_frame>::RegType methods[];

    protected :

        focus_frame* pFocusFrameParent_;
    };

    /** \endcond
    */
}

#endif
