#ifndef LXGUI_GUI_EDITBOX_HPP
#define LXGUI_GUI_EDITBOX_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include <lxgui/utils_string.hpp>
#include "lxgui/input_keys.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_color.hpp"
#include "lxgui/gui_bounds2.hpp"

namespace lxgui {
namespace gui
{
    class font_string;
    class texture;

    /// A repeating timer
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
        double get_elapsed() const;

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

    /// A #frame with an editable text box.
    /** This frame lets the user input arbitrary text into a box,
    *   which can be read and used by the rest of the interface.
    *   The text box can be either single-line or multi-line.
    *   Text can be selected by holding the Shift key, and natural
    *   navigation is available with the Left, Right, Up, Down, Home,
    *   End, Page Up, and Page Down keys. Copy and paste operations
    *   are also supported. The edit box can also remember the history
    *   of previously entered values or commands, which can be brought
    *   back at will. The characters entered as handled by the
    *   operating system, hence this class will use whatever keyboard
    *   layout is currently in use. Finally, the edit box can be
    *   configured to only accept numeric values (of either sign, or
    *   positive only), and to hide the input characters to simulate a
    *   password box (no encryption or other safety measure is used).
    *
    *   Note that an edit_box has frame::enable_mouse set to `true`
    *   and frame::register_for_drag set to `"LeftButton"` by default.
    *
    *   __Events.__ Hard-coded events available to all edit_boxes,
    *   in addition to those from #frame:
    *
    *   - `OnChar`: Triggered whenever a new character is added to the
    *   edit box. Will always be preceeded by `OnTextChanged`.
    *   - `OnCursorChanged`: Triggered whenever the position of the edit
    *   cursor is changed (not yet implemented).
    *   - `OnEnterPressed`: Triggered when the `Enter` (or `Return`) key
    *   is pressed while the edit box is focussed. This captures both
    *   the main keyboard key and the smaller one on the numpad.
    *   - `OnEscapePressed`: Triggered when the `Escape` key is pressed
    *   while the edit box is focussed.
    *   - `OnSpacePressed`: Triggered when the `Space` key is pressed
    *   while the edit box is focussed.
    *   - `OnTabPressed`: Triggered when the `Tab` key is pressed
    *   while the edit box is focussed.
    *   - `OnUpPressed`: Triggered when the `Up` key is pressed
    *   while the edit box is focussed.
    *   - `OnDownPressed`: Triggered when the `Down` key is pressed
    *   while the edit box is focussed.
    *   - `OnTextChanged`: Triggered whenever the text contained in the
    *   edit box changes (character added or deleted, text set or pasted,
    *   etc.).
    *   - `OnTextSet`: Triggered by edit_box::set_text. Will always be
    *   followed by `OnTextChanged`.
    */
    class edit_box : public frame
    {
        using base = frame;

    public :

        /// Constructor.
        explicit edit_box(utils::control_block& mBlock, manager& mManager);

        /// Copies an uiobject's parameters into this edit_box (inheritance).
        /** \param mObj The uiobject to copy
        */
        void copy_from(const uiobject& mObj) override;

        /// Updates this widget's logic.
        /** \param fDelta Time spent since last update
        *   \note Triggered callbacks could destroy the frame. If you need
        *         to use the frame again after calling this function, use
        *         the helper class alive_checker.
        */
        void update(float fDelta) override;

        /// Calls a script.
        /** \param sScriptName The name of the script
        *   \param mData       Stores scripts arguments
        *   \note Triggered callbacks could destroy the frame. If you need
        *         to use the frame again after calling this function, use
        *         the helper class alive_checker.
        */
        void on_script(const std::string& sScriptName, const event_data& mData = event_data{}) override;

        /// Returns 'true' if this edit_box can use a script.
        /** \param sScriptName The name of the script
        *   \note This method can be overriden if needed.
        */
        bool can_use_script(const std::string& sScriptName) const override;

        /// Sets the content of this edit_box.
        /** \param sText The content of this edit_box
        */
        void set_text(const utils::ustring& sText);

        /// Returns the content of this edit_box.
        /** \return The content of this edit_box
        */
        const utils::ustring& get_text() const;

        /// Selects a portion of the content.
        /** \param uiStart      The first character to select
        *   \param uiEnd        The last character to select
        *   \param bForceUpdate 'true' to bypass all redundancy checks
        *   \note Will select (uiEnd - uiStart) characters
        */
        void highlight_text(std::size_t uiStart = 0u,
            std::size_t uiEnd = std::numeric_limits<std::size_t>::max(), bool bForceUpdate = false);

        /// Deselects the selected text, if any.
        void unlight_text();

        /// Sets the color of the highlight quad.
        /** \param mColor The color
        */
        void set_highlight_color(const color& mColor);

        /// Inserts some text after the cursor.
        /** \param sText The text to insert
        */
        void insert_after_cursor(const utils::ustring& sText);

        /// Returns the current position of the cursor.
        /** \return The position of the cursor (0: before first character,
                    get_num-letters(): after last character).
        */
        std::size_t get_cursor_position() const;

        /// Moves the cursor to a chosen position.
        /** \param uiPos The new cursor position (0: before first character,
                         get_num-letters(): after last character).
        */
        void set_cursor_position(std::size_t uiPos);

