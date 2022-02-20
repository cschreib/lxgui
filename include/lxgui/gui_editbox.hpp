#ifndef LXGUI_GUI_EDITBOX_HPP
#define LXGUI_GUI_EDITBOX_HPP

#include "lxgui/gui_bounds2.hpp"
#include "lxgui/gui_color.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/input_keys.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/utils_periodic_timer.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui::gui {

class font_string;
class texture;

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
 *   etc.). Triggered after `OnChar`.
 *   - `OnTextSet`: Triggered by edit_box::set_text. Will always be
 *   followed by `OnTextChanged`.
 */
class edit_box : public frame {
    using base = frame;

public:
    /// Constructor.
    explicit edit_box(utils::control_block& block, manager& mgr);

    /// Copies a region's parameters into this edit_box (inheritance).
    /** \param obj The region to copy
     */
    void copy_from(const region& obj) override;

    /// Updates this region's logic.
    /** \param delta Time spent since last update
     *   \note Triggered callbacks could destroy the frame. If you need
     *         to use the frame again after calling this function, use
     *         the helper class alive_checker.
     */
    void update(float delta) override;

    /// Calls a script.
    /** \param script_name The name of the script
     *   \param data       Stores scripts arguments
     *   \note Triggered callbacks could destroy the frame. If you need
     *         to use the frame again after calling this function, use
     *         the helper class alive_checker.
     */
    void
    fire_script(const std::string& script_name, const event_data& data = event_data{}) override;

    /// Returns 'true' if this edit_box can use a script.
    /** \param script_name The name of the script
     *   \note This method can be overriden if needed.
     */
    bool can_use_script(const std::string& script_name) const override;

    /// Sets the content of this edit_box.
    /** \param content The content of this edit_box
     */
    void set_text(const utils::ustring& content);

    /// Returns the content of this edit_box.
    /** \return The content of this edit_box
     */
    const utils::ustring& get_text() const;

    /// Selects a portion of the content.
    /** \param start      The first character to select
     *   \param end        The last character to select
     *   \param force_update 'true' to bypass all redundancy checks
     *   \note Will select (end - start) characters
     */
    void highlight_text(
        std::size_t start        = 0u,
        std::size_t end          = std::numeric_limits<std::size_t>::max(),
        bool        force_update = false);

    /// Deselects the selected text, if any.
    void unlight_text();

    /// Sets the color of the highlight quad.
    /** \param c The color
     */
    void set_highlight_color(const color& c);

    /// Inserts some text after the cursor.
    /** \param content The text to insert
     */
    void insert_after_cursor(const utils::ustring& content);

    /// Returns the current position of the cursor.
    /** \return The position of the cursor (0: before first character,
                get_num-letters(): after last character).
    */
    std::size_t get_cursor_position() const;

    /// Moves the cursor to a chosen position.
    /** \param pos The new cursor position (0: before first character,
                     get_num-letters(): after last character).
    */
    void set_cursor_position(std::size_t pos);

    /// Sets the maximum number of letters to allow in this edit_box.
    /** \param max_letters The max number of letters
     */
    void set_max_letters(std::size_t max_letters);

    /// Returns the maximum number of letters to allow in this edit_box.
    /** \return the maximum number of letters to allow in this edit_box
     */
    std::size_t get_max_letters() const;

    /// Returns the number of letters in the content.
    /** \return The number of letters in the content
     */
    std::size_t get_num_letters() const;

    /// Sets the carret's blink speed.
    /** \param blink_period The number of seconds to wait between each blink
     */
    void set_blink_period(double blink_period);

    /// Returns the carret's blink speed.
    /** \return the carret's blink speed (time in seconds between each blink)
     */
    double get_blink_period() const;

    /// Makes this edit_box allow numeric characters only.
    /** \param numeric_only 'true' to only allow numeric characters
     */
    void set_numeric_only(bool numeric_only);

    /// Makes this edit_box allow positive numbers only.
    /** \param positive_only 'true' to only allow positive numbers
     *   \note Only workds if set_numeric_only(true) has been called.
     */
    void set_positive_only(bool positive_only);

    /// Makes this edit_box allow integer numbers only.
    /** \param integer_only 'true' to only allow integer numbers
     *   \note Only workds if set_numeric_only(true) has been called.
     */
    void set_integer_only(bool integer_only);

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
    /** \param enable 'true' to enable password mode
     *   \note In password mode, the content of the edit_box is replaced
     *         by stars (*).
     */
    void enable_password_mode(bool enable);

    /// Checks if this edit_box is in password mode.
    /** \return 'true' if this edit_box is in password mode
     */
    bool is_password_mode_enabled() const;

    /// Allows this edit_box to have several lines in it.
    /** \param multi_line 'true' to allow several lines in this edit_box
     *   \note The behavior of a "multi line" edit_box is very different from
     *         a single line one.<br>
     *         History lines are only available to single line edit_boxes.<br>
     *         Scrolling in a single line edit_box is done horizontally, while
     *         it is only done vertically in a multi line one.
     */
    void set_multi_line(bool multi_line);

