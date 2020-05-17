#ifndef GUI_MANAGER_HPP
#define GUI_MANAGER_HPP

#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_sprite.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/input_keys.hpp"

#include <lxgui/utils_exception.hpp>
#include <lxgui/utils_refptr.hpp>
#include <lxgui/utils_wptr.hpp>

#include <string>
#include <vector>
#include <map>
#include <array>
#include <functional>

namespace lxgui {
namespace lua {
    class state;
}

namespace input {
    class source_impl;
    class manager;
}

namespace gui
{
    class uiobject;
    class region;
    class layered_region;
    class frame;
    class focus_frame;
    class sprite;
    class font;
    class color;
    struct quad;
    struct vertex;

    class renderer_impl;

    /// Exception to be thrown by GUI code.
    /** \note These exceptions should always be handled.<br>
    *         The GUI is not a critical part of the Engine, so
    *         whatever happens there <b>musn't</b> close the
    *         program.
    */
    class exception : public utils::exception
    {
    public :

        explicit exception(const std::string& sMessage) : utils::exception(sMessage)
        {
        }

        exception(const std::string& sClassName, const std::string& sMessage) : utils::exception(sClassName, sMessage)
        {
        }
    };

    /// A piece of the user interface
    struct addon
    {
        std::string sName;
        std::string sVersion;
        std::string sUIVersion;
        std::string sAuthor;

        bool bEnabled = true;

        std::string sMainDirectory;
        std::string sDirectory;

        std::vector<std::string> lFileList;
        std::vector<std::string> lSavedVariableList;
    };

    enum class frame_strata
    {
        PARENT,
        BACKGROUND,
        LOW,
        MEDIUM,
        HIGH,
        DIALOG,
        FULLSCREEN,
        FULLSCREEN_DIALOG,
        TOOLTIP
    };

    /// Contains frame
    struct level
    {
        std::vector<frame*> lFrameList;
    };

    /// Contains level
    struct strata
    {
        frame_strata                 mStrata = frame_strata::PARENT;
        std::map<int, level>         lLevelList;
        mutable bool                 bRedraw = true;
        utils::refptr<render_target> pRenderTarget;
        sprite                       mSprite;
        mutable uint                 uiRedrawCount = 0u;
    };

    /// Manages the user interface
    class manager : public event_receiver
    {
    private :

        template <class T>
        static frame* create_new_frame(manager* pMgr)
        {
            return new T(pMgr);
        }

        template <class T>
        static layered_region* create_new_layered_region(manager* pMgr)
        {
            return new T(pMgr);
        }

    public :

        /// Constructor.
        /** \param pInputSource   The input source to use
        *   \param sLocale        The name of the game locale ("enGB", ...)
        *   \param uiScreenWidth  The width of the screen
        *   \param uiScreenHeight The height of the screen
        *   \param pImpl          The renderer implementation
        */
        manager(std::unique_ptr<input::source_impl> pInputSource, const std::string& sLocale,
                uint uiScreenWidth, uint uiScreenHeight, std::unique_ptr<renderer_impl> pImpl);

        /// Destructor.
        ~manager() override;

        manager(const manager& mMgr) = delete;
        manager(manager&& mMgr) = delete;
        manager& operator = (const manager& mMgr) = delete;
        manager& operator = (manager&& mMgr) = delete;

        /// Returns the "screen" width.
        /** \return The screen width
        *   \note This is not necessarily the real screen width.
        *         If the GUI is rendered into a small render target
        *         that takes only a portion of the real screen, then
        *         this function returns the width of the render target.
        */
        uint get_screen_width() const;

        /// Returns the "screen" height.
        /** \return The screen height
        *   \note This is not necessarily the real screen height.
        *         If the GUI is rendered into a small render target
        *         that takes only a portion of the real screen, then
        *         this function returns the height of the render target.
        */
        uint get_screen_height() const;

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

