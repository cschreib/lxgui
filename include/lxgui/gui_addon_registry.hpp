#ifndef LXGUI_GUI_ADDON_REGISTRY_HPP
#define LXGUI_GUI_ADDON_REGISTRY_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_addon.hpp"

#include <string>
#include <vector>
#include <unordered_map>

/** \cond INCLUDE_INTERNALS_IN_DOC
*/
namespace sol {
    class state;
}
/** \endcond
*/

namespace lxgui {
namespace gui
{
    class localizer;
    class event_emitter;
    class uiroot;
    class virtual_uiroot;

    /// Loads and owns addons
    class addon_registry
    {
    public :

        /// Constructor.
        /** \param mLua          The GUI Lua state
        *   \param mLocalizer    The localizer class, to load new translation into
        *   \param mEventEmitter The event emitter, to fire "addon loaded" events
        *   \param mRoot         The GUI root, to create new frames into
        *   \param mVirtualRoot  The virtual root, to create new virtual frames into
        */
        addon_registry(sol::state& mLua, localizer& mLocalizer, event_emitter& mEventEmitter,
            uiroot& mRoot, virtual_uiroot& mVirtualRoot);

        addon_registry(const addon_registry&) = delete;
        addon_registry(addon_registry&&) = delete;
        addon_registry& operator = (const addon_registry&) = delete;
        addon_registry& operator = (addon_registry&&) = delete;

        /// Parse all addons inside a directory.
        /** \note The directory must contain a file named addon.txt, and
        *         listing all enabled (and possibly disabled) addons.
        *         Each addon is then a sub-directory.
        */
        void load_addon_directory(const std::string& sDirectory);

        /// Returns the addon that is being parsed.
        /** \return The addon that is being parsed
        */
        const addon* get_current_addon();

        /// Sets the current addon.
        /** \param pAddOn The current addon
        *   \note The current addon is used to set the addon of each new uiobject.
        *         This is normally set by the parser, while loading each addon.
        *         For uiobjects created manually, after the loading stage, this is
        *         also set by @ref frame, before each call to a handler function.
        */
        void set_current_addon(const addon* pAddOn);

        /// Save Lua variables registred for saving for all addons.
        void save_variables() const;

    private :

        void load_addon_toc_(const std::string& sAddOnName, const std::string& sAddOnDirectory);
        void load_addon_files_(const addon& mAddOn);

        void save_variables_(const addon& mAddOn) const noexcept;

        void parse_layout_file_(const std::string& sFile, const addon& mAddOn);

        template<typename T>
        using string_map = std::unordered_map<std::string,T>;

        sol::state&     mLua_;
        localizer&      mLocalizer_;
        event_emitter&  mEventEmitter_;
        uiroot&         mRoot_;
        virtual_uiroot& mVirtualRoot_;

        const addon*                  pCurrentAddOn_ = nullptr;
        string_map<string_map<addon>> lAddOnList_;
    };
}
}


#endif
