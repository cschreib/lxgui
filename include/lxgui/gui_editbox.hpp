#ifndef GUI_EDITBOX_HPP
#define GUI_EDITBOX_HPP

#include <lxgui/utils.hpp>
#include <lxgui/utils_string.hpp>
#include "lxgui/input_keys.hpp"
#include "lxgui/gui_focusframe.hpp"
#include "lxgui/gui_color.hpp"
#include "lxgui/gui_quad2.hpp"

#include <deque>

namespace lxgui {
namespace gui
{
    class font_string;
    class texture;

    /// A... periodic timer
    /** This timer is meant to tick periodicaly,
    *   so you can use it for any periodic event
    *   such as key repetition or a count down.
    */
    class periodic_timer
    {
    public :

        enum class start_type
        {
            /// The timer will start if you call Start()
            PAUSED,
            /// The timer starts immediatly after it is created
            NOW,
            /// The timer will start when you first call Ticks()
            FIRST_TICK
        };

        /// Default constructor
        /** \param dDuration The time interval between each tick
        *   \param mType     See TimerType
        *   \param bTicks    The timer ticks immediately
        */
        periodic_timer(double dDuration, start_type mType, bool bTicks);

        /// Returns the time elapsed since the last tick.
        /** \return The time elapsed since last tick
        */
        double get_elapsed();

        /// Returns the period of the periodic_timer.
        /** \return The period of the periodic_timer
        */
        double get_period() const;

        /// Cheks if this periodic_timer is paused.
        /** \return 'true' if this periodic_timer is paused
        */
        bool is_paused() const;

        /// Checks if the timer's period has been reached.
        /** \return 'true' if the period has been reached
        */
        bool ticks();

        /// Pauses the timer and resets it.
        void stop();

        /// Starts the timer but doesn't reset it.
        void start();

        /// Pauses the timer.
        void pause();

        /// Resets the timer but doesn't pause it.
        void zero();

        /// Updates this timer (adds time).
        /** \param dDelta The time elapsed since last update
        */
        void update(double dDelta);

    private :

        double dElapsed_ = 0.0;
        double dDuration_ = 0.0;
        bool   bPaused_ = true;
        bool   bFirstTick_ = true;

        start_type mType_ = start_type::PAUSED;
    };

    /// An editable text box.
    class edit_box : public focus_frame
    {
    public :

        /// Constructor.
        explicit edit_box(manager* pManager);

        /// Copies an uiobject's parameters into this edit_box (inheritance).
        /** \param pObj The uiobject to copy
        */
        void copy_from(uiobject* pObj) override;

        /// updates this widget's logic.
        void update(float fDelta) override;

        /// Calls the on_event script.
        /** \param mEvent The Event that occured
        */
        void on_event(const event& mEvent) override;

        /// Calls a script.
        /** \param sScriptName The name of the script
        *   \param pEvent      Stores scripts arguments
        */
        void on(const std::string& sScriptName, event* pEvent = nullptr) override;

        /// Returns 'true' if this edit_box can use a script.
        /** \param sScriptName The name of the script
        *   \note This method can be overriden if needed.
        */
        bool can_use_script(const std::string& sScriptName) const override;

        /// Sets if this edit_box can receive keyboard input.
        /** \param bIsKeyboardEnabled 'true' to enable
        */
        void enable_keyboard(bool bIsKeyboardEnabled) override;

        /// Sets the content of this edit_box.
        /** \param sText The content of this edit_box
        */
        void set_text(const std::string& sText);

        /// Returns the content of this edit_box.
        /** \return The content of this edit_box
        */
        const std::string& get_text() const;

        /// Selects a portion of the content.
        /** \param uiStart      The first character to select
        *   \param uiEnd        The last character to select
        *   \param bForceUpdate 'true' to bypass all redundancy checks
        *   \note Will select (uiEnd - uiStart) characters
        */
        void highlight_text(uint uiStart = 0u, uint uiEnd = uint(-1), bool bForceUpdate = false);

        /// Deselects the selected text, if any.
        void unlight_text();

        /// Sets the color of the highlight quad.
        /** \param mColor The color
        */
        void set_highlight_color(const color& mColor);

        /// Inserts some text after the cursor.
        /** \param sText The text to insert
        */
        void insert_after_cursor(const std::string& sText);