        /// Checks the provided string is suitable for naming a widget.
        /** \param sName The string to test
        *   \return 'true' if the provided string can be the name of a widget
        */
        bool check_uiobject_name(const std::string& sName) const;

        /// Creates a new uiobject.
        /** \param sClassName The class of the uiobject (Frame, fontString, Button, ...)
        *   \return The new uiobject
        *   \note You have the responsability to detroy and initialize the created
        *         uiobject by yourself.
        */
        uiobject* create_uiobject(const std::string& sClassName);

        /// Creates a new frame.
        /** \param sClassName The sub class of the frame (Button, ...)
        *   \return The new frame
        *   \note You have the responsability to detroy and initialize the created
        *         frame by yourself.
        */
        frame* create_frame(const std::string& sClassName);

        /// Creates a new frame, ready for use.
        /** \param sClassName   The sub class of the frame (Button, ...)
        *   \param sName        The name of this frame
        *   \param pParent      The parent of the created frame (can be nullptr)
        *   \param sInheritance The name of the frame that should be inherited
        *                       (empty if none)
        *   \return The new frame
        *   \note You don't have the responsability to detroy the created
        *         frame, it will be done automatically, either by its parent
        *         or by this manager.
        *   \note This function takes care of the basic initializing : the
        *         frame is directly usable.
        */
        frame* create_frame(
            const std::string& sClassName, const std::string& sName,
            frame* pParent = nullptr, const std::string& sInheritance = ""
        );

        /// Creates a new frame, ready for use.
        /** \param sName        The name of this frame
        *   \param pParent      The parent of the created frame (can be nullptr)
        *   \param sInheritance The name of the frame that should be inherited
        *                       (empty if none)
        *   \return The new frame
        *   \note You don't have the responsability to detroy the created
        *         frame, it will be done automatically, either by its parent
        *         or by this manager.
        *   \note This function takes care of the basic initializing : the
        *         frame is directly usable.
        */
        template<typename frame_type, typename enable = typename std::enable_if<std::is_base_of<gui::frame, frame_type>::value>::type>
        frame_type* create_frame(const std::string& sName, frame* pParent = nullptr, const std::string& sInheritance = "")
        {
            return dynamic_cast<frame_type*>(create_frame(frame_type::CLASS_NAME, sName, pParent, sInheritance));
        }

        /// Creates a new layered_region.
        /** \param sClassName The sub class of the layered_region (FontString or texture)
        *   \return The new layered_region
        *   \note You have the responsability to detroy and initialize the created
        *         layered_region by yourself.
        */
        layered_region* create_layered_region(const std::string& sClassName);

        /// Creates a new sprite.
        /** \param pMat The material with which to create the sprite
        *   \return The new sprite
        */
        sprite create_sprite(utils::refptr<material> pMat) const;

        /// Creates a new sprite.
        /** \param pMat    The material with which to create the sprite
        *   \param fWidth  The width of the sprite
        *   \param fHeight The height of the sprite
        *   \note If the width and height you provide are smaller than
        *         the texture's ones, the texture will be cut on the right
        *         and bottom edges.<br>
        *         However, if they are larger than the texture's one, the
        *         texture will be tiled.
        *   \return The new sprite
        */
        sprite create_sprite(utils::refptr<material> pMat, float fWidth, float fHeight) const;

        /// Creates a new sprite.
        /** \param pMat    The material with which to create the sprite
        *   \param fU      The top left corner of the sprite in the material
        *   \param fV      The top left corner of the sprite in the material
        *   \param fWidth  The width of the sprite
        *   \param fHeight The height of the sprite
        *   \note If the width and height you provide are smaller than
        *         the texture's ones, the texture will be cut on the right
        *         and bottom edges.<br>
        *         However, if they are larger than the texture's one, the
        *         texture will be tiled.
        *   \return The new sprite
        */
        sprite create_sprite(utils::refptr<material> pMat,
            float fU, float fV, float fWidth, float fHeight) const;

