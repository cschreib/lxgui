#ifndef LXGUI_GUI_MANAGER_HPP
#define LXGUI_GUI_MANAGER_HPP

#include "lxgui/input_keys.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_observer.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

/** \cond INCLUDE_INTERNALS_IN_DOC
 */
namespace sol {

class state;

}
/** \endcond
 */

namespace lxgui::input {

class source;
class window;
class dispatcher;
class world_dispatcher;

} // namespace lxgui::input

namespace lxgui::gui {

class renderer;
class localizer;
class factory;
class root;
class virtual_root;
class addon_registry;
class event_emitter;

/// Manages the user interface
class manager : utils::enable_observer_from_this<manager> {
public:
    /// Constructor.
    /** \param mBlock       The owner pointer control block
     *   \param input_source The input source to use
     *   \param renderer    The renderer implementation
     */
    manager(
        utils::control_block&          m_block,
        std::unique_ptr<input::source> p_input_source,
        std::unique_ptr<renderer>      p_renderer);

    /// Destructor.
    ~manager() override;

    manager(const manager& m_mgr) = delete;
    manager(manager&& m_mgr)      = delete;
    manager& operator=(const manager& m_mgr) = delete;
    manager& operator=(manager&& m_mgr) = delete;

    /// Sets the global UI scaling factor.
    /** \param fScalingFactor The factor to use for rescaling (1: no rescaling, default)
     *   \note This value determines how to convert sizing units or position coordinates
     *         into actual number of pixels. By default, units specified for sizes and
     *         positions are 1:1 mapping with pixels on the screen. If designing the UI
     *         on a "traditional" display (say, 1080p resolution monitor), the UI will not
     *         scale correctly when running on high-DPI displays unless the scaling factor is
     *         adjusted accordingly. The value of the scaling factor should be the ratio
     *         DPI_target/DPI_dev, where DPI_dev is the DPI of the display used for
     *         development, and DPI_target is the DPI of the display used to run the program.
     *         In addition, the scaling factor can also be used to improve accessibility of
     *         the interface to users with poorer eye sight, which would benefit from larger
     *         font sizes and larger icons.
     */
    void set_interface_scaling_factor(float f_scaling_factor);

    /// Returns the current UI scaling factor.
    /** \return The current UI scaling factor
     *   \see set_interface_scaling_factor()
     */
    float get_interface_scaling_factor() const;

    /// Enables or disables interface caching.
    /** \param enable_caching 'true' to enable, 'false' to disable
     *   \see toggle_caching()
     */
    void enable_caching(bool enable_caching);

    /// Toggles interface caching.
    /** \note Disabled by default. Enabling this will most likely improve performances,
     *         at the expense of higher GPU memory usage. The UI will be cached into
     *         large render targets, which are only redrawn when the UI changes, rather
     *         than redrawn on each frame.
     */
    void toggle_caching();

    /// Checks if interface caching is enabled.
    /** \return 'true' if interface caching is enabled
     *   \see toggle_caching()
     */
    bool is_caching_enabled() const;

    /// Adds a new directory to be parsed for UI addons.
    /** \param directory The new directory
     *   \note If the UI is already loaded, this change will only take effect after
     *         the UI is reloaded, see reload_ui().
     */
    void add_addon_directory(const std::string& directory);

    /// Clears the addon directory list.
    /** \note This is usefull whenever you need to reload a
     *         completely different UI (for example, when switching
     *         from your game's main menu to the real game).
     *   \note If the UI is already loaded, this change will only take effect after
     *         the UI is reloaded, see reload_ui().
     */
    void clear_addon_directory_list();

    /// Set the code to be executed on each fresh Lua state.
    /** \param pLuaRegs Some code that will get executed immediately after the Lua
     *                   state is created
     *   \note This function is usefull if you need to create additionnal
     *         resources on the Lua state before the GUI files are loaded.
     *         The argument to this function will be stored and reused, each time
     *         the Lua state is created (e.g., when the GUI is re-loaded, see reload_ui()).
     *         If the UI is already loaded, this change will only take effect after
     *         the UI is reloaded.
     */
    void register_lua_glues(std::function<void(gui::manager&)> p_lua_regs);

    /// Prints debug informations in the log file.
    /** \note Calls region::serialize().
     */
    std::string print_ui() const;

    /// Loads the UI.
    /** \note Creates the Lua state and loads addon files (if any).
     *         Calling this function if the UI is already loaded will have no effect.
     *         If your intent is to re-load the UI, use reload_ui() instead.
     */
    void load_ui();

    /// Closes the UI (at the end of the current or next update_ui()).
    /** \note The actual closing may be deferred if called from within update_ui(),
     *         therefore it is safe to call this function at any time. If you need to
     *         close the UI without delay, use close_ui_now().
     */
    void close_ui();

    /// Closes the UI (immediately).
    /** \note All regions will be deleted, and the Lua state will be closed.
     *   \warning Do not call this function while the manager is running update_ui()
     *            (i.e., do not call this directly from a frame's callback, C++ or Lua).
     */
    void close_ui_now();

    /// Closes the UI and load it again (at the end of the current or next update_ui()).
    /** \note The actual re-loading may be deferred if called from within update_ui(),
     *         therefore it is safe to call this function at any time. If you need to
     *         reload the UI without delay, use reload_ui_now().
     */
    void reload_ui();

    /// Closes the UI and load it again (immediately).
    /** \note Calls close_ui_now() then load_ui().
     *   \warning Do not call this function while the manager is running update_ui()
     *            (i.e., do not call this directly from a frame's callback, C++ or Lua).
     */
    void reload_ui_now();

    /// Checks if the UI has been loaded.
    /** \return 'true' if the UI has being loaded
     */
    bool is_loaded() const;

