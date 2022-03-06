#ifndef LXGUI_GUI_MANAGER_HPP
#define LXGUI_GUI_MANAGER_HPP

#include "lxgui/input_keys.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_observer.hpp"
#include "lxgui/utils_signal.hpp"

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
    /**
     * \brief Constructor.
     * \param block The owner pointer control block
     * \param src The input source to use
     * \param rdr The renderer implementation
     */
    manager(
        utils::control_block&          block,
        std::unique_ptr<input::source> src,
        std::unique_ptr<renderer>      rdr);

    /// Destructor.
    ~manager() override;

    // Non-copiable, non-movable
    manager(const manager&) = delete;
    manager(manager&&)      = delete;
    manager& operator=(const manager&) = delete;
    manager& operator=(manager&&) = delete;

    /**
     * \brief Sets the global UI scaling factor.
     * \param scaling_factor The factor to use for rescaling (1: no rescaling, default)
     * \note This value determines how to convert sizing units or position coordinates
     * into actual number of pixels. By default, units specified for sizes and
     * positions are 1:1 mapping with pixels on the screen. If designing the UI
     * on a "traditional" display (say, 1080p resolution monitor), the UI will not
     * scale correctly when running on high-DPI displays unless the scaling factor is
     * adjusted accordingly. The value of the scaling factor should be the ratio
     * DPI_target/DPI_dev, where DPI_dev is the DPI of the display used for
     * development, and DPI_target is the DPI of the display used to run the program.
     * In addition, the scaling factor can also be used to improve accessibility of
     * the interface to users with poorer eye sight, which would benefit from larger
     * font sizes and larger icons.
     */
    void set_interface_scaling_factor(float scaling_factor);

    /**
     * \brief Returns the current UI scaling factor.
     * \return The current UI scaling factor
     * \see set_interface_scaling_factor()
     */
    float get_interface_scaling_factor() const;

    /**
     * \brief Enables or disables interface caching.
     * \param enable_caching 'true' to enable, 'false' to disable
     * \see toggle_caching()
     */
    void enable_caching(bool enable_caching);

    /**
     * \brief Toggles interface caching.
     * \note Disabled by default. Enabling this will most likely improve performances,
     * at the expense of higher GPU memory usage. The UI will be cached into
     * large render targets, which are only redrawn when the UI changes, rather
     * than redrawn on each frame.
     */
    void toggle_caching();

    /**
     * \brief Checks if interface caching is enabled.
     * \return 'true' if interface caching is enabled
     * \see toggle_caching()
     */
    bool is_caching_enabled() const;

    /**
     * \brief Adds a new directory to be parsed for UI addons.
     * \param directory The new directory
     * \note If the UI is already loaded, this change will only take effect after
     * the UI is reloaded, see reload_ui().
     */
    void add_addon_directory(const std::string& directory);

    /**
     * \brief Clears the addon directory list.
     * \note This is useful whenever you need to reload a
     * completely different UI (for example, when switching
     * from your game's main menu to the real game).
     * \note If the UI is already loaded, this change will only take effect after
     * the UI is reloaded, see reload_ui().
     */
    void clear_addon_directory_list();

    /**
     * \brief Adds a new directory to be parsed for localization.
     * \param directory The new directory
     * \note If the UI is already loaded, this change will only take effect after
     * the UI is reloaded, see reload_ui().
     */
    void add_localization_directory(const std::string& directory);

    /**
     * \brief Clears the localization directory list.
     * \note This is useful whenever you need to reload a
     * completely different UI (for example, when switching
     * from your game's main menu to the real game).
     * \note If the UI is already loaded, this change will only take effect after
     * the UI is reloaded, see reload_ui().
     */
    void clear_localization_directory_list();

    /**
     * \brief Triggers on each fresh Lua state (e.g., on startup or after a UI re-load).
     * \note This signal is useful if you need to create additionnal
     * resources on the Lua state before the GUI files are loaded.
     * This signal will be triggered each time the Lua state is created (e.g., when the GUI is
     * re-loaded, see reload_ui()). If the UI is already loaded, any new callback connected to this
     * signal will only take effect after the UI is reloaded.
     */
    utils::signal<void(sol::state&)> on_create_lua;

    /**
     * \brief Prints debug information in the log file.
     * \note Calls region::serialize().
     */
    std::string print_ui() const;

    /**
     * \brief Loads the UI.
     * \note Creates the Lua state and loads addon files (if any).
     * Calling this function if the UI is already loaded will have no effect.
     * If your intent is to re-load the UI, use reload_ui() instead.
     */
    void load_ui();

    /**
     * \brief Closes the UI safely (at the end of update_ui()).
     * \note The actual closing will be deferred until the end of update_ui(),
     * therefore it is safe to call this function at any time. If you need to
     * close the UI without delay, use close_ui_now().
     */
    void close_ui();

    /**
     * \brief Closes the UI (immediately).
     * \note All regions will be deleted, and the Lua state will be closed.
     * \warning Do not call this function while the manager is running update_ui()
     * (i.e., do not call this directly from a frame's callback, C++ or Lua).
     */
    void close_ui_now();

    /**
     * \brief Closes the UI and load it again safely (at the end of update_ui()).
     * \note The actual re-loading will be deferred until the end of update_ui(),
     * therefore it is safe to call this function at any time. If you need to
     * reload the UI without delay, use reload_ui_now().
     */
    void reload_ui();

    /**
     * \brief Closes the UI and load it again (immediately).
     * \note Calls close_ui_now() then load_ui().
     * \warning Do not call this function while the manager is running update_ui()
     * (i.e., do not call this directly from a frame's callback, C++ or Lua).
     */
    void reload_ui_now();

    /**
     * \brief Checks if the UI has been loaded.
     * \return 'true' if the UI has being loaded
     */
    bool is_loaded() const;

    /// Renders the UI into the current render target.
    void render_ui() const;

    /**
     * \brief Updates this manager and its regions.
     * \param delta The time elapsed since the last call
     */
    void update_ui(float delta);

    /**
     * \brief Returns the GUI Lua state (sol wrapper).
     * \return The GUI Lua state
     */
    sol::state& get_lua();

    /**
     * \brief Returns the GUI Lua state (sol wrapper).
     * \return The GUI Lua state
     */
    const sol::state& get_lua() const;

    /**
     * \brief Returns the renderer implementation.
     * \return The renderer implementation
     */
    const renderer& get_renderer() const {
        return *renderer_;
    }

    /**
     * \brief Returns the renderer implementation.
     * \return The renderer implementation
     */
    renderer& get_renderer() {
        return *renderer_;
    }

    /**
     * \brief Returns the window in which this gui is being displayed.
     * \return The window in which this gui is being displayed
     */
    const input::window& get_window() const {
        return *window_;
    }

    /**
     * \brief Returns the window in which this gui is being displayed.
     * \return The window in which this gui is being displayed
     */
    input::window& get_window() {
        return *window_;
    }

    /**
     * \brief Returns the input manager associated to this gui.
     * \return The input manager associated to this gui
     */
    const input::dispatcher& get_input_dispatcher() const {
        return *input_dispatcher_;
    }

    /**
     * \brief Returns the input manager associated to this gui.
     * \return The input manager associated to this gui
     */
    input::dispatcher& get_input_dispatcher() {
        return *input_dispatcher_;
    }

    /**
     * \brief Returns the input manager associated to this gui.
     * \return The input manager associated to this gui
     */
    const input::world_dispatcher& get_world_input_dispatcher() const {
        return *world_input_dispatcher_;
    }

    /**
     * \brief Returns the input manager associated to this gui.
     * \return The input manager associated to this gui
     */
    input::world_dispatcher& get_world_input_dispatcher() {
        return *world_input_dispatcher_;
    }

    /**
     * \brief Returns the gui event emitter.
     * \return The gui event emitter
     */
    const event_emitter& get_event_emitter() const {
        return *event_emitter_;
    }

    /**
     * \brief Returns the gui event emitter.
     * \return The gui event emitter
     */
    event_emitter& get_event_emitter() {
        return *event_emitter_;
    }

    /**
     * \brief Returns the object used for localizing strings.
     * \return The current localizer
     */
    localizer& get_localizer() {
        return *localizer_;
    }

    /**
     * \brief Returns the object used for localizing strings.
     * \return The current localizer
     */
    const localizer& get_localizer() const {
        return *localizer_;
    }

    /**
     * \brief Returns the UI root object, which contains root frames.
     * \return The root object
     */
    root& get_root() {
        return *root_;
    }

    /**
     * \brief Returns the UI root object, which contains root frames.
     * \return The root object
     */
    const root& get_root() const {
        return *root_;
    }

    /**
     * \brief Returns the UI root object, which contains root frames.
     * \return The root object
     */
    virtual_root& get_virtual_root() {
        return *virtual_root_;
    }

    /**
     * \brief Returns the UI root object, which contains root frames.
     * \return The root object
     */
    const virtual_root& get_virtual_root() const {
        return *virtual_root_;
    }

    /**
     * \brief Returns the UI object factory, which is used to create new objects.
     * \return The factory object
     */
    factory& get_factory() {
        return *factory_;
    }

    /**
     * \brief Returns the UI object factory, which is used to create new objects.
     * \return The factory object
     */
    const factory& get_factory() const {
        return *factory_;
    }

    /**
     * \brief Returns the addon registry, which keeps track of loaded addons.
     * \return The registry object
     */
    addon_registry* get_addon_registry() {
        return addon_registry_.get();
    }

    /**
     * \brief Returns the addon registry, which keeps track of loaded addons.
     * \return The registry object
     */
    const addon_registry* get_addon_registry() const {
        return addon_registry_.get();
    }

