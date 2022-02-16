#ifndef LXGUI_GUI_ADDON_REGISTRY_HPP
#define LXGUI_GUI_ADDON_REGISTRY_HPP

#include "lxgui/gui_addon.hpp"
#include "lxgui/lxgui.hpp"

#include <string>
#include <unordered_map>
#include <vector>

/** \cond INCLUDE_INTERNALS_IN_DOC
 */
namespace sol {

class state;

}
/** \endcond
 */

namespace lxgui::gui {

class localizer;
class event_emitter;
class root;
class virtual_root;

/// Loads and owns addons
class addon_registry {
public:
    /// Constructor.
    /** \param mLua          The GUI Lua state
     *   \param mLocalizer    The localizer class, to load new translation into
     *   \param event_emitter The event emitter, to fire "addon loaded" events
     *   \param mRoot         The GUI root, to create new frames into
     *   \param mVirtualRoot  The virtual root, to create new virtual frames into
     */
    addon_registry(
        sol::state&    m_lua,
        localizer&     m_localizer,
        event_emitter& m_event_emitter,
        root&          m_root,
        virtual_root&  m_virtual_root);

    addon_registry(const addon_registry&) = delete;
    addon_registry(addon_registry&&)      = delete;
    addon_registry& operator=(const addon_registry&) = delete;
    addon_registry& operator=(addon_registry&&) = delete;

    /// Parse all addons inside a directory.
    /** \param directory The directory to load addons from
     *   \note The directory must contain a file named addon.txt, and
     *         listing all enabled (and possibly disabled) addons.
     *         Each addon is then a sub-directory.
     */
    void load_addon_directory(const std::string& directory);

    /// Returns the addon that is being parsed.
    /** \return The addon that is being parsed
     */
    const addon* get_current_addon();

    /// Sets the current addon.
    /** \param pAddOn The current addon
     *   \note The current addon is used to set the addon of each new region.
     *         This is normally set by the parser, while loading each addon.
     *         For regions created manually, after the loading stage, this is
     *         also set by @ref frame, before each call to a handler function.
     */
    void set_current_addon(const addon* p_add_on);

    /// Save Lua variables registred for saving for all addons.
    void save_variables() const;

private:
    void load_addon_toc_(const std::string& add_on_name, const std::string& add_on_directory);
    void load_addon_files_(const addon& m_add_on);

    void save_variables_(const addon& m_add_on) const noexcept;

    void parse_layout_file_(const std::string& file, const addon& m_add_on);

    template<typename T>
    using string_map = std::unordered_map<std::string, T>;

    sol::state&    m_lua_;
    localizer&     m_localizer_;
    event_emitter& m_event_emitter_;
    root&          m_root_;
    virtual_root&  m_virtual_root_;

    const addon*                  p_current_add_on_ = nullptr;
    string_map<string_map<addon>> add_on_list_;
};

} // namespace lxgui::gui

#endif