        /// Sets the maximum number of letters to allow in this edit_box.
        /** \param uiMaxLetters The max number of letters
        */
        void set_max_letters(uint uiMaxLetters);

        /// Returns the maximum number of letters to allow in this edit_box.
        /** \return the maximum number of letters to allow in this edit_box
        */
        uint get_max_letters() const;

        /// Returns the number of letters in the content.
        /** \return The number of letters in the content
        */
        uint get_num_letters() const;

        /// Sets the carret's blink speed.
        /** \param dBlinkSpeed The number of seconds to wait between each blink
        */
        void set_blink_speed(double dBlinkSpeed);

        /// Returns the carret's blink speed.
        /** \return the carret's blink speed (time in seconds between each blink)
        */
        double get_blink_speed() const;

        /// Makes this edit_box allow numeric characters only.
        /** \param bNumericOnly 'true' to only allow numeric characters
        */
        void set_numeric_only(bool bNumericOnly);

        /// Makes this edit_box allow positive numbers only.
        /** \param bPositiveOnly 'true' to only allow positive numbers
        *   \note Only workds if set_numeric_only(true) has been called.
        */
        void set_positive_only(bool bPositiveOnly);

        /// Makes this edit_box allow integer numbers only.
        /** \param bIntegerOnly 'true' to only allow integer numbers
        *   \note Only workds if set_numeric_only(true) has been called.
        */
        void set_integer_only(bool bIntegerOnly);

        /// Checks if this edit_box allows numeric characters only.
        /** \return 'true' if this edit_box allows numeric characters only
        */
        bool is_numeric_only() const;

        /// Checks if this edit_box allows positive numbers only.
        /** \return 'true' if this edit_box allows positive numbers only
        */
        bool is_positive_only() const;

        /// Checks if this edit_box allows integer numbers only.
        /** \return 'true' if this edit_box allows integer numbers only
        */
        bool is_integer_only() const;

        /// Enables password mode.
        /** \param bEnable 'true' to enable password mode
        *   \note In password mode, the content of the edit_box is replaced
        *         by stars (*).
        */
        void enable_password_mode(bool bEnable);

        /// Checks if this edit_box is in password mode.
        /** \return 'true' if this edit_box is in password mode
        */
        bool is_password_mode_enabled() const;

        /// Allows this edit_box to have several lines in it.
        /** \param bMultiLine 'true' to allow several lines in this edit_box
        *   \note The behavior of a "multi line" edit_box is very different from
        *         a single line one.<br>
        *         History lines are only available to single line edit_boxes.<br>
        *         Scrolling in a single line edit_box is done horizontally, while
        *         it is only done vertically in a multi line one.
        */
        void set_multi_line(bool bMultiLine);

        /// Checks if this edit_box can have several lines in it.
        /** \return 'true' if this edit_box can have several lines in it
        */
        bool is_multi_line() const;

        /// Sets the maximum number of history lines this edit_box can keep.
        /** \param uiMaxHistoryLines The max number of history lines
        */
        void set_max_history_lines(uint uiMaxHistoryLines);

        /// Returns the maximum number of history lines this edit_box can keep.
        /** \return The maximum number of history lines this edit_box can keep
        */
        uint get_max_history_lines() const;

        /// Adds a new history line to the history line list.
        /** \param sHistoryLine The content of this history line
        *   \note This option is only available to single line edit_boxes.
        */
        void add_history_line(const std::string& sHistoryLine);

        /// Returns the history line list.
        /** \return The history line list
        *   \note This list will always be empty for multi line edit_boxes.
        */
        const std::vector<std::string>& get_history_lines() const;

        /// Sets whether keyboard arrows move the carret or not.
        /** \param bArrowsIgnored 'true' to ignore arrow keys
        */
        void set_arrows_ignored(bool bArrowsIgnored);

        /// Sets the insets used to render the content text.
        /** \param iLeft   The left inset
        *   \param iRight  The right inset
        *   \param iTop    The top inset
        *   \param iBottom The bottom inset
        *   \note Positive insets will reduce the text area, while
        *         negative ones will enlarge it
        */
        void set_text_insets(int iLeft, int iRight, int iTop, int iBottom);

        /// Sets the insets used to render the content text.
        /** \param lInsets (left, right, top, bottom)
        *   \note Positive insets will reduce the text area, while
        *         negative ones will enlarge it
        */
        void set_text_insets(const quad2i& lInsets);

