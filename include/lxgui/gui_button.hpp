#ifndef LXGUI_GUI_BUTTON_HPP
#define LXGUI_GUI_BUTTON_HPP

#include "lxgui/gui_frame.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/utils_string.hpp"

#include <string>

namespace lxgui::gui {

class texture;
class font_string;

/**
 * \brief A #frame with a button that can be clicked.
 * \details This class can handle three different states: "normal", "pushed"
 * and "disabled". You can provide a different texture for each of
 * these states, and two different fontstrings for "normal" and
 * "disabled".
 *
 * In addition, you can provide another texture/fontstring for the
 * "highlight" state (when the mouse is over the button region).
 *
 * Note that there is no fontstring for the "pushed" state: in this
 * case, the "normal" font is rendered with a slight offset that you
 * are free to define.
 *
 * Note that a button has frame::enable_mouse set to `true` by
 * default.
 *
 * __Events.__ Hard-coded events available to all buttons, in
 * addition to those from frame:
 *
 * - `OnClick`: Triggered when the button is clicked, either when
 * button::click is called, or just when a mouse button is pressed
 * when the cursor is over the button.
 * - `OnDoubleClick`: Triggered when the button is double-clicked.
 * - `OnEnable`: Triggered by button::enable.
 * - `OnDisable`: Triggered by button::disable.
 */
class button : public frame {
public:
    using base = frame;

    enum class state { up, down, disabled };

    /// Constructor.
    explicit button(utils::control_block& block, manager& mgr, const frame_core_attributes& attr);

    /**
     * \brief Prints all relevant information about this region in a string.
     * \param tab The offset to give to all lines
     * \return All relevant information about this region
     */
    std::string serialize(const std::string& tab) const override;

    /**
     * \brief Returns 'true' if this button can use a script.
     * \param script_name The name of the script
     * \note This method can be overridden if needed.
     */
    bool can_use_script(const std::string& script_name) const override;

    /**
     * \brief Calls a script.
     * \param script_name The name of the script
     * \param data Stores scripts arguments
     * \note Triggered callbacks could destroy the frame. If you need
     * to use the frame again after calling this function, use
     * the helper class alive_checker.
     */
    void
    fire_script(const std::string& script_name, const event_data& data = event_data{}) override;

    /**
     * \brief Copies a region's parameters into this button (inheritance).
     * \param obj The region to copy
     */
    void copy_from(const region& obj) override;

    /**
     * \brief Sets this button's text.
     * \param content The new text
     */
    void set_text(const utils::ustring& content);

    /**
     * \brief Returns this button's text.
     * \return This button's text
     */
    const utils::ustring& get_text() const;

    /**
     * \brief Returns this button's normal texture.
     * \return This button's normal texture
     */
    const utils::observer_ptr<texture>& get_normal_texture() {
        return normal_texture_;
    }

    /**
     * \brief Returns this button's normal texture.
     * \return This button's normal texture
     */
    utils::observer_ptr<const texture> get_normal_texture() const {
        return normal_texture_;
    }

    /**
     * \brief Returns this button's pushed texture.
     * \return This button's pushed texture
     */
    const utils::observer_ptr<texture>& get_pushed_texture() {
        return pushed_texture_;
    }

    /**
     * \brief Returns this button's pushed texture.
     * \return This button's pushed texture
     */
    utils::observer_ptr<const texture> get_pushed_texture() const {
        return pushed_texture_;
    }

    /**
     * \brief Returns this button's disabled texture.
     * \return This button's disabled texture
     */
    const utils::observer_ptr<texture>& get_disabled_texture() {
        return disabled_texture_;
    }

    /**
     * \brief Returns this button's disabled texture.
     * \return This button's disabled texture
     */
    utils::observer_ptr<const texture> get_disabled_texture() const {
        return disabled_texture_;
    }

    /**
     * \brief Returns this button's highlight texture.
     * \return This button's highlight texture
     */
    const utils::observer_ptr<texture>& get_highlight_texture() {
        return highlight_texture_;
    }

    /**
     * \brief Returns this button's highlight texture.
     * \return This button's highlight texture
     */
    utils::observer_ptr<const texture> get_highlight_texture() const {
        return highlight_texture_;
    }

    /**
     * \brief Returns this button's normal text.
     * \return This button's normal text
     */
    const utils::observer_ptr<font_string>& get_normal_text() {
        return normal_text_;
    }

    /**
     * \brief Returns this button's normal text.
     * \return This button's normal text
     */
    utils::observer_ptr<const font_string> get_normal_text() const {
        return normal_text_;
    }

    /**
     * \brief Returns this button's highlight text.
     * \return This button's highlight text
     */
    const utils::observer_ptr<font_string>& get_highlight_text() {
        return highlight_text_;
    }

