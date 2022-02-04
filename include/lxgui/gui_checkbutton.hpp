#ifndef LXGUI_GUI_CHECKBUTTON_HPP
#define LXGUI_GUI_CHECKBUTTON_HPP

#include "lxgui/gui_button.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

namespace lxgui::gui {

/// A #button with two additional states: checked and unchecked.
/** This region works exactly like a classic #button, but is has two
 *   additional special textures for the check sign.
 */
class check_button : public button {
    using base = button;

public:
    /// Constructor.
    explicit check_button(utils::control_block& mBlock, manager& mManager);

    /// Prints all relevant information about this region in a string.
    /** \param sTab The offset to give to all lines
     *   \return All relevant information about this region
     */
    std::string serialize(const std::string& sTab) const override;

    /// Copies a region's parameters into this check button (inheritance).
    /** \param mObj The region to copy
     */
    void copy_from(const region& mObj) override;

    /// Checks this button.
    virtual void check();

    /// UnChecks this button.
    virtual void uncheck();

    /// Disables this check button.
    /** \note A disabled button doesn't receive any input.
     */
    void disable() override;

    /// Enables this check button.
    void enable() override;

    /// Releases this check button.
    /** \note This function only has a visual impact:
     *         the OnClick() handler is not called.
     */
    void release() override;

    /// Checks if this check button is checked.
    /** \return 'true' if checked, 'false' otherwise
     */
    bool is_checked() const;

    /// Returns this button's checked texture.
    /** \return This button's checked texture
     */
    const utils::observer_ptr<texture>& get_checked_texture() {
        return pCheckedTexture_;
    }

    /// Returns this button's checked texture.
    /** \return This button's checked texture
     */
    utils::observer_ptr<const texture> get_checked_texture() const {
        return pCheckedTexture_;
    }

    /// Returns this button's disabled checked texture.
    /** \return This button's disabled checked texture
     */
    const utils::observer_ptr<texture>& get_disabled_checked_texture() {
        return pDisabledCheckedTexture_;
    }

    /// Returns this button's disabled checked texture.
    /** \return This button's disabled checked texture
     */
    utils::observer_ptr<const texture> get_disabled_checked_texture() const {
        return pDisabledCheckedTexture_;
    }

    /// Sets this button's checked texture.
    /** \param pTexture The new texture
     */
    void set_checked_texture(utils::observer_ptr<texture> pTexture);

    /// Sets this button's disabled checked texture.
    /** \param pTexture The new texture
     */
    void set_disabled_checked_texture(utils::observer_ptr<texture> pTexture);

    /// Returns this region's Lua glue.
    void create_glue() override;

    /// Registers this region class to the provided Lua state
    static void register_on_lua(sol::state& mLua);

    static constexpr const char* CLASS_NAME = "CheckButton";

protected:
    void parse_all_nodes_before_children_(const layout_node& mNode) override;

    bool bChecked_ = false;

    utils::observer_ptr<texture> pCheckedTexture_         = nullptr;
    utils::observer_ptr<texture> pDisabledCheckedTexture_ = nullptr;
};

} // namespace lxgui::gui

#endif
