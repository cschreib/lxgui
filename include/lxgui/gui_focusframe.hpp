#ifndef LXGUI_GUI_FOCUSFRAME_HPP
#define LXGUI_GUI_FOCUSFRAME_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_frame.hpp"

namespace lxgui {
namespace gui
{
    /// A #frame that can receive and loose focus
    /** A typical usage example is the #lxgui::gui::edit_box class.
    */
    class focus_frame : public frame
    {
        using base = frame;

    public :

        /// Constructor.
        explicit focus_frame(utils::control_block& mBlock, manager& mManager);

        /// Destructor.
        ~focus_frame();

        /// Copies an uiobject's parameters into this focus_frame (inheritance).
        /** \param mObj The uiobject to copy
        */
        void copy_from(const uiobject& mObj) override;

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

        /// Check if this frame currently has focus.
        /** \return 'true' if the frame has focus, 'false' otherwise
        */
        bool has_focus() const;

        /// Notifies this focus_frame it has gained/lost focus.
        /** \param bFocus 'true' if the focus_frame has gained focus
        *   \note This function is called by the manager.
        */
        virtual void notify_focus(bool bFocus);

        /// Returns this widget's Lua glue.
        void create_glue() override;

        /// Notifies this widget that it is now visible on screen.
        /** \note Automatically called by show()/hide().
        */
        void notify_visible() override;

        /// Notifies this widget that it is no longer visible on screen.
        /** \note Automatically called by show()/hide().
        */
        void notify_invisible() override;

        /// Registers this widget class to the provided Lua state
        static void register_on_lua(sol::state& mLua);

        static constexpr const char* CLASS_NAME = "FocusFrame";

    protected :

        void parse_attributes_(const layout_node& mNode) override;

        bool bFocus_ = false;
        bool bAutoFocus_ = false;
    };
}
}

#endif