        /// Sets the maximum number of letters to allow in this edit_box.
        /** \param uiMaxLetters The max number of letters
        */
        void set_max_letters(std::size_t uiMaxLetters);

        /// Returns the maximum number of letters to allow in this edit_box.
        /** \return the maximum number of letters to allow in this edit_box
        */
        std::size_t get_max_letters() const;

        /// Returns the number of letters in the content.
        /** \return The number of letters in the content
        */
        std::size_t get_num_letters() const;

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
        void set_max_history_lines(std::size_t uiMaxHistoryLines);

        /// Returns the maximum number of history lines this edit_box can keep.
        /** \return The maximum number of history lines this edit_box can keep
        */
        std::size_t get_max_history_lines() const;

        /// Adds a new history line to the history line list.
        /** \param sHistoryLine The content of this history line
        *   \note This option is only available to single line edit_boxes.
        */
        void add_history_line(const utils::ustring& sHistoryLine);

        /// Returns the history line list.
        /** \return The history line list
        *   \note This list will always be empty for multi line edit_boxes.
        */
        const std::vector<utils::ustring>& get_history_lines() const;

        /// Clears the history line list.
        void clear_history();

        /// Sets whether keyboard arrows move the carret or not.
        /** \param bArrowsIgnored 'true' to ignore arrow keys
        */
        void set_arrows_ignored(bool bArrowsIgnored);

        /// Sets the insets used to render the content text.
        /** \param lInsets (left, right, top, bottom)
        *   \note Positive insets will reduce the text area, while
        *         negative ones will enlarge it
        */
        void set_text_insets(const bounds2f& lInsets);

        /// Returns the text insets.
        /** \return The text insets
        */
        const bounds2f& get_text_insets() const;

        /// Returns the font_string used to render the content.
        /** \return The font_string used to render the content
        */
        const utils::observer_ptr<font_string>& get_font_string() { return pFontString_; }

        /// Returns the font_string used to render the content.
        /** \return The font_string used to render the content
        */
        utils::observer_ptr<const font_string> get_font_string() const { return pFontString_; }

        /// Sets the font_string to use to render the content.
        /** \param pFont The font_string to use to render the content
        */
        void set_font_string(utils::observer_ptr<font_string> pFont);

        /// Sets the font (file and size) to render the content.
        /** \param sFontName The file path to the .ttf file
        *   \param fHeight   The font height
        */
        void set_font(const std::string& sFontName, float fHeight);

        /// Notifies this frame that it has received or lost focus.
        /** \param bFocus 'true' if focus is received, 'false' if lost
        */
        void notify_focus(bool bFocus) override;

        /// Tells this widget that the global interface scaling factor has changed.
        void notify_scaling_factor_updated() override;

        /// Returns this widget's Lua glue.
        void create_glue() override;

        /// Registers this widget class to the provided Lua state
        static void register_on_lua(sol::state& mLua);

        static constexpr const char* CLASS_NAME = "EditBox";

    protected :

        void parse_attributes_(const layout_node& mNode) override;
        void parse_all_nodes_before_children_(const layout_node& mNode) override;
        void parse_font_string_node_(const layout_node& mNode);
        void parse_text_insets_node_(const layout_node& mNode);

        void create_font_string_();
        void create_highlight_();
        void create_carret_();

        void check_text_();
        void update_displayed_text_();
        void update_font_string_();
        void update_carret_position_();

        bool add_char_(char32_t sChar);
        bool remove_char_();
        std::size_t get_letter_id_at_(const vector2f& mPosition);
        bool move_carret_at_(const vector2f& mPosition);
        bool move_carret_horizontally_(bool bForward = true);
        bool move_carret_vertically_(bool bDown = true);

        void process_key_(input::key uiKey);

        utils::ustring           sUnicodeText_;
        utils::ustring           sDisplayedText_;
        utils::ustring::iterator iterCarretPos_;
        utils::ustring::iterator iterCarretPosOld_;

        std::size_t uiDisplayPos_ = 0;
        std::size_t uiNumLetters_ = 0;
        std::size_t uiMaxLetters_ = std::numeric_limits<std::size_t>::max();
        bool bNumericOnly_ = false;
        bool bPositiveOnly_ = false;
        bool bIntegerOnly_ = false;
        bool bPasswordMode_ = false;
        bool bMultiLine_ = false;
        bool bArrowsIgnored_ = false;

        std::string sComboKey_;

        utils::observer_ptr<texture> pHighlight_ = nullptr;
        color                        mHighlightColor_ = color(1.0f, 1.0f, 1.0f, 0.5f);
        std::size_t                  uiSelectionStartPos_ = 0u;
        std::size_t                  uiSelectionEndPos_ = 0u;
        bool                         bSelectedText_ = false;

        utils::observer_ptr<texture> pCarret_ = nullptr;
        double                       dBlinkSpeed_ = 0.5;
        periodic_timer               mCarretTimer_;

        std::vector<utils::ustring> lHistoryLineList_;
        std::size_t                 uiMaxHistoryLines_ = std::numeric_limits<std::size_t>::max();
        std::size_t                 uiCurrentHistoryLine_ = std::numeric_limits<std::size_t>::max();

        utils::observer_ptr<font_string> pFontString_ = nullptr;

        bounds2f lTextInsets_ = bounds2f::ZERO;
    };
}
}

#endif
