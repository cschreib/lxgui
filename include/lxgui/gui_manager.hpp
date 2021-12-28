#ifndef LXGUI_GUI_MANAGER_HPP
#define LXGUI_GUI_MANAGER_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_eventmanager.hpp"
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_addon.hpp"
#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_uiroot.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/input_keys.hpp"

#include <lxgui/utils_exception.hpp>
#include <lxgui/utils_view.hpp>
#include <lxgui/utils_observer.hpp>

#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <tuple>
#include <array>
#include <functional>
#include <memory>

namespace sol {
    class state;
}

namespace lxgui {

namespace input {
    class source;
    class manager;
}

namespace gui
{
    class region;
    class layered_region;
    class frame;
    class focus_frame;
    class renderer;
    class registry;
    class virtual_registry;
    class localizer;
    class factory;
    struct vertex;

    /// Manages the user interface
    class manager : private event_manager, public event_receiver
    {
    public :

        /// Constructor.
        /** \param pInputSource The input source to use
        *   \param pRenderer    The renderer implementation
        */
        manager(std::unique_ptr<input::source> pInputSource,
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

        /// Adds a new directory to be parsed for UI addons.
        /** \param sDirectory The new directory
        */
        void add_addon_directory(const std::string& sDirectory);

        /// Clears the addon directory list.
        /** \note This is usefull whenever you need to reload a
        *         completely different UI (for example, when switching
        *         from your game's main menu to the real game).
        */
        void clear_addon_directory_list();

        /// Prints in the log several performance statistics.
        void print_statistics();

        /// Prints debug informations in the log file.
        /** \note Calls uiobject::serialize().
        */
        std::string print_ui() const;

        /// Returns the addon that is being parsed.
        /** \return The addon that is being parsed
        */
        const addon* get_current_addon();

        /// Sets the current addon.
        /** \param pAddOn The current addon
        *   \note The current addon is used when parsing file names.
        *         See parse_file_name() for more information. For uiobjects
        *         that are created manually after the loading stage, one
        *         needs to specify the addon that is actually creating the
        *         widget, and that is the purpose of this method.
        *         It is called by frame automatically, before each call to
        *         handler functions.
        */
        void set_current_addon(const addon* pAddOn);

        /// Reads a file address and completes it to make a working address.
        /** \param sFileName The raw file name
        *   \return The modified file name
        *   \note All file names are relative to the Engine's executable path,
        *         but sometimes you'd like to use a path that is relative to
        *         your addon directory for example. To do so, you need to append
        *         "|" in front of your file name.
        */
        std::string parse_file_name(const std::string& sFileName) const;

        /// Binds some Lua code to a key.
        /** \param uiKey      The key to bind
        *   \param sLuaString The Lua code that will be executed
        */
        void set_key_binding(input::key uiKey, const std::string& sLuaString);

        /// Binds some Lua code to a key.
        /** \param uiKey      The key to bind
        *   \param uiModifier The modifier key (shift, ctrl, ...)
        *   \param sLuaString The Lua code that will be executed
        */
        void set_key_binding(input::key uiKey, input::key uiModifier, const std::string& sLuaString);

        /// Binds some Lua code to a key.
        /** \param uiKey       The key to bind
        *   \param uiModifier1 The first modifier key (shift, ctrl, ...)
        *   \param uiModifier2 The second modifier key (shift, ctrl, ...)
        *   \param sLuaString  The Lua code that will be executed
        */
        void set_key_binding(
            input::key uiKey, input::key uiModifier1, input::key uiModifier2,
            const std::string& sLuaString
        );

        /// Unbinds a key.
        /** \param uiKey      The key to unbind
        *   \param uiModifier1 The first modifier key (shift, ctrl, ...), default is no modifier
        *   \param uiModifier2 The second modifier key (shift, ctrl, ...), default is no modified
        */
        void remove_key_binding(
            input::key uiKey, input::key uiModifier1 = input::key::K_UNASSIGNED,
            input::key uiModifier2 = input::key::K_UNASSIGNED
        );

        /// Returns the GUI Lua state (sol wrapper).
        /** \return The GUI Lua state
        */
        sol::state& get_lua();

        /// Returns the GUI Lua state (sol wrapper).
        /** \return The GUI Lua state
        */
        const sol::state& get_lua() const;

        /// Creates the lua::state that will be used to communicate with the GUI.
        /** \param pLuaRegs Some code that will get exectued each time the lua
        *                   state is created
        *   \note This function is usefull if you need to create additionnal
        *         resources on the Lua state before the GUI files are loaded.
        *         You need to do this inside the provided argument function,
        *         because this code will need to be called again in case the GUI
        *         is reloaded (see reload_ui()).
        *         Else, you can simply use load_ui().
        *   \warning Do not call this function while the manager is running update()
        *           (i.e., do not call this directly from a frame's callback, C++ or Lua).
        */
        void create_lua(std::function<void(gui::manager&)> pLuaRegs = nullptr);

        /// Reads GUI files in the directory list.
        /** \note See add_addon_directory().
        *   \note See load_ui().
        *   \note See create_lua().
        *   \warning Do not call this function while the manager is running update()
        *            (i.e., do not call this directly from a frame's callback, C++ or Lua).
        */
        void read_files();

        /// Loads the UI.
        /** \note Calls create_lua() then read_files().
        *   \warning Do not call this function while the manager is running update()
        *            (i.e., do not call this directly from a frame's callback, C++ or Lua).
        */
        void load_ui();

        /// Closes the UI, deletes widgets.
        /** \warning Do not call this function while the manager is running update()
        *            (i.e., do not call this directly from a frame's callback, C++ or Lua).
        */
        void close_ui();

        /// Closes the UI and load it again (at the end of the current or next update()).
        /** \note Because the actual re-loading is deferred to the end of the current update() call,
        *         or to the end of the next update() call, it is safe to call this function at any
        *         time. If you need to reload the UI without delay, use reload_ui_now().
        */
        void reload_ui();

        /// Closes the UI and load it again (immediately).
        /** \note Calls close_ui() then load_ui().
        *   \warning Do not call this function while the manager is running update()
        *            (i.e., do not call this directly from a frame's callback, C++ or Lua).
        */
        void reload_ui_now();

        /// Tells the rendering back-end to start rendering into a new target.
        /** \param pTarget The target to render to (nullptr to render to the screen)
        */
        void begin(std::shared_ptr<render_target> pTarget = nullptr) const;

        /// Tells the rendering back-end we are done rendering on the current target.
        /** \note For most back-ends, this is when the rendering is actually
        *         done, so do not forget to call it even if it appears to do nothing.
        */
        void end() const;

        /// Renders the UI into the current render target.
        void render_ui() const;

        /// Checks if the UI has been loaded.
        /** \return 'true' if the UI has being loaded
        */
        bool is_loaded() const;

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

        /// Returns the accumulated mouse movement.
        /** \return The accumulated mouse movement
        *   \note This vector is reset to zero whenever start_moving() or
        *         start_sizing() is called.
        */
        const vector2f& get_movement() const;

        /// Tells this manager an object has moved.
        void notify_object_moved();

        /// Enables/disables input response for all widgets.
        /** \param bEnable 'true' to enable input
        *   \note See toggle_input() and is_input_enabled().
        */
        void enable_input(bool bEnable);

        /// Toggles input response for all widgets.
        /** \note Enabled by default.
        *   \note See is_input_enabled().
        */
        void toggle_input();

        /// Checks if input response is enabled for all widgets.
        /** \return 'true' if input response is enabled
        *   \note All widgets must call this function and check
        *         its return value before reacting to input events.
        *   \note See toggle_input().
        */
        bool is_input_enabled() const;

        /// Returns the frame under the mouse.
        /** \return The frame under the mouse (nullptr if none)
        */
        const utils::observer_ptr<frame>& get_hovered_frame();

        /// Notifies this manager that it should update the hovered frame.
        void notify_hovered_frame_dirty();

        /// Asks this manager for focus.
        /** \param pFocusFrame The focus_frame requesting focus
        */
        void request_focus(utils::observer_ptr<focus_frame> pFocusFrame);

        /// updates this manager and its widgets.
        /** \param fDelta The time elapsed since the last call
        */
        void update(float fDelta);

        /// Called whenever an Event occurs.
        /** \param mEvent The Event which has occured
        */
        void on_event(const event& mEvent) override;

        /// Returns the renderer implementation.
        /** \return The renderer implementation
        */
        const renderer& get_renderer() const { return *pRenderer_; }

        /// Returns the renderer implementation.
        /** \return The renderer implementation
        */
        renderer& get_renderer() { return *pRenderer_; }

        /// Returns the gui event manager.
        /** \return The gui event manager
        */
        const event_manager& get_event_manager() const { return *this; }

        /// Returns the gui event manager.
        /** \return The gui event manager
        */
        event_manager& get_event_manager() { return *this; }

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
        frame_container& get_virtual_root() { return *pVirtualRoot_; }

        /// Returns the UI root object, which contains root frames.
        /** \return The root object
        */
        const frame_container& get_virtual_root() const { return *pVirtualRoot_; }

        /// Returns the UI object registry, which keeps track of all objects in the UI.
        /** \return The registry object
        */
        registry& get_registry() { return *pObjectRegistry_; }

        /// Returns the UI object registry, which keeps track of all objects in the UI.
        /** \return The registry object
        */
        const registry& get_registry() const { return *pObjectRegistry_; }

        /// Returns the UI virtual object registry, which keeps track of all virtual objects in the UI.
        /** \return The registry object
        */
        virtual_registry& get_virtual_registry() { return *pVirtualObjectRegistry_; }

        /// Returns the UI virtual object registry, which keeps track of all virtual objects in the UI.
        /** \return The registry object
        */
        const virtual_registry& get_virtual_registry() const { return *pVirtualObjectRegistry_; }

        /// Returns the UI object factory, which is used to create new objects.
        /** \return The factory object
        */
        factory& get_factory() { return *pFactory_; }

        /// Returns the UI object factory, which is used to create new objects.
        /** \return The factory object
        */
        const factory& get_factory() const { return *pFactory_; }

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

        void load_addon_toc_(const std::string& sAddOnName, const std::string& sAddOnDirectory);
        void load_addon_files_(addon* pAddOn);
        void load_addon_directory_(const std::string& sDirectory);

        void save_variables_(const addon* pAddOn);
        std::string serialize_global_(const std::string& sVariable) const;

        void clear_hovered_frame_();
        void update_hovered_frame_();
        void set_hovered_frame_(utils::observer_ptr<frame> pFrame,
            const vector2f& mMousePos = vector2f::ZERO);

        void parse_layout_file_(const std::string& sFile, addon* pAddOn);

        std::string sUIVersion_ = "0001";
        float       fScalingFactor_ = 1.0f;
        float       fBaseScalingFactor_ = 1.0f;

        std::unique_ptr<sol::state>        pLua_;
        std::function<void(gui::manager&)> pLuaRegs_;

        bool bClosed_ = true;
        bool bLoadingUI_ = false;
        bool bReloadUI_ = false;
        bool bFirstIteration_ = true;
        bool bUpdating_ = false;

        bool                            bInputEnabled_ = true;
        std::unique_ptr<input::manager> pInputManager_;

        template<typename T>
        using key_map = std::unordered_map<input::key,T>;
        template<typename T>
        using string_map = std::unordered_map<std::string,T>;

        key_map<key_map<key_map<std::string>>> lKeyBindingList_;

        std::unique_ptr<registry>         pObjectRegistry_;
        std::unique_ptr<virtual_registry> pVirtualObjectRegistry_;

        utils::owner_ptr<uiroot> pRoot_;
        utils::owner_ptr<frame_container> pVirtualRoot_;

        std::vector<std::string>      lGUIDirectoryList_;
        const addon*                  pCurrentAddOn_ = nullptr;
        string_map<string_map<addon>> lAddOnList_;

        bool                             bObjectMoved_ = false;
        utils::observer_ptr<frame>       pHoveredFrame_ = nullptr;
        bool                             bUpdateHoveredFrame_ = false;
        utils::observer_ptr<focus_frame> pFocusedFrame_ = nullptr;

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

        std::unique_ptr<factory> pFactory_;

        std::unique_ptr<localizer> pLocalizer_;
        std::unique_ptr<renderer>  pRenderer_;
    };
}
}


#endif
