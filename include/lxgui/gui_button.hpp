#ifndef LXGUI_GUI_BUTTON_HPP
#define LXGUI_GUI_BUTTON_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include <lxgui/utils_string.hpp>
#include "lxgui/gui_frame.hpp"

#include <string>

namespace lxgui {
namespace gui
{
    class texture;
    class font_string;

    /// A #frame with a button that can be clicked.
    /** This class can handle three different states: "normal", "pushed"
    *   and "disabled". You can provide a different texture for each of
    *   these states, and two different fontstrings for "normal" and
    *   "disabled".
    *
    *   In addition, you can provide another texture/fontstring for the
    *   "highlight" state (when the mouse is over the button widget).
    *
    *   Note that there is no fontstring for the "pushed" state: in this
    *   case, the "normal" font is rendered with a slight offset that you
    *   are free to define.
    *
    *   Note that a button has frame::enable_mouse set to `true` by
    *   default.
    *
    *   __Events.__ Hard-coded events available to all buttons, in
    *   addition to those from frame:
    *
    *   - `OnClick`: Triggered when the button is clicked, either when
    *   button::click is called, or after the mouse is released after a
    *   click over the button.
    *   - `OnDoubleClick`: Triggered when the button is double-clicked.
    *   - `OnEnable`: Triggered by button::enable.
    *   - `OnDisable`: Triggered by button::disable.
    */
    class button : public frame
    {
    public :

        enum class state
        {
            UP,
            DOWN,
            DISABLED
        };

        /// Constructor.
        explicit button(manager& mManager);

        /// Prints all relevant information about this widget in a string.
        /** \param sTab The offset to give to all lines
        *   \return All relevant information about this widget
        */
        std::string serialize(const std::string& sTab) const override;

        /// Creates the associated Lua glue.
        void create_glue() override;

        /// Returns 'true' if this Button can use a script.
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

        /// Calls the on_event script.
        /** \param mEvent The Event that occured
        *   \note Triggered callbacks could destroy the frame. If you need
        *         to use the frame again after calling this function, use
        *         the helper class alive_checker.
        */
        void on_event(const event& mEvent) override;

        /// Copies an uiobject's parameters into this Button (inheritance).
        /** \param pObj The uiobject to copy
        */
        void copy_from(const uiobject& mObj) override;

        /// Sets this button's text.
        /** \param sText The new text
        */
        virtual void set_text(const utils::ustring& sText);

        /// Returns this button's text.
        /** \return This button's text
        */
        const utils::ustring& get_text() const;

        /// Returns this button's normal texture.
        /** \return This button's normal texture
        */
        const utils::observer_ptr<texture>& get_normal_texture() { return pNormalTexture_; }

        /// Returns this button's normal texture.
        /** \return This button's normal texture
        */
        utils::observer_ptr<const texture> get_normal_texture() const { return pNormalTexture_; }

        /// Returns this button's pushed texture.
        /** \return This button's pushed texture
        */
        const utils::observer_ptr<texture>& get_pushed_texture() { return pPushedTexture_; }

        /// Returns this button's pushed texture.
        /** \return This button's pushed texture
        */
        utils::observer_ptr<const texture> get_pushed_texture() const { return pPushedTexture_; }

        /// Returns this button's disabled texture.
        /** \return This button's disabled texture
        */
        const utils::observer_ptr<texture>& get_disabled_texture() { return pDisabledTexture_; }

        /// Returns this button's disabled texture.
        /** \return This button's disabled texture
        */
        utils::observer_ptr<const texture> get_disabled_texture() const { return pDisabledTexture_; }

        /// Returns this button's highlight texture.
        /** \return This button's highlight texture
        */
        const utils::observer_ptr<texture>& get_highlight_texture() { return pHighlightTexture_; }

        /// Returns this button's highlight texture.
        /** \return This button's highlight texture
        */
        utils::observer_ptr<const texture> get_highlight_texture() const { return pHighlightTexture_; }

        /// Returns this button's normal text.
        /** \return This button's normal text
        */
        const utils::observer_ptr<font_string>& get_normal_text() { return pNormalText_; }

        /// Returns this button's normal text.
        /** \return This button's normal text
        */
        utils::observer_ptr<const font_string> get_normal_text() const { return pNormalText_; }

        /// Returns this button's highlight text.
        /** \return This button's highlight text
        */
        const utils::observer_ptr<font_string>& get_highlight_text() { return pHighlightText_; }

        /// Returns this button's highlight text.
        /** \return This button's highlight text
        */
        utils::observer_ptr<const font_string> get_highlight_text() const { return pHighlightText_; }

        /// Returns this button's disabled text.
        /** \return This button's disabled text
        */
        const utils::observer_ptr<font_string>& get_disabled_text() { return pDisabledText_; }