        /// Creates a new material from a texture file.
        /** \param sFileName The name of the file
        *   \param mFilter   The filtering to apply to the texture
        *   \return The new material
        *   \note Supported texture formats are defined by implementation.
        *         The gui library is completely unaware of this.
        */
        utils::refptr<material> create_material(const std::string& sFileName,
            material::filter mFilter = material::filter::NONE) const;

        /// Creates a new material from a plain color.
        /** \param mColor The color to use
        *   \return The new material
        */
        utils::refptr<material> create_material(const color& mColor) const;

        /// Creates a new material from a render target.
        /** \param pRenderTarget The render target from which to read the pixels
        *   \return The new material
        */
        utils::refptr<material> create_material(utils::refptr<render_target> pRenderTarget) const;

        /// Creates a new render target.
        /** \param uiWidth  The width of the render target
        *   \param uiHeight The height of the render target
        */
        utils::refptr<render_target> create_render_target(uint uiWidth, uint uiHeight) const;

        /// Creates a new font.
        /** \param sFontFile The file from which to read the font
        *   \param uiSize    The requested size of the characters (in points)
        *   \note Even though the gui has been designed to use vector fonts files
        *         (such as .ttf or .otf font formats), nothing prevents the implementation
        *         from using any other font type, including bitmap fonts.
        */
        utils::refptr<font> create_font(const std::string& sFontFile, uint uiSize) const;

        /// Adds an uiobject to be handled by this manager.
        /** \param pObj The object to add
        *   \return 'false' if the name of the widget was already taken
        */
        bool add_uiobject(uiobject* pObj);

        /// Removes an uiobject from this manager.
        /** \param pObj The object to remove
        *   \note Only call this function if you have to delete an object
        *         manually (instead of waiting for the UI to close).
        */
        void remove_uiobject(uiobject* pObj);

        /// Tells this manager that the provided object now has a parent.
        /** \param pObj The object that has recently been given a parent
        *   \note When the GUI closes, the manager deletes all objects
        *         that have no parents first, which then delete all their
        *         children. This function is just here to prevent double
        *         deleting of widgets that once were orphans.
        */
        void notify_object_has_parent(uiobject* pObj);

        /// Returns the uiobject associated with the given ID.
        /** \param uiID The unique ID representing the widget
        *   \return The uiobject associated with the given ID
        */
        const uiobject* get_uiobject(uint uiID) const;

        /// Returns the uiobject associated with the given ID.
        /** \param uiID The unique ID representing the widget
        *   \return The uiobject associated with the given ID
        */
        uiobject* get_uiobject(uint uiID);

        /// Returns the uiobject associated with the given name.
        /** \param sName    The name of the widget you're after
        *   \param bVirtual 'true' to search for a virtual frame
        *   \return The uiobject associated with the given name
        */
        const uiobject* get_uiobject_by_name(const std::string& sName, bool bVirtual = false) const;

        /// Returns the uiobject associated with the given name.
        /** \param sName    The name of the widget you're after
        *   \param bVirtual 'true' to search for a virtual frame
        *   \return The uiobject associated with the given name
        */
        uiobject* get_uiobject_by_name(const std::string& sName, bool bVirtual = false);

        /// Tells this manager it must rebuild its strata list.
        void fire_build_strata_list();

        /// Prints in the log several performance statistics.
        void print_statistics();

        /// Prints debug informations in the log file.
        /** \note Calls uiobject::serialize().
        */
        std::string print_ui() const;

        /// Returns the addon that is being parsed.
        /** \return The addon that is being parsed
        */
        addon* get_current_addon();

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
        void set_current_addon(addon* pAddOn);

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

        /// Returns the GUI Lua state.
        /** \return The GUI Lua state
        */
        lua::state* get_lua();

        /// Returns the GUI Lua state.
        /** \return The GUI Lua state
        */
        const lua::state* get_lua() const;

