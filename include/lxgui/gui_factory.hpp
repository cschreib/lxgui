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

/**
 * \brief Handles the creation of new UI objects.
 * \note This is a low-level class, which is only meant to be used
 * internally by the GUI. To create your own UI objects, use
 * root::create_root_frame(), frame::create_child(), or
 * frame::create_region().
 */
class factory {
private:
    template<typename T>
    static utils::owner_ptr<region> create_new_object(manager& mgr) {
        return utils::make_owned<T>(mgr);
    }

    template<typename T>
    static utils::owner_ptr<frame> create_new_frame(manager& mgr) {
        return utils::make_owned<T>(mgr);
    }

    template<typename T>
    static utils::owner_ptr<layered_region> create_new_layered_region(manager& mgr) {
        return utils::make_owned<T>(mgr);
    }

public:
    /**
     * \brief Constructor.
     * \param mgr The GUI manager
     */
    explicit factory(manager& mgr);

    // Non-copiable, non-movable
    factory(const factory&) = delete;
    factory(factory&&)      = delete;
    factory& operator=(const factory&) = delete;
    factory& operator=(factory&&) = delete;

    /**
     * \brief Creates a new region.
     * \param reg The registry in which to register this object
     * \param attr The attributes of the object
     * \return The new frame
     * \note This function takes care of the basic initializing: the
     * object is directly usable.
     */
    utils::owner_ptr<region> create_region(registry& reg, const region_core_attributes& attr);

    /**
     * \brief Creates a new frame.
     * \param reg The registry in which to register this frame
     * \param rdr The frame_renderer that will render this frame
     * \param attr The attributes of the frame
     * \return The new frame
     * \note This function takes care of the basic initializing: the
     * frame is directly usable. However, you still need to call
     * notify_loaded() when you are done with any extra initialization
     * you require on this frame. If you do not, the frame's OnLoad
     * callback will not fire.
     */
    utils::owner_ptr<frame>
    create_frame(registry& reg, frame_renderer* rdr, const region_core_attributes& attr);

    /**
     * \brief Creates a new layered_region.
     * \param reg The registry in which to register this region
     * \param attr The attributes of the region
     * \return The new layered_region
     * \note This function takes care of the basic initializing: the
     * region is directly usable.
     */
    utils::owner_ptr<layered_region>
    create_layered_region(registry& reg, const region_core_attributes& attr);

    /**
     * \brief Registers a new object type.
     * \note Set the first template argument as the C++ type of this object.
     */
    template<
        typename ObjectType,
        typename Enable =
            typename std::enable_if<std::is_base_of<gui::region, ObjectType>::value>::type>
    void register_region_type() {
        if constexpr (std::is_base_of_v<gui::layered_region, ObjectType>) {
            custom_region_list_[ObjectType::class_name] = &create_new_layered_region<ObjectType>;
        } else if constexpr (std::is_base_of_v<gui::frame, ObjectType>) {
            custom_frame_list_[ObjectType::class_name] = &create_new_frame<ObjectType>;
        } else {
            custom_object_list_[ObjectType::class_name] = &create_new_object<ObjectType>;
        }

        custom_lua_regs_[ObjectType::class_name] = &ObjectType::register_on_lua;
    }

    /**
     * \brief Registers a new object type.
     * \param factory_func Factory function to call to create a new instance
     * \note Set the first template argument as the C++ type of this object.
     */
    template<
        typename ObjectType,
        typename FunctionType,
        typename Enable =
            typename std::enable_if<std::is_base_of<gui::region, ObjectType>::value>::type>
    void register_region_type(FunctionType&& factory_func) {
        if constexpr (std::is_base_of_v<gui::layered_region, ObjectType>) {
            custom_region_list_[ObjectType::class_name] =
                [factory_func = std::move(factory_func)](manager& mgr) {
                    return factory_func(mgr);
                };
        } else if constexpr (std::is_base_of_v<gui::frame, ObjectType>) {
            custom_frame_list_[ObjectType::class_name] =
                [factory_func = std::move(factory_func)](manager& mgr) {
                    return factory_func(mgr);
                };
        } else {
            custom_object_list_[ObjectType::class_name] =
                [factory_func = std::move(factory_func)](manager& mgr) {
                    return factory_func(mgr);
                };
        }

        custom_lua_regs_[ObjectType::class_name] = &ObjectType::register_on_lua;
    }

    /**
     * \brief Registers all region types on the provided Lua state
     * \param lua The Lua state
     */
    void register_on_lua(sol::state& lua);

private:
    bool finalize_object_(registry& reg, region& object, const region_core_attributes& attr);

    void apply_inheritance_(region& object, const region_core_attributes& attr);

    manager& manager_;

    template<typename T>
    using string_map = std::unordered_map<std::string, T>;

    string_map<std::function<utils::owner_ptr<region>(manager&)>>         custom_object_list_;
    string_map<std::function<utils::owner_ptr<frame>(manager&)>>          custom_frame_list_;
    string_map<std::function<utils::owner_ptr<layered_region>(manager&)>> custom_region_list_;
    string_map<std::function<void(sol::state&)>>                          custom_lua_regs_;
};

} // namespace lxgui::gui

#endif
