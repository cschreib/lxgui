#ifndef LXGUI_GUI_FACTORY_HPP
#define LXGUI_GUI_FACTORY_HPP

#include <lxgui/lxgui.hpp>

#include <lxgui/utils_observer.hpp>

#include <string>
#include <unordered_map>
#include <functional>

namespace sol {
    class state;
}

namespace lxgui {

namespace gui
{
    class uiobject;
    class layered_region;
    class frame;
    class registry;
    class manager;

    /// Handles the creation of new UI objects.
    class factory
    {
    private :

        template <class T>
        static utils::owner_ptr<uiobject> create_new_object_(manager& pMgr)
        {
            return utils::make_owned<T>(pMgr);
        }

        template <class T>
        static utils::owner_ptr<frame> create_new_frame_(manager& pMgr)
        {
            return utils::make_owned<T>(pMgr);
        }

        template <class T>
        static utils::owner_ptr<layered_region> create_new_layered_region_(manager& pMgr)
        {
            return utils::make_owned<T>(pMgr);
        }

    public :

        /// Constructor.
        /** \param mManager The GUI manager
        */
        explicit factory(manager& mManager);

        factory(const factory& mMgr) = delete;
        factory(factory&& mMgr) = delete;
        factory& operator = (const factory& mMgr) = delete;
        factory& operator = (factory&& mMgr) = delete;

        /// Creates a new uiobject.
        /** \param sClassName The sub class of the frame (Button, ...)
        *   \return The new frame
        *   \note This is a low level function; the returned frame only has the bare
        *         minimum initialization. Use create_root_frame() or frame::create_child()
        *         to get a fully-functional frame object.
        */
        utils::owner_ptr<uiobject> create_uiobject(
           registry& mRegistry, const std::string& sClassName, bool bVirtual,
           const std::string& sName, utils::observer_ptr<frame> pParent = nullptr);

        /// Creates a new frame.
        /** \param sClassName The sub class of the frame (Button, ...)
        *   \return The new frame
        *   \note This is a low level function; the returned frame only has the bare
        *         minimum initialization. Use create_root_frame() or frame::create_child()
        *         to get a fully-functional frame object.
        */
        utils::owner_ptr<frame> create_frame(
           registry& mRegistry, const std::string& sClassName, bool bVirtual,
           const std::string& sName, utils::observer_ptr<frame> pParent = nullptr);

        /// Creates a new layered_region.
        /** \param sClassName The sub class of the layered_region (FontString or texture)
        *   \return The new layered_region
        *   \note This is a low level function; the returned region only has the bare
        *         minimum initialization. Use frame::create_region() to get a fully-functional
        *         region object.
        */
        utils::owner_ptr<layered_region> create_layered_region(
            registry& mRegistry, const std::string& sClassName, bool bVirtual,
            const std::string& sName, utils::observer_ptr<frame> pParent = nullptr);

        /// Registers a new object type.
        /** \note Set the first template argument as the C++ type of this object.
        */
        template<typename object_type,
            typename enable = typename std::enable_if<std::is_base_of<gui::uiobject, object_type>::value>::type>
        void register_uiobject_type()
        {
            if constexpr (std::is_base_of_v<gui::layered_region, object_type>)
                lCustomRegionList_[object_type::CLASS_NAME] = &create_new_layered_region_<object_type>;
            if constexpr (std::is_base_of_v<gui::frame, object_type>)
                lCustomFrameList_[object_type::CLASS_NAME] = &create_new_frame_<object_type>;

            lCustomObjectList_[object_type::CLASS_NAME] = &create_new_object_<object_type>;

            object_type::register_on_lua(get_lua());
        }

        /// Returns the GUI Lua state (sol wrapper).
        /** \return The GUI Lua state
        */
        sol::state& get_lua();

        /// Returns the GUI Lua state (sol wrapper).
        /** \return The GUI Lua state
        */
        const sol::state& get_lua() const;

    private :

        bool finalize_object_(registry& mRegistry, uiobject& mObject,
            bool bVirtual, const std::string& sName, utils::observer_ptr<frame> pParent);

        manager& mManager_;

        template<typename T>
        using string_map = std::unordered_map<std::string,T>;

        string_map<std::function<utils::owner_ptr<uiobject>(manager&)>>       lCustomObjectList_;
        string_map<std::function<utils::owner_ptr<frame>(manager&)>>          lCustomFrameList_;
        string_map<std::function<utils::owner_ptr<layered_region>(manager&)>> lCustomRegionList_;
    };
}
}


#endif