        /// Creates the lua::State that will be used to communicate with the GUI.
        /** \param pLuaRegs Some code that will get exectued each time the lua
        *                   state is created
        *   \note This function is usefull if you need to create additionnal
        *         resources on the lua::State before the GUI files are loaded.
        *         You need to do this inside the provided argument function,
        *         because this code will need to be called again in case the GUI
        *         is reloaded (see reload_ui()).
        *         Else, you can simply use load_ui().
        */
        void create_lua(std::function<void()> pLuaRegs = nullptr);

        /// Reads GUI files in the directory list.
        /** \note See add_addon_directory().
        *   \note See load_ui().
        *   \note See create_lua().
        */
        void read_files();

        /// Loads the UI.
        /** \note Calls create_lua() then read_files().
        */
        void load_ui();

        /// Closes the UI, deletes widgets.
        void close_ui();

        /// Closes the current UI and load it again.
        /** \note Calls close_ui() then load_ui().
        */
        void reload_ui();

        /// Renders the UI into the current render target.
        void render_ui() const;

        /// Renders a quad.
        /** \param mQuad The quad to render on the current render target
        *   \note This function is meant to be called between begin() and
        *         end() only.
        */
        void render_quad(const quad& mQuad) const;

        /// Renders a set of quads.
        /** \param mQuad     The base quad to use for rendering (material, blending, ...)
        *   \param lQuadList The list of the quads you want to render
        *   \note This function is meant to be called between begin() and
        *         end() only. It is always more efficient to call this method
        *         than calling render_quad repeatedly, as it allows to batch
        *         count reduction.
        */
        void render_quads(const quad& mQuad, const std::vector<std::array<vertex,4>>& lQuadList) const;

        /// Checks if the UI is currently being loaded.
        /** \return 'true' if the UI is currently being loaded
        */
        bool is_loading_ui() const;

        /// Ask this manager for movement management.
        /** \param pObj        The object to move
        *   \param pAnchor     The reference anchor
        *   \param mConstraint The constraint axis if any
        *   \note Movement is handled by the manager itself, you don't
        *         need to do anything.
        */
        void start_moving(
            uiobject* pObj, anchor* pAnchor = nullptr, constraint mConstraint = constraint::NONE,
            std::function<void()> pApplyConstraintFunc = nullptr
        );

        /// Stops movement for the given object.
        /** \param pObj The object to stop moving
        */
        void stop_moving(uiobject* pObj);

        /// Checks if the given object is allowed to move.
        /** \param pObj The object to check
        *   \return 'true' if the given object is allowed to move
        */
        bool is_moving(uiobject* pObj) const;

        /// Starts resizing a widget.
        /** \param pObj   The object to resize
        *   \param mPoint The sizing point
        *   \note Resizing is handled by the manager itself, you don't
        *         need to do anything.
        */
        void start_sizing(uiobject* pObj, anchor_point mPoint);

        /// Stops sizing for the given object.
        /** \param pObj The object to stop sizing
        */
        void stop_sizing(uiobject* pObj);

        /// Checks if the given object is allowed to be resized.
        /** \param pObj The object to check
        *   \return 'true' if the given object is allowed to be resized
        */
        bool is_sizing(uiobject* pObj) const;

        /// Returns the cumuled horizontal mouse movement.
        /** \return The cumuled horizontal mouse movement
        *   \note This value is reset to zero whenever start_moving() or
        *         start_sizing() is called.
        */
        int get_movement_x() const;

        /// Returns the cumuled vertical mouse movement.
        /** \return The cumuled vertical mouse movement
        *   \note This value is reset to zero whenever start_moving() or
        *         start_sizing() is called.
        */
        int get_movement_y() const;

        /// Tells this manager an object has moved.
        void notify_object_moved();

        /// Tells this manager to redraw the UI.
        void fire_redraw(frame_strata mStrata) const;

        /// Enables/disables GUI caching.
        /** \param bEnable 'true' to enable
        *   \note See toggle_caching().
        */
        void enable_caching(bool bEnable);