    /// Renders the UI into the current render target.
    void render_ui() const;

    /// Updates this manager and its regions.
    /** \param delta The time elapsed since the last call
     */
    void update_ui(float f_delta);

    /// Returns the GUI Lua state (sol wrapper).
    /** \return The GUI Lua state
     */
    sol::state& get_lua();

    /// Returns the GUI Lua state (sol wrapper).
    /** \return The GUI Lua state
     */
    const sol::state& get_lua() const;

    /// Returns the renderer implementation.
    /** \return The renderer implementation
     */
    const renderer& get_renderer() const {
        return *p_renderer_;
    }

    /// Returns the renderer implementation.
    /** \return The renderer implementation
     */
    renderer& get_renderer() {
        return *p_renderer_;
    }

    /// Returns the window in which this gui is being displayed.
    /** \return The window in which this gui is being displayed
     */
    const input::window& get_window() const {
        return *p_window_;
    }

    /// Returns the window in which this gui is being displayed.
    /** \return The window in which this gui is being displayed
     */
    input::window& get_window() {
        return *p_window_;
    }

    /// Returns the input manager associated to this gui.
    /** \return The input manager associated to this gui
     */
    const input::dispatcher& get_input_dispatcher() const {
        return *p_input_dispatcher_;
    }

    /// Returns the input manager associated to this gui.
    /** \return The input manager associated to this gui
     */
    input::dispatcher& get_input_dispatcher() {
        return *p_input_dispatcher_;
    }

    /// Returns the input manager associated to this gui.
    /** \return The input manager associated to this gui
     */
    const input::world_dispatcher& get_world_input_dispatcher() const {
        return *p_world_input_dispatcher_;
    }

    /// Returns the input manager associated to this gui.
    /** \return The input manager associated to this gui
     */
    input::world_dispatcher& get_world_input_dispatcher() {
        return *p_world_input_dispatcher_;
    }

    /// Returns the gui event emitter.
    /** \return The gui event emitter
     */
    const event_emitter& get_event_emitter() const {
        return *p_event_emitter_;
    }

    /// Returns the gui event emitter.
    /** \return The gui event emitter
     */
    event_emitter& get_event_emitter() {
        return *p_event_emitter_;
    }

    /// Returns the object used for localizing strings.
    /** \return The current localizer
     */
    localizer& get_localizer() {
        return *p_localizer_;
    }

    /// Returns the object used for localizing strings.
    /** \return The current localizer
     */
    const localizer& get_localizer() const {
        return *p_localizer_;
    }

    /// Returns the UI root object, which contains root frames.
    /** \return The root object
     */
    root& get_root() {
        return *p_root_;
    }

    /// Returns the UI root object, which contains root frames.
    /** \return The root object
     */
    const root& get_root() const {
        return *p_root_;
    }

    /// Returns the UI root object, which contains root frames.
    /** \return The root object
     */
    virtual_root& get_virtual_root() {
        return *p_virtual_root_;
    }

    /// Returns the UI root object, which contains root frames.
    /** \return The root object
     */
    const virtual_root& get_virtual_root() const {
        return *p_virtual_root_;
    }

    /// Returns the UI object factory, which is used to create new objects.
    /** \return The factory object
     */
    factory& get_factory() {
        return *p_factory_;
    }

    /// Returns the UI object factory, which is used to create new objects.
    /** \return The factory object
     */
    const factory& get_factory() const {
        return *p_factory_;
    }

    /// Returns the addon registry, which keeps track of loaded addons.
    /** \return The registry object
     */
    addon_registry* get_addon_registry() {
        return p_add_on_registry_.get();
    }

    /// Returns the addon registry, which keeps track of loaded addons.
    /** \return The registry object
     */
    const addon_registry* get_addon_registry() const {
        return p_add_on_registry_.get();
    }

private:
    /// Creates the lua::state that will be used to communicate with the GUI.
    /**
     *   \warning Do not call this function while the manager is running update_ui()
     *           (i.e., do not call this directly from a frame's callback, C++ or Lua).
     */
    void create_lua_();

    /// Reads GUI files in the directory list.
    /** \note See add_addon_directory().
     *   \note See load_ui().
     *   \note See create_lua().
     *   \warning Do not call this function while the manager is running update_ui()
     *            (i.e., do not call this directly from a frame's callback, C++ or Lua).
     */
    void read_files_();

    // Persistent state
    float                              f_scaling_factor_      = 1.0f;
    float                              f_base_scaling_factor_ = 1.0f;
    bool                               enable_caching_        = false;
    std::function<void(gui::manager&)> p_lua_regs_;
    std::vector<std::string>           gui_directory_list_;

    // Implementations
    std::unique_ptr<input::source> p_input_source_;
    std::unique_ptr<renderer>      p_renderer_;

    // IO
    std::unique_ptr<input::window>           p_window_;
    std::unique_ptr<input::dispatcher>       p_input_dispatcher_;
    std::unique_ptr<input::world_dispatcher> p_world_input_dispatcher_;
    std::unique_ptr<event_emitter>           p_event_emitter_;

    // UI state
    std::unique_ptr<factory>        p_factory_;
    std::unique_ptr<localizer>      p_localizer_;
    std::unique_ptr<sol::state>     p_lua_;
    utils::owner_ptr<root>          p_root_;
    utils::owner_ptr<virtual_root>  p_virtual_root_;
    std::unique_ptr<addon_registry> p_add_on_registry_;

    bool is_loaded_          = false;
    bool reload_ui_flag_     = false;
    bool close_ui_flag_      = false;
    bool is_first_iteration_ = true;
    bool is_updating_        = false;
};

} // namespace lxgui::gui

#endif
