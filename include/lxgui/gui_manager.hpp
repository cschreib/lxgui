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
    struct vertex;

    /// Manages the user interface
    class manager : private event_manager, public event_receiver
    {
    private :

        template <class T>
        static utils::owner_ptr<frame> create_new_frame(manager& pMgr)
        {
            return utils::make_owned<T>(pMgr);
        }

        template <class T>
        static utils::owner_ptr<layered_region> create_new_layered_region(manager& pMgr)
        {
            return utils::make_owned<T>(pMgr);
        }

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

        /// Creates a new uiobject.
        /** \param sClassName The class of the uiobject (Frame, fontString, Button, ...)
        *   \return The new uiobject
        *   \note This is a low level function; the returned object only has the bare
        *         minimum initialization. Use create_root_frame() or frame::create_child()
        *         to get a fully-functional frame object, or frame::create_region() for
        *         region objects.
        */
        utils::owner_ptr<uiobject> create_uiobject(const std::string& sClassName);

        /// Creates a new frame.
        /** \param sClassName The sub class of the frame (Button, ...)
        *   \return The new frame
        *   \note This is a low level function; the returned frame only has the bare
        *         minimum initialization. Use create_root_frame() or frame::create_child()
        *         to get a fully-functional frame object.
        */
        utils::owner_ptr<frame> create_frame(const std::string& sClassName);

        /// Creates a new layered_region.
        /** \param sClassName The sub class of the layered_region (FontString or texture)
        *   \return The new layered_region
        *   \note This is a low level function; the returned region only has the bare
        *         minimum initialization. Use frame::create_region() to get a fully-functional
        *         region object.
        */
        utils::owner_ptr<layered_region> create_layered_region(
            const std::string& sClassName);

        /// Adds an uiobject to be handled by this manager.
        /** \param pObj The object to add
        *   \return 'false' if the name of the widget was already taken
        */
        bool add_uiobject(utils::observer_ptr<uiobject> pObj);

         /// Removes an uiobject from this manager.
        /** \param pObj The object to remove
        *   \note Called automatically by uiobject destructor.
        */
        void remove_uiobject(const utils::observer_ptr<uiobject>& pObj);

         /// Removes a frame from this manager.
        /** \param pObj The frame to remove
        *   \note Called automatically by frame destructor.
        */
        void remove_frame(const utils::observer_ptr<frame>& pObj);

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

        /// Checks if the UI is currently being loaded.
        /** \return 'true' if the UI is currently being loaded
        */
        bool is_loading_ui() const;

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

        /// Registers a new frame type.
        /** \note Set the first template argument as the C++ type of this frame.
        */
        template<typename frame_type,
            typename enable = typename std::enable_if<std::is_base_of<gui::frame, frame_type>::value>::type>
        void register_frame_type()
        {
            lCustomFrameList_[frame_type::CLASS_NAME] = &create_new_frame<frame_type>;
            frame_type::register_on_lua(*pLua_);
        }

        /// Registers a new frame type.
        /** \param mFactoryFunction A function that creates new frames of this type. Must take a
        *                           lxgui::gui::manager& as first and only argument, and return
        *                           a utils::owner_ptr<frame>.
        *   \note Set the first template argument as the C++ type of this frame.
        */
        template<typename frame_type, typename function_type,
            typename enable = typename std::enable_if<std::is_base_of<gui::frame, frame_type>::value>::type>
        void register_frame_type(function_type&& mFactoryFunction)
        {
            lCustomFrameList_[frame_type::CLASS_NAME] = std::forward<function_type>(mFactoryFunction);
            frame_type::register_on_lua(*pLua_);
        }

        /// Registers a new layered_region type.
        /** \note Set the first template argument as the C++ type of this layered_region.
        */
        template<typename region_type,
            typename enable = typename std::enable_if<std::is_base_of<gui::layered_region, region_type>::value>::type>
        void register_region_type()
        {
            lCustomRegionList_[region_type::CLASS_NAME] = &create_new_layered_region<region_type>;
            region_type::register_on_lua(*pLua_);
        }

        /// Registers a new layered_region type.
        /** \param mFactoryFunction A function that creates new layered regions of this type. Must
        *                           take a lxgui::gui::manager& as first and only argument, and
        *                           return a utils::owner_ptr<layered_region>.
        *   \note Set the first template argument as the C++ type of this layered_region.
        */
        template<typename region_type, typename function_type,
            typename enable = typename std::enable_if<std::is_base_of<gui::layered_region, region_type>::value>::type>
        void register_region_type(function_type&& mFactoryFunction)
        {
            lCustomRegionList_[region_type::CLASS_NAME] = std::forward<function_type>(mFactoryFunction);
            region_type::register_on_lua(*pLua_);
        }

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

        void clear_focussed_frame_();
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

        string_map<std::function<utils::owner_ptr<frame>(manager&)>>          lCustomFrameList_;
        string_map<std::function<utils::owner_ptr<layered_region>(manager&)>> lCustomRegionList_;

        std::unique_ptr<localizer> pLocalizer_;
        std::unique_ptr<renderer>  pRenderer_;
    };
}
}


#endif