        /// Toggles render caching.
        /** \note Enabled by default.
        *   \note Enabling this will most likely improve performances.
        */
        void toggle_caching();

        /// Checks if GUI caching is enabled.
        /** \return 'true' if GUI caching is enabled
        */
        bool is_caching_enabled() const;

        /// Enables/disables input response for all widgets.
        /** \parem bEnable 'true' to enable input
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

        /// Sets wether the Manager should clear all fonts when closed.
        /** \param bClear 'true' to clear fonts
        *   \note Enabled by default. Note that when enabled, it will also
        *         clear fonts when the UI is reloaded, and load them once
        *         again.
        */
        void clear_fonts_on_close(bool bClear);

        /// Manually create a strata's render target.
        /** \param mframe_strata The strata to create the render target for
        *   \note Creating a render target can take some times. To
        *         avoid GUI hangs during creation, the manager
        *         automatically creates the render targets when
        *         read_files() is called, but only for strata that
        *         already contain frames when all .xml files are loaded.
        *         Other ones are created when needed, just before
        *         rendering on them. If you know that one of them is
        *         empty at load time, and is going to be filled later
        *         during run time, you can call this function to ensure
        *         the render target will be created at load time,
        *         causing no GUI hangs.
        */
        void create_strata_render_target(frame_strata mframe_strata);

        /// Returns the frame under the mouse.
        /** \return The frame under the mouse (nullptr if none)
        */
        const frame* get_overed_frame() const;

        /// Asks this manager for focus.
        /** \param pFocusFrame The focus_frame requesting focus
        */
        void request_focus(focus_frame* pFocusFrame);

        /// Returns the highest level on the provided strata.
        /** \param mframe_strata The strata to inspect
        *   \return The highest level on the provided strata
        */
        int get_highest_level(frame_strata mframe_strata) const;

        /// updates this manager and its widgets.
        /** \param fDelta The time elapsed since the last call
        */
        void update(float fDelta);

        /// Called whenever an Event occurs.
        /** \param mEvent The Event which has occured
        */
        void on_event(const event& mEvent) override;

        /// Tells the underlying graphics engine to start rendering into a new target.
        /** \param pTarget The target to render to (nullptr to render to the screen)
        */
        void begin(utils::refptr<render_target> pTarget = nullptr) const;

        /// Tells the underlying graphics engine we're done rendering.
        /** \note For most engines, this is when the rendering is actually
        *         done, so don't forget to call it !
        */
        void end() const;

        /// Registers a new frame type.
        /** \note Use as template argument the C++ type of this frame.
        *   \note The string parameter is not used anymore (still here for
        *         compatibility reasons, will disappear in a future release).
        */
        template<typename frame_type, typename enable = typename std::enable_if<std::is_base_of<gui::frame, frame_type>::value>::type>
        void register_frame_type(const std::string&)
        {
            lCustomFrameList_[frame_type::CLASS_NAME] = &create_new_frame<frame_type>;
            frame_type::register_glue(pLua_.get());
        }

        /// Registers a new frame type.
        /** \note Use as template argument the C++ type of this frame.
        */
        template<typename frame_type, typename enable = typename std::enable_if<std::is_base_of<gui::frame, frame_type>::value>::type>
        void register_frame_type()
        {
            lCustomFrameList_[frame_type::CLASS_NAME] = &create_new_frame<frame_type>;
            frame_type::register_glue(pLua_.get());
        }

        /// Registers a new layered_region type.
        /** \note Use as template argument the C++ type of this layered_region.
        *   \note The string parameter is not used anymore (still here for
        *         compatibility reasons, will disappear in a future release).
        */
        template<typename region_type, typename enable = typename std::enable_if<std::is_base_of<gui::layered_region, region_type>::value>::type>
        void register_region_type(const std::string&)
        {
            lCustomRegionList_[region_type::CLASS_NAME] = &create_new_layered_region<region_type>;
            region_type::register_glue(pLua_.get());
        }

