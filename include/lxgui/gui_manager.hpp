#ifndef LXGUI_GUI_MANAGER_HPP
#define LXGUI_GUI_MANAGER_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_eventemitter.hpp"
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_anchor.hpp"
#include "lxgui/input_keys.hpp"

#include <lxgui/utils_observer.hpp>

#include <string>
#include <vector>
#include <functional>
#include <memory>

/** \cond INCLUDE_INTERNALS_IN_DOC
*/
namespace sol {
    class state;
}
/** \endcond
*/

namespace lxgui {

namespace input {
    class source;
    class manager;
}

namespace gui
{
    class uiobject;
    class frame;
    class focus_frame;
    class renderer;
    class localizer;
    class factory;
    class uiroot;
    class virtual_uiroot;
    class addon_registry;
    class keybinder;

    /// Manages the user interface
    class manager : private event_emitter, public event_receiver
    {
    public :

        /// Constructor.
        /** \param mBlock       The owner pointer control block
        *   \param pInputSource The input source to use
        *   \param pRenderer    The renderer implementation
        */
        manager(utils::control_block& mBlock,
            std::unique_ptr<input::source> pInputSource,
            std::unique_ptr<renderer> pRenderer);

        /// Destructor.
        ~manager() override;

        manager(const manager& mMgr) = delete;
        manager(manager&& mMgr) = delete;
        manager& operator = (const manager& mMgr) = delete;
        manager& operator = (manager&& mMgr) = delete;

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
        void set_interface_scaling_factor(float fScalingFactor);

        /// Returns the current UI scaling factor.
        /** \return The current UI scaling factor
        *   \see set_interface_scaling_factor()
        */
        float get_interface_scaling_factor() const;

        /// Enables or disables interface caching.
        /** \param bEnableCaching 'true' to enable, 'false' to disable
        *   \see toggle_caching()
        */
        void enable_caching(bool bEnableCaching);

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
        /** \param sDirectory The new directory
        *   \note If the UI is already loaded, this change will only take effect after
        *         the UI is reloaded, see reload_ui().
        */
        void add_addon_directory(const std::string& sDirectory);

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
        void register_lua_glues(std::function<void(gui::manager&)> pLuaRegs);

        /// Prints debug informations in the log file.
        /** \note Calls uiobject::serialize().
        */
        std::string print_ui() const;

        /// Loads the UI.
        /** \note Creates the Lua state and loads addon files (if any).
        *         Calling this function if the UI is already loaded will have no effect.
        *         If your intent is to re-load the UI, use reload_ui() instead.
        */
        void load_ui();

        /// Closes the UI (at the end of the current or next update_ui()).
        /** \note Because the actual re-loading is deferred to the end of the current update_ui() call,
        *         or to the end of the next update_ui() call, it is safe to call this function at any
        *         time. If you need to close the UI without delay, use close_ui_now().
        */
        void close_ui();

        /// Closes the UI (immediately).
        /** \note All widgets will be deleted, and the Lua state will be closed.
        *   \warning Do not call this function while the manager is running update_ui()
        *            (i.e., do not call this directly from a frame's callback, C++ or Lua).
        */
        void close_ui_now();

        /// Closes the UI and load it again (at the end of the current or next update_ui()).
        /** \note Because the actual re-loading is deferred to the end of the current update_ui() call,
        *         or to the end of the next update_ui() call, it is safe to call this function at any
        *         time. If you need to reload the UI without delay, use reload_ui_now().
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

        /// Updates this manager and its widgets.
        /** \param fDelta The time elapsed since the last call
        */
        void update_ui(float fDelta);

        /// Called whenever an Event occurs.
        /** \param mEvent The Event which has occured
        */
        void on_event(const event& mEvent) override;

        /// Ask this manager for movement management.
        /** \param pObj        The object to move
        *   \param pAnchor     The reference anchor
        *   \param mConstraint The constraint axis if any
        *   \param mApplyConstraintFunc Optional function to implement further constraints
        *   \note Movement is handled by the manager itself, you don't
        *         need to do anything.
        */
        void start_moving(
            utils::observer_ptr<uiobject> pObj, anchor* pAnchor = nullptr,
            constraint mConstraint = constraint::NONE,
            std::function<void()> mApplyConstraintFunc = nullptr
        );

        /// Stops movement for the given object.
        /** \param mObj The object to stop moving
        */
        void stop_moving(const uiobject& mObj);

        /// Checks if the given object is allowed to move.
        /** \param mObj The object to check
        *   \return 'true' if the given object is allowed to move
        */
        bool is_moving(const uiobject& mObj) const;

        /// Starts resizing a widget.
        /** \param pObj   The object to resize
        *   \param mPoint The sizing point
        *   \note Resizing is handled by the manager itself, you don't
        *         need to do anything.
        */
        void start_sizing(utils::observer_ptr<uiobject> pObj, anchor_point mPoint);

        /// Stops sizing for the given object.
        /** \param mObj The object to stop sizing
        */
        void stop_sizing(const uiobject& mObj);

        /// Checks if the given object is allowed to be resized.
        /** \param mObj The object to check
        *   \return 'true' if the given object is allowed to be resized
        */
        bool is_sizing(const uiobject& mObj) const;