private:
    /**
     * \brief Creates the lua::state that will be used to communicate with the GUI.
     *
     * \warning Do not call this function while the manager is running update_ui()
     * (i.e., do not call this directly from a frame's callback, C++ or Lua).
     */
    void create_lua_();

    /**
     * \brief Reads GUI files in the directory list.
     * \note See add_addon_directory().
     * \note See load_ui().
     * \note See create_lua().
     * \warning Do not call this function while the manager is running update_ui()
     * (i.e., do not call this directly from a frame's callback, C++ or Lua).
     */
    void read_files_();

    // Persistent state
    float                    scaling_factor_      = 1.0f;
    float                    base_scaling_factor_ = 1.0f;
    bool                     enable_caching_      = false;
    std::vector<std::string> localization_directory_list_;
    std::vector<std::string> gui_directory_list_;

    // Implementations
    std::unique_ptr<input::source> input_source_;
    std::unique_ptr<renderer>      renderer_;

    // IO
    std::unique_ptr<input::window>           window_;
    std::unique_ptr<input::dispatcher>       input_dispatcher_;
    std::unique_ptr<input::world_dispatcher> world_input_dispatcher_;
    std::unique_ptr<event_emitter>           event_emitter_;

    // UI state
    std::unique_ptr<factory>        factory_;
    std::unique_ptr<localizer>      localizer_;
    std::unique_ptr<sol::state>     lua_;
    utils::owner_ptr<root>          root_;
    utils::owner_ptr<virtual_root>  virtual_root_;
    std::unique_ptr<addon_registry> addon_registry_;

    bool is_loaded_          = false;
    bool reload_ui_flag_     = false;
    bool close_ui_flag_      = false;
    bool is_first_iteration_ = true;
};

} // namespace lxgui::gui

#endif