        /// Registers a new layered_region type.
        /** \note Use as template argument the C++ type of this layered_region.
        */
        template<typename region_type, typename enable = typename std::enable_if<std::is_base_of<gui::layered_region, region_type>::value>::type>
        void register_region_type()
        {
            lCustomRegionList_[region_type::CLASS_NAME] = &create_new_layered_region<region_type>;
            region_type::register_glue(pLua_.get());
        }

        /// Returns the renderer implementation.
        /** \return The renderer implementation
        */
        const renderer_impl* get_renderer() const;

        /// Returns the renderer implementation.
        /** \return The renderer implementation
        */
        renderer_impl* get_renderer();

        /// Returns the gui event manager.
        /** \return The gui event manager
        */
        const event_manager* get_event_manager() const;

        /// Returns the gui event manager.
        /** \return The gui event manager
        */
        event_manager* get_event_manager();

        /// Returns the input manager associated to this gui.
        /** \return The input manager associated to this gui
        */
        const input::manager* get_input_manager() const;

        /// Returns the input manager associated to this gui.
        /** \return The input manager associated to this gui
        */
        input::manager* get_input_manager();

        /// Returns the current game locale ("enGB", ...).
        /** \return The current game locale
        */
        const std::string& get_locale() const;

        /// Returns the gui manager associated to the provided lua::state.
        /** \param pState The lua::state
        */
        static manager* get_manager(lua::state* pState);

    private :

        void register_lua_manager_();

        void load_addon_toc_(const std::string& sAddOnName, const std::string& sAddOnDirectory);
        void load_addon_files_(addon* pAddOn);
        void load_addon_directory_(const std::string& sDirectory);

        void save_variables_(const addon* pAddOn);

        void set_overed_frame_(frame* pFrame, int iX = 0, int iY = 0);

        void create_strata_render_target_(strata& mStrata);
        void render_strata_(strata& mStrata);

        void parse_xml_file_(const std::string& sFile, addon* pAddOn);

        std::string sUIVersion_ = "0001";
        uint        uiScreenWidth_ = 0u;
        uint        uiScreenHeight_=  0u;

        bool bClearFontsOnClose_ = true;

        std::unique_ptr<lua::state> pLua_;
        std::function<void()>       pLuaRegs_;
        bool                        bClosed_ = true;
        bool                        bLoadingUI_ = false;
        bool                        bFirstIteration_ = true;

        bool                            bInputEnabled_ = true;
        std::unique_ptr<input::manager> pInputManager_;
        std::map<input::key, std::map<input::key, std::map<input::key, std::string>>> lKeyBindingList_;

        std::map<std::string, uiobject*> lNamedObjectList_;
        std::map<std::string, uiobject*> lNamedVirtualObjectList_;

        std::map<uint, uiobject*> lObjectList_;
        std::map<uint, uiobject*> lMainObjectList_;

        std::vector<std::string> lGUIDirectoryList_;
        addon*                   pCurrentAddOn_ = nullptr;
        std::map<std::string, std::map<std::string, addon>> lAddOnList_;

        std::map<uint, frame*>         lFrameList_;
        std::map<frame_strata, strata> lStrataList_;
        bool                           bBuildStrataList_ = false;
        bool                           bObjectMoved_ = false;
        frame*                         pHoveredFrame_ = nullptr;
        bool                           bUpdateHoveredFrame_ = false;
        focus_frame*                   pFocusedFrame_ = nullptr;

        uiobject* pMovedObject_ = nullptr;
        uiobject* pSizedObject_ = nullptr;
        float     fMouseMovementX_ = 0.0f;
        float     fMouseMovementY_ = 0.0f;

        anchor*    pMovedAnchor_ = nullptr;
        int        iMovementStartPositionX_ = 0;
        int        iMovementStartPositionY_ = 0;
        constraint mConstraint_ = constraint::NONE;
        std::function<void()> pApplyConstraintFunc_;