    /**
     * \brief Returns this button's highlight text.
     * \return This button's highlight text
     */
    utils::observer_ptr<const font_string> get_highlight_text() const {
        return highlight_text_;
    }

    /**
     * \brief Returns this button's disabled text.
     * \return This button's disabled text
     */
    const utils::observer_ptr<font_string>& get_disabled_text() {
        return disabled_text_;
    }

    /**
     * \brief Returns this button's disabled text.
     * \return This button's disabled text
     */
    utils::observer_ptr<const font_string> get_disabled_text() const {
        return disabled_text_;
    }

    /**
     * \brief Returns the currently displayed text object.
     * \return The currently displayed text object
     */
    const utils::observer_ptr<font_string>& get_current_font_string() {
        return current_font_string_;
    }

    /**
     * \brief Returns the currently displayed text object.
     * \return The currently displayed text object
     */
    utils::observer_ptr<const font_string> get_current_font_string() const {
        return current_font_string_;
    }

    /**
     * \brief Sets this button's normal texture.
     * \param tex The new texture
     */
    void set_normal_texture(utils::observer_ptr<texture> tex);

    /**
     * \brief Sets this button's pushed texture.
     * \param tex The new texture
     */
    void set_pushed_texture(utils::observer_ptr<texture> tex);

    /**
     * \brief Sets this button's disabled texture.
     * \param tex The new texture
     */
    void set_disabled_texture(utils::observer_ptr<texture> tex);

    /**
     * \brief Sets this button's highlight texture.
     * \param tex The new texture
     */
    void set_highlight_texture(utils::observer_ptr<texture> tex);

    /**
     * \brief Sets this button's normal text.
     * \param fstr The new text object
     */
    void set_normal_text(utils::observer_ptr<font_string> fstr);

    /**
     * \brief Sets this button's highlight text.
     * \param fstr The new text object
     */
    void set_highlight_text(utils::observer_ptr<font_string> fstr);

    /**
     * \brief Sets this button's disabled text.
     * \param fstr The new text object
     */
    void set_disabled_text(utils::observer_ptr<font_string> fstr);

    /**
     * \brief Disables this button.
     * \note A disabled button doesn't receive any input.
     */
    virtual void disable();

    /// Enables this button.
    virtual void enable();

    /**
     * \brief Checks if this button is enabled.
     * \return 'true' if this button is enabled
     */
    bool is_enabled() const;

    /**
     * \brief Pushed this button.
     * \note This function only has a visual impact: the OnClick() handler is not called.
     */
    virtual void push();

    /**
     * \brief Releases this button.
     * \note This function only has a visual impact: the OnClick() handler is not called.
     */
    virtual void release();

    /**
     * \brief Highlights this button.
     * \note The button will be highlighted even if the
     * mouse is not over it. It will stop when the
     * mouse leaves it.
     */
    virtual void highlight();

    /**
     * \brief Unlights this button.
     * \note The button will be unlighted even if the
     * mouse is over it. It will highlight again
     * when the mouse leaves then enters its region.
     */
    virtual void unlight();

    /**
     * \brief Returns this button's state.
     * \return This button's state (see ButtonState)
     */
    state get_button_state() const;

    /**
     * \brief Locks this button's highlighting.
     * \note The button will always be highlighted
     * until you call unlock_highlight().
     */
    void lock_highlight();

    /// Unlocks this button's highlighting.
    void unlock_highlight();

    /**
     * \brief Sets this button's pushed text offset.
     * \param offset The pushed text offset
     */
    void set_pushed_text_offset(const vector2f& offset);

    /**
     * \brief Returns this button's pushed text offset.
     * \return This button's pushed text offset
     */
    const vector2f& get_pushed_text_offset() const;

    /// Registers this region class to the provided Lua state
    static void register_on_lua(sol::state& lua);

    static constexpr const char* class_name = "Button";

protected:
    void parse_attributes_(const layout_node& node) override;
    void parse_all_nodes_before_children_(const layout_node& node) override;

    const std::vector<std::string>& get_type_list_() const override;

    state state_               = state::up;
    bool  is_highlighted_      = false;
    bool  is_highlight_locked_ = false;

    utils::ustring content_;

    utils::observer_ptr<texture> normal_texture_    = nullptr;
    utils::observer_ptr<texture> pushed_texture_    = nullptr;
    utils::observer_ptr<texture> disabled_texture_  = nullptr;
    utils::observer_ptr<texture> highlight_texture_ = nullptr;

    utils::observer_ptr<font_string> normal_text_         = nullptr;
    utils::observer_ptr<font_string> highlight_text_      = nullptr;
    utils::observer_ptr<font_string> disabled_text_       = nullptr;
    utils::observer_ptr<font_string> current_font_string_ = nullptr;

    vector2f pushed_text_offset_ = vector2f::zero;
};

} // namespace lxgui::gui

#endif