        /// Returns the frame under the mouse.
        /** \return The frame under the mouse (nullptr if none)
        */
        const utils::observer_ptr<frame>& get_hovered_frame() const;

        /// Notifies this manager that it should update the hovered frame.
        void notify_hovered_frame_dirty();

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
        const renderer& get_renderer() const { return *pRenderer_; }

        /// Returns the renderer implementation.
        /** \return The renderer implementation
        */
        renderer& get_renderer() { return *pRenderer_; }

        /// Returns the gui event emitter.
        /** \return The gui event emitter
        */
        const event_emitter& get_event_emitter() const { return *this; }

        /// Returns the gui event emitter.
        /** \return The gui event emitter
        */
        event_emitter& get_event_emitter() { return *this; }

        /// Returns the input manager associated to this gui.
        /** \return The input manager associated to this gui
        */
        const input::manager& get_input_manager() const { return *pInputManager_; }

        /// Returns the input manager associated to this gui.
        /** \return The input manager associated to this gui
        */
        input::manager& get_input_manager() { return *pInputManager_; }

        /// Returns the object used for localizing strings.
        /** \return The current localizer
        */
        localizer& get_localizer() { return *pLocalizer_; }

        /// Returns the object used for localizing strings.
        /** \return The current localizer
        */
        const localizer& get_localizer() const { return *pLocalizer_; }

        /// Returns the UI root object, which contains root frames.
        /** \return The root object
        */
        uiroot& get_root() { return *pRoot_; }

        /// Returns the UI root object, which contains root frames.
        /** \return The root object
        */
        const uiroot& get_root() const { return *pRoot_; }

        /// Returns the UI root object, which contains root frames.
        /** \return The root object
        */
        virtual_uiroot& get_virtual_root() { return *pVirtualRoot_; }

        /// Returns the UI root object, which contains root frames.
        /** \return The root object
        */
        const virtual_uiroot& get_virtual_root() const { return *pVirtualRoot_; }

        /// Returns the UI object factory, which is used to create new objects.
        /** \return The factory object
        */
        factory& get_factory() { return *pFactory_; }

        /// Returns the UI object factory, which is used to create new objects.
        /** \return The factory object
        */
        const factory& get_factory() const { return *pFactory_; }

        /// Returns the addon registry, which keeps track of loaded addons.
        /** \return The registry object
        */
        addon_registry* get_addon_registry() { return pAddOnRegistry_.get(); }

        /// Returns the addon registry, which keeps track of loaded addons.
        /** \return The registry object
        */
        const addon_registry* get_addon_registry() const { return pAddOnRegistry_.get(); }

        /// Return an observer pointer to 'this'.
        /** \return A new observer pointer pointing to 'this'.
        */
        utils::observer_ptr<const manager> observer_from_this() const
        {
            return utils::static_pointer_cast<const manager>(event_receiver::observer_from_this());
        }

        /// Return an observer pointer to 'this'.
        /** \return A new observer pointer pointing to 'this'.
        */
        utils::observer_ptr<manager> observer_from_this()
        {
            return utils::static_pointer_cast<manager>(event_receiver::observer_from_this());
        }

    private :

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

        void clear_hovered_frame_();
        void update_hovered_frame_();
        void set_hovered_frame_(utils::observer_ptr<frame> pFrame,
            const vector2f& mMousePos = vector2f::ZERO);

        // Persistent state
        float fScalingFactor_ = 1.0f;
        float fBaseScalingFactor_ = 1.0f;
        bool  bEnableCaching_ = false;
        std::function<void(gui::manager&)> pLuaRegs_;
        std::vector<std::string>           lGUIDirectoryList_;

        // Implementations
        utils::owner_ptr<input::manager> pInputManager_;
        std::unique_ptr<renderer>        pRenderer_;

        // UI state
        std::unique_ptr<factory>         pFactory_;
        std::unique_ptr<localizer>       pLocalizer_;
        std::unique_ptr<sol::state>      pLua_;
        utils::owner_ptr<uiroot>         pRoot_;
        utils::owner_ptr<virtual_uiroot> pVirtualRoot_;
        std::unique_ptr<addon_registry>  pAddOnRegistry_;
        utils::owner_ptr<keybinder>      pKeybinder_;

        bool bLoaded_ = false;
        bool bReloadUI_ = false;
        bool bCloseUI_ = false;
        bool bFirstIteration_ = true;
        bool bUpdating_ = false;

        utils::observer_ptr<frame> pHoveredFrame_ = nullptr;

        utils::observer_ptr<uiobject> pMovedObject_ = nullptr;
        utils::observer_ptr<uiobject> pSizedObject_ = nullptr;
        vector2f                      mMouseMovement_;

        anchor*    pMovedAnchor_ = nullptr;
        vector2f   mMovementStartPosition_;
        constraint mConstraint_ = constraint::NONE;
        std::function<void()> mApplyConstraintFunc_;

        vector2f mResizeStart_;
        bool bResizeWidth_ = false;
        bool bResizeHeight_ = false;
        bool bResizeFromRight_ = false;
        bool bResizeFromBottom_ = false;
    };
}
}


#endif
