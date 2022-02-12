#ifndef LXGUI_GUI_FACTORY_HPP
#define LXGUI_GUI_FACTORY_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils_observer.hpp"

#include <functional>
#include <string>
#include <unordered_map>

/** \cond INCLUDE_INTERNALS_IN_DOC
 */
namespace sol {

class state;

}
/** \endcond
 */

namespace lxgui::gui {

class region;
struct region_core_attributes;
class layered_region;
class frame;
class registry;
class frame_renderer;
class manager;

/// Handles the creation of new UI objects.
/** \note This is a low-level class, which is only meant to be used
 *         internally by the GUI. To create your own UI objects, use
 *         root::create_root_frame(), frame::create_child(), or
 *         frame::create_region().
 */
class factory {
private:
    template<typename T>
    static utils::owner_ptr<region> create_new_object(manager& p_mgr) {
        return utils::make_owned<T>(p_mgr);
    }

    template<typename T>
    static utils::owner_ptr<frame> create_new_frame(manager& p_mgr) {
        return utils::make_owned<T>(p_mgr);
    }

    template<typename T>
    static utils::owner_ptr<layered_region> create_new_layered_region(manager& p_mgr) {
        return utils::make_owned<T>(p_mgr);
    }

public:
    /// Constructor.
    /** \param mManager The GUI manager
     */
    explicit factory(manager& m_manager);

    factory(const factory& m_mgr) = delete;
    factory(factory&& m_mgr)      = delete;
    factory& operator=(const factory& m_mgr) = delete;
    factory& operator=(factory&& m_mgr) = delete;

    /// Creates a new region.
    /** \param mRegistry The registry in which to register this object
     *   \param mAttr     The attributes of the object
     *   \return The new frame
     *   \note This function takes care of the basic initializing: the
     *         object is directly usable.
     */
    utils::owner_ptr<region>
    create_region(registry& m_registry, const region_core_attributes& m_attr);

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
        registry& m_registry, frame_renderer* p_renderer, const region_core_attributes& m_attr);

    /// Creates a new layered_region.
    /** \param mRegistry The registry in which to register this region
     *   \param mAttr     The attributes of the region
     *   \return The new layered_region
     *   \note This function takes care of the basic initializing: the
     *         region is directly usable.
     */
    utils::owner_ptr<layered_region>
    create_layered_region(registry& m_registry, const region_core_attributes& m_attr);

    /// Registers a new object type.
    /** \note Set the first template argument as the C++ type of this object.
     */
    template<
        typename ObjectType,
        typename Enable =
            typename std::enable_if<std::is_base_of<gui::region, ObjectType>::value>::type>
    void register_region_type() {
        if constexpr (std::is_base_of_v<gui::layered_region, ObjectType>)
            l_custom_region_list_[ObjectType::class_name] = &create_new_layered_region<ObjectType>;
        else if constexpr (std::is_base_of_v<gui::frame, ObjectType>)
            l_custom_frame_list_[ObjectType::class_name] = &create_new_frame<ObjectType>;
        else
            l_custom_object_list_[ObjectType::class_name] = &create_new_object<ObjectType>;

        ObjectType::register_on_lua(get_lua());
    }

    /// Returns the GUI Lua state (sol wrapper).
    /** \return The GUI Lua state
     */
    sol::state& get_lua();

    /// Returns the GUI Lua state (sol wrapper).
    /** \return The GUI Lua state
     */
    const sol::state& get_lua() const;

private:
    bool
    finalize_object_(registry& m_registry, region& m_object, const region_core_attributes& m_attr);

    void apply_inheritance_(region& m_object, const region_core_attributes& m_attr);

    manager& m_manager_;

    template<typename T>
    using string_map = std::unordered_map<std::string, T>;

    string_map<std::function<utils::owner_ptr<region>(manager&)>>         l_custom_object_list_;
    string_map<std::function<utils::owner_ptr<frame>(manager&)>>          l_custom_frame_list_;
    string_map<std::function<utils::owner_ptr<layered_region>(manager&)>> l_custom_region_list_;
};

} // namespace lxgui::gui

#endif
