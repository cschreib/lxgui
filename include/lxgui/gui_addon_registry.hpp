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
    /** \param lua The GUI Lua state
     * \param loc The localizer class, to load new translation into
     * \param emitter The event emitter, to fire "addon loaded" events
     * \param r The GUI root, to create new frames into
     * \param vr The virtual root, to create new virtual frames into
     */
    addon_registry(
        sol::state& lua, localizer& loc, event_emitter& emitter, root& r, virtual_root& vr);

    addon_registry(const addon_registry&) = delete;
    addon_registry(addon_registry&&)      = delete;
    addon_registry& operator=(const addon_registry&) = delete;
    addon_registry& operator=(addon_registry&&) = delete;

    /// Parse all addons inside a directory.
    /** \param directory The directory to load addons from
     * \note The directory must contain a file named addon.txt, and
     *       listing all enabled (and possibly disabled) addons.
     *       Each addon is then a sub-directory.
     */
    void load_addon_directory(const std::string& directory);

    /// Returns the addon that is being parsed.
    /** \return The addon that is being parsed
     */
    const addon* get_current_addon();

    /// Sets the current addon.
    /** \param a The current addon
     * \note The current addon is used to set the addon of each new region.
     *       This is normally set by the parser, while loading each addon.
     *       For regions created manually, after the loading stage, this is
     *       also set by @ref frame, before each call to a handler function.
     */
    void set_current_addon(const addon* a);

    /// Save Lua variables registred for saving for all addons.
    void save_variables() const;

private:
    void load_addon_toc_(const std::string& addon_name, const std::string& addon_directory);
    void load_addon_files_(const addon& a);

    void save_variables_(const addon& a) const noexcept;

    void parse_layout_file_(const std::string& file_name, const addon& a);

    template<typename T>
    using string_map = std::unordered_map<std::string, T>;

    sol::state&    lua_;
    localizer&     localizer_;
    event_emitter& event_emitter_;
    root&          root_;
    virtual_root&  virtual_root_;

    const addon*                  current_addon_ = nullptr;
    string_map<string_map<addon>> addon_list_;
};

} // namespace lxgui::gui

#endif
