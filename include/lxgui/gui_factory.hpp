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
    class uiobject_core_attributes;
    class layered_region;
    class frame;
    class registry;
    class frame_renderer;
    class manager;

    /// Handles the creation of new UI objects.
    /** \note This is a low-level class, which is only meant to be used
    *         internally by the GUI. To create your own UI objects, use
    *         uiroot::create_root_frame(), frame::create_child(), or
    *         frame::create_region().
    */
    class factory
    {
    private :

        template<typename T>
        static utils::owner_ptr<uiobject> create_new_object_(manager& pMgr)
        {
            return utils::make_owned<T>(pMgr);
        }

        template<typename T>
        static utils::owner_ptr<frame> create_new_frame_(manager& pMgr)
        {
            return utils::make_owned<T>(pMgr);
        }

        template<typename T>
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
        /** \param mRegistry The registry in which to register this object
        *   \param mAttr     The attributes of the object
        *   \return The new frame
        *   \note This function takes care of the basic initializing: the
        *         object is directly usable.
        */
        utils::owner_ptr<uiobject> create_uiobject(
            registry& mRegistry, const uiobject_core_attributes& mAttr);

        /// Creates a new frame.
        /** \param mRegistry The registry in which to register this frame
        *   \param pRenderer The frame_renderer that will render this frame
        *   \param mAttr     The attributes of the frame
        *   \return The new frame
        *   \note This function takes care of the basic initializing: the
        *         frame is directly usable. However, you still need to call
        *         notify_loaded() when you are done with any extra initialization
        *         you require on this frame. If you do not, the frame's OnLoad
        *         callback will not fire.
        */
        utils::owner_ptr<frame> create_frame(
            registry& mRegistry, frame_renderer* pRenderer, const uiobject_core_attributes& mAttr);

        /// Creates a new layered_region.
        /** \param mRegistry The registry in which to register this region
        *   \param mAttr     The attributes of the region
        *   \return The new layered_region
        *   \note This function takes care of the basic initializing: the
        *         region is directly usable.
        */
        utils::owner_ptr<layered_region> create_layered_region(
            registry& mRegistry, const uiobject_core_attributes& mAttr);

        /// Registers a new object type.
        /** \note Set the first template argument as the C++ type of this object.
        */
        template<typename object_type,
            typename enable = typename std::enable_if<std::is_base_of<gui::uiobject, object_type>::value>::type>
        void register_uiobject_type()
        {
            if constexpr (std::is_base_of_v<gui::layered_region, object_type>)
                lCustomRegionList_[object_type::CLASS_NAME] = &create_new_layered_region_<object_type>;
            else if constexpr (std::is_base_of_v<gui::frame, object_type>)
                lCustomFrameList_[object_type::CLASS_NAME] = &create_new_frame_<object_type>;
            else
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
            const uiobject_core_attributes& mAttr);

        void apply_inheritance_(uiobject& mObject, const uiobject_core_attributes& mAttr);

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