        /// Returns the text insets.
        /** \return The text insets
        */
        const quad2i& get_text_insets() const;

        /// Returns the font_string used to render the content.
        /** \return The font_string used to render the content
        */
        font_string* get_font_string();

        /// Sets the font_string to use to render the content.
        /** \param pFont The font_string to use to render the content
        */
        void set_font_string(font_string* pFont);

        /// Notifies this edit_box it has gained/lost focus.
        /** \param bFocus 'true' if the edit_box has gained focus
        *   \note This function is called by manager.
        */
        void notify_focus(bool bFocus) override;

        /// Returns this widget's Lua glue.
        void create_glue() override;

        /// Parses data from an xml::block.
        /** \param pBlock The edit_box's xml::block
        */
        void parse_block(xml::block* pBlock) override;

        /// Registers this widget to the provided lua::state
        static void register_glue(lua::state* pLua);

        static constexpr const char* CLASS_NAME = "EditBox";

    protected :

        void notify_invisible_(bool bTriggerEvents = true) override;

        void parse_font_string_block_(xml::block* pBlock);
        void parse_text_insets_block_(xml::block* pBlock);

        font_string* create_font_string_();
        void         create_highlight_();
        void         create_carret_();

        void check_text_();
        void update_displayed_text_();
        void update_font_string_();
        void update_carret_position_();

        bool add_char_(char32_t sChar);
        bool remove_char_();
        uint get_letter_id_at_(int iX, int iY);
        bool move_carret_at_(int iX, int iY);
        bool move_carret_horizontally_(bool bForward = true);
        bool move_carret_vertically_(bool bDown = true);

        void process_key_(input::key uiKey);

        std::string              sText_;
        utils::ustring           sUnicodeText_;
        utils::ustring           sDisplayedText_;
        utils::ustring::iterator iterCarretPos_;

        uint uiDisplayPos_ = 0;
        uint uiNumLetters_ = 0;
        uint uiMaxLetters_ = uint(-1);
        bool bNumericOnly_ = false;
        bool bPositiveOnly_ = false;
        bool bIntegerOnly_ = false;
        bool bPasswordMode_ = false;
        bool bMultiLine_ = false;
        bool bArrowsIgnored_ = false;

        std::string sComboKey_;

        texture* pHighlight_ = nullptr;
        color    mHighlightColor_ = color(1.0f, 1.0f, 1.0f, 0.35f);
        uint     uiSelectionStartPos_ = 0u;
        uint     uiSelectionEndPos_ = 0u;
        bool     bSelectedText_ = false;

        texture*       pCarret_ = nullptr;
        double         dBlinkSpeed_ = 0.5;
        periodic_timer mCarretTimer_;

        std::vector<std::string> lHistoryLineList_;
        uint                     uiMaxHistoryLines_ = uint(-1);

        font_string* pFontString_ = nullptr;
        quad2i       lTextInsets_ = quad2i::ZERO;

        input::key     mLastKeyPressed_;
        double         dKeyRepeatSpeed_ = 0.03;
        periodic_timer mKeyRepeatTimer_;
    };

    /** \cond NOT_REMOVE_FROM_DOC
    */

    class lua_edit_box : public lua_focus_frame
    {
    public :

        explicit lua_edit_box(lua_State* pLua);

        // Glues
        int _add_history_line(lua_State*);
        int _get_blink_speed(lua_State*);
        int _get_history_lines(lua_State*);
        int _get_max_letters(lua_State*);
        int _get_num_letters(lua_State*);
        int _get_number(lua_State*);
        int _get_text(lua_State*);
        int _get_text_insets(lua_State*);
        int _highlight_text(lua_State*);
        int _insert(lua_State*);
        int _is_multi_line(lua_State*);
        int _is_numeric(lua_State*);
        int _is_password(lua_State*);
        int _set_blink_speed(lua_State*);
        int _set_max_history_lines(lua_State*);
        int _set_max_letters(lua_State*);
        int _set_multi_line(lua_State*);
        int _set_number(lua_State*);
        int _set_numeric(lua_State*);
        int _set_password(lua_State*);
        int _set_text(lua_State*);
        int _set_text_insets(lua_State*);

        static const char  className[];
        static const char* classList[];
        static lua::Lunar<lua_edit_box>::RegType methods[];

    protected :

        edit_box* pEditBoxParent_;
    };

    /** \endcond
    */
}
}

#endif