        /// Returns this button's disabled text.
        /** \return This button's disabled text
        */
        utils::observer_ptr<const font_string> get_disabled_text() const { return pDisabledText_; }

        /// Returns the currently displayed text object.
        /** \return The currently displayed text object
        */
        const utils::observer_ptr<font_string>& get_current_font_string() { return pCurrentFontString_; }

        /// Returns the currently displayed text object.
        /** \return The currently displayed text object
        */
        utils::observer_ptr<const font_string> get_current_font_string() const { return pCurrentFontString_; }

        /// Sets this button's normal texture.
        /** \param pTexture The new texture
        */
        void set_normal_texture(utils::observer_ptr<texture> pTexture);

        /// Sets this button's pushed texture.
        /** \param pTexture The new texture
        */
        void set_pushed_texture(utils::observer_ptr<texture> pTexture);

        /// Sets this button's disabled texture.
        /** \param pTexture The new texture
        */
        void set_disabled_texture(utils::observer_ptr<texture> pTexture);

        /// Sets this button's highlight texture.
        /** \param pTexture The new texture
        */
        void set_highlight_texture(utils::observer_ptr<texture> pTexture);

        /// Sets this button's normal text.
        /** \param pFont The new text object
        */
        void set_normal_text(utils::observer_ptr<font_string> pFont);

        /// Sets this button's highlight text.
        /** \param pFont The new text object
        */
        void set_highlight_text(utils::observer_ptr<font_string> pFont);

        /// Sets this button's disabled text.
        /** \param pFont The new text object
        */
        void set_disabled_text(utils::observer_ptr<font_string> pFont);

        /// Disables this Button.
        /** \note A disabled button doesn't receive any input.
        */
        virtual void disable();

        /// Enables this Button.
        virtual void enable();

        /// Checks if this Button is enabled.
        /** \return 'true' if this Button is enabled
        */
        bool is_enabled() const;

        /// Pushed this Button.
        /** \note This function only has a visual impact :
        *         the OnClick() handler is not called.
        */
        virtual void push();

        /// Releases this Button.
        /** \note This function only has a visual impact :
        *         the OnClick() handler is not called.
        */
        virtual void release();

        /// Highlights this Button.
        /** \note The Button will be highlighted even if the
        *         mouse is not over it. It will stop when the
        *         mouse leaves it.
        */
        virtual void highlight();

        /// Unlights this Button.
        /** \note The Button will be unlighted even if the
        *         mouse is over it. It will highlight again
        *         when the mouse leaves then enters its region.
        */
        virtual void unlight();

        /// Returns this button's state.
        /** \return This button's state (see ButtonState)
        */
        state get_button_state() const;

        /// Locks this button's highlighting.
        /** \note The button will always be highlighted
        *         until you call unlock_highlight().
        */
        void lock_highlight();

        /// Unlocks this button's highlighting.
        void unlock_highlight();

        /// Sets this button's pushed text offset.
        /** \param lOffset The pushed text offset
        */
        void set_pushed_text_offset(const vector2f& lOffset);

        /// Returns this button's pushed text offset.
        /** \return This button's pushed text offset
        */
        const vector2f& get_pushed_text_offset() const;

        /// Registers this widget class to the provided Lua state
        static void register_on_lua(sol::state& mLua);

        static constexpr const char* CLASS_NAME = "Button";

    protected :

        void parse_attributes_(xml::block* pBlock) override;
        void parse_all_blocks_before_children_(xml::block* pBlock) override;

        utils::owner_ptr<texture>     create_normal_texture_();
        utils::owner_ptr<texture>     create_pushed_texture_();
        utils::owner_ptr<texture>     create_disabled_texture_();
        utils::owner_ptr<texture>     create_highlight_texture_();
        utils::owner_ptr<font_string> create_normal_text_();
        utils::owner_ptr<font_string> create_highlight_text_();
        utils::owner_ptr<font_string> create_disabled_text_();

        state     mState_ = state::UP;
        bool      bHighlighted_ = false;
        bool      bLockHighlight_ = false;

        utils::ustring sText_;

        utils::observer_ptr<texture> pNormalTexture_ = nullptr;
        utils::observer_ptr<texture> pPushedTexture_ = nullptr;
        utils::observer_ptr<texture> pDisabledTexture_ = nullptr;
        utils::observer_ptr<texture> pHighlightTexture_ = nullptr;

        utils::observer_ptr<font_string> pNormalText_ = nullptr;
        utils::observer_ptr<font_string> pHighlightText_ = nullptr;
        utils::observer_ptr<font_string> pDisabledText_ = nullptr;
        utils::observer_ptr<font_string> pCurrentFontString_ = nullptr;

        vector2f mPushedTextOffset_ = vector2f::ZERO;
    };
}
}

#endif