        uint uiResizeStartW_ = 0u;
        uint uiResizeStartH_ = 0u;
        bool bResizeWidth_ = false;
        bool bResizeHeight_ = false;
        bool bResizeFromRight_ = false;
        bool bResizeFromBottom_ = false;

        uint uiFrameNumber_ = 0u;

        bool bEnableCaching_= true;

        utils::refptr<render_target> pRenderTarget_;
        sprite                       mSprite_;

        std::map<std::string, frame*(*)(manager*)>          lCustomFrameList_;
        std::map<std::string, layered_region*(*)(manager*)> lCustomRegionList_;

        std::string                    sLocale_;
        std::unique_ptr<event_manager> pEventManager_;
        std::unique_ptr<renderer_impl> pImpl_;
    };

    /// Abstract type for implementation specific management
    class renderer_impl
    {
    public :

        /// Constructor.
        renderer_impl() = default;

        /// Destructor.
        virtual ~renderer_impl() = default;

        /// Gives a pointer to the base class.
        /** \note This function is automatically called by gui::manager
        *         on creation.
        */
        void set_parent(manager* pParent);

        /// Begins rendering on a particular render target.
        /** \param pTarget The render target (main screen if nullptr)
        */
        virtual void begin(utils::refptr<render_target> pTarget = nullptr) const = 0;

        /// Ends rendering.
        virtual void end() const = 0;

        /// Renders a quad.
        /** \param mQuad The quad to render on the current render target
        *   \note This function is meant to be called between begin() and
        *         end() only.
        */
        virtual void render_quad(const quad& mQuad) const = 0;

        /// Renders a set of quads.
        /** \param mQuad     The base quad to use for rendering (material, blending, ...)
        *   \param lQuadList The list of the quads you want to render
        *   \note This function is meant to be called between begin() and
        *         end() only. It is always more efficient to call this method
        *         than calling render_quad repeatedly, as it allows to batch
        *         count reduction.
        */
        virtual void render_quads(const quad& mQuad, const std::vector<std::array<vertex,4>>& lQuadList) const = 0;

        /// Creates a new material from a texture file.
        /** \param sFileName The name of the file
        *   \return The new material
        *   \note Supported texture formats are defined by implementation.
        *         The gui library is completely unaware of this.
        */
        virtual utils::refptr<material> create_material(const std::string& sFileName,
            material::filter mFilter = material::filter::NONE) const = 0;

        /// Creates a new material from a plain color.
        /** \param mColor The color to use
        *   \return The new material
        */
        virtual utils::refptr<material> create_material(const color& mColor) const = 0;

        /// Creates a new material from a render target.
        /** \param pRenderTarget The render target from which to read the pixels
        *   \return The new material
        */
        virtual utils::refptr<material> create_material(utils::refptr<render_target> pRenderTarget) const = 0;

        /// Creates a new render target.
        /** \param uiWidth  The width of the render target
        *   \param uiHeight The height of the render target
        */
        virtual utils::refptr<render_target> create_render_target(uint uiWidth, uint uiHeight) const = 0;

        /// Creates a new font.
        /** \param sFontFile The file from which to read the font
        *   \param uiSize    The requested size of the characters (in points)
        *   \note Even though the gui has been designed to use vector fonts files
        *         (such as .ttf or .otf font formats), nothing prevents the implementation
        *         from using any other font type, including bitmap fonts.
        */
        virtual utils::refptr<font> create_font(const std::string& sFontFile, uint uiSize) const = 0;

        /// Notifies the renderer that the render window has been resized.
        /** \param uiNewWidth  The new window width
        *   \param uiNewHeight The new window height
        */
        virtual void notify_window_resized(uint uiNewWidth, uint uiNewHeight);

    protected :

        manager* pParent_ = nullptr;
    };
}
}


#endif