    /// Checks if this edit_box can have several lines in it.
    /** \return 'true' if this edit_box can have several lines in it
     */
    bool is_multi_line() const;

    /// Sets the maximum number of history lines this edit_box can keep.
    /** \param max_history_lines The max number of history lines
     */
    void set_max_history_lines(std::size_t max_history_lines);

    /// Returns the maximum number of history lines this edit_box can keep.
    /** \return The maximum number of history lines this edit_box can keep
     */
    std::size_t get_max_history_lines() const;

    /// Adds a new history line to the history line list.
    /** \param history_line The content of this history line
     *   \note This option is only available to single line edit_boxes.
     */
    void add_history_line(const utils::ustring& history_line);

    /// Returns the history line list.
    /** \return The history line list
     *   \note This list will always be empty for multi line edit_boxes.
     */
    const std::vector<utils::ustring>& get_history_lines() const;

    /// Clears the history line list.
    void clear_history();

    /// Sets whether keyboard arrows move the carret or not.
    /** \param arrows_ignored 'true' to ignore arrow keys
     */
    void set_arrows_ignored(bool arrows_ignored);

    /// Sets the insets used to render the content text.
    /** \param insets (left, right, top, bottom)
     *   \note Positive insets will reduce the text area, while
     *         negative ones will enlarge it
     */
    void set_text_insets(const bounds2f& insets);

    /// Returns the text insets.
    /** \return The text insets
     */
    const bounds2f& get_text_insets() const;

    /// Returns the font_string used to render the content.
    /** \return The font_string used to render the content
     */
    const utils::observer_ptr<font_string>& get_font_string() {
        return font_string_;
    }

    /// Returns the font_string used to render the content.
    /** \return The font_string used to render the content
     */
    utils::observer_ptr<const font_string> get_font_string() const {
        return font_string_;
    }

    /// Sets the font_string to use to render the content.
    /** \param fstr The font_string to use to render the content
     */
    void set_font_string(utils::observer_ptr<font_string> fstr);

    /// Sets the font (file and size) to render the content.
    /** \param font_name The file path to the .ttf file
     *   \param height   The font height
     */
    void set_font(const std::string& font_name, float height);

    /// Notifies this frame that it has received or lost focus.
    /** \param focus 'true' if focus is received, 'false' if lost
     */
    void notify_focus(bool focus) override;

    /// Tells this region that the global interface scaling factor has changed.
    void notify_scaling_factor_updated() override;

    /// Returns this region's Lua glue.
    void create_glue() override;

    /// Registers this region class to the provided Lua state
    static void register_on_lua(sol::state& lua);

    static constexpr const char* class_name = "EditBox";

protected:
    void parse_attributes_(const layout_node& node) override;
    void parse_all_nodes_before_children_(const layout_node& node) override;
    void parse_font_string_node_(const layout_node& node);
    void parse_text_insets_node_(const layout_node& node);

    void create_font_string_();
    void create_highlight_();
    void create_carret_();

    void check_text_();
    void update_displayed_text_();
    void update_font_string_();
    void update_carret_position_();

    bool        add_char_(char32_t c);
    bool        remove_char_();
    std::size_t get_letter_id_at_(const vector2f& position) const;
    bool        move_carret_at_(const vector2f& position);
    bool        move_carret_horizontally_(bool forward = true);
    bool        move_carret_vertically_(bool down = true);

    void process_key_(input::key key_id, bool shift_is_pressed, bool ctrl_is_pressed);

    utils::ustring           unicode_text_;
    utils::ustring           displayed_text_;
    utils::ustring::iterator iter_carret_pos_;
    utils::ustring::iterator iter_carret_pos_old_;

    std::size_t display_pos_        = 0;
    std::size_t num_letters_        = 0;
    std::size_t max_letters_        = std::numeric_limits<std::size_t>::max();
    bool        is_numeric_only_    = false;
    bool        is_positive_only_   = false;
    bool        is_integer_only_    = false;
    bool        is_password_mode_   = false;
    bool        is_multi_line_      = false;
    bool        are_arrows_ignored_ = false;

    utils::observer_ptr<texture> highlight_           = nullptr;
    color                        highlight_color_     = color(1.0f, 1.0f, 1.0f, 0.5f);
    std::size_t                  selection_start_pos_ = 0u;
    std::size_t                  selection_end_pos_   = 0u;
    bool                         is_text_selected_    = false;

    utils::observer_ptr<texture> carret_       = nullptr;
    double                       blink_period_ = 0.5;
    utils::periodic_timer        carret_timer_;

    std::vector<utils::ustring> history_line_list_;
    std::size_t                 max_history_lines_    = std::numeric_limits<std::size_t>::max();
    std::size_t                 current_history_line_ = std::numeric_limits<std::size_t>::max();

    utils::observer_ptr<font_string> font_string_ = nullptr;

    bounds2f text_insets_ = bounds2f::zero;
};

} // namespace lxgui::gui

#endif
