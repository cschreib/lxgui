#ifndef LXGUI_GUI_FRAME_CONTAINER_HPP
#define LXGUI_GUI_FRAME_CONTAINER_HPP

#include <lxgui/lxgui.hpp>

#include <lxgui/utils_view.hpp>
#include <lxgui/utils_observer.hpp>

#include <vector>
#include <list>
#include <memory>

namespace lxgui {

namespace gui
{
    class uiobject;
    class frame;
    class registry;
    class frame_renderer;
    class manager;

    /// Container of frames.
    /** This class contains and owns "root" frames (frames with no parents)
    *   and is responsible for their lifetime.
    */
    class frame_container
    {
    public :

        /// Type of the root frame list.
        /** \note Constraints on the choice container type:
        *          - must not invalidate iterators on back insertion
        *          - must allow forward iteration
        *          - iterators can be invalidated on removal
        *          - most common use is iteration, not addition or removal
        *          - ordering of elements is irrelevant
        */
        using root_frame_list = std::list<utils::owner_ptr<frame>>;
        using root_frame_list_view = utils::view::adaptor<root_frame_list,
            utils::view::smart_ptr_dereferencer,
            utils::view::non_null_filter>;
        using const_root_frame_list_view = utils::view::adaptor<const root_frame_list,
            utils::view::smart_ptr_dereferencer,
            utils::view::non_null_filter>;

        /// Constructor.
        /** \param mManager The GUI manager
        */
        explicit frame_container(manager& mManager, frame_renderer* pRenderer);

        frame_container(const frame_container&) = delete;
        frame_container(frame_container&&) = delete;
        frame_container& operator = (const frame_container&) = delete;
        frame_container& operator = (frame_container&&) = delete;

        /// Creates a new frame, ready for use, and owned by this frame_container.
        /** \param sClassName   The sub class of the frame (Button, ...)
        *   \param sName        The name of this frame
        *   \param lInheritance The objects to inherit from
        *   \return The new frame
        *   \note This function takes care of the basic initializing: the
        *         frame is directly usable. However, you still need to call
        *         notify_loaded() when you are done with any extra initialization
        *         you require on this frame. If you do not, the frame's OnLoad
        *         callback will not fire.
        */
        utils::observer_ptr<frame> create_root_frame(
            const std::string& sClassName, const std::string& sName,
            const std::vector<utils::observer_ptr<const uiobject>>& lInheritance = {})
        {
            return create_root_frame_(get_registry(), pRenderer_,
                sClassName, sName, false, lInheritance);
        }

        /// Creates a new frame, ready for use, and owned by this frame_container.
        /** \param sName        The name of this frame
        *   \param lInheritance The objects to inherit from
        *   \return The new frame
        *   \note This function takes care of the basic initializing: the
        *         frame is directly usable. However, you still need to call
        *         notify_loaded() when you are done with any extra initialization
        *         you require on this frame. If you do not, the frame's OnLoad
        *         callback will not fire.
        */
        template<typename frame_type, typename enable =
            typename std::enable_if<std::is_base_of<gui::frame, frame_type>::value>::type>
        utils::observer_ptr<frame> create_root_frame(const std::string& sName,
            const std::vector<utils::observer_ptr<const uiobject>>& lInheritance = {})
        {
            return utils::static_pointer_cast<frame_type>(
                create_root_frame_(get_registry(), pRenderer_,
                    frame_type::CLASS_NAME, sName, false, lInheritance));
        }

        /// Creates a new virtual frame, ready for use, and owned by this manager.
        /** \param sClassName   The sub class of the frame (Button, ...)
        *   \param sName        The name of this frame
        *   \param lInheritance The objects to inherit from
        *   \return The new frame
        *   \note This function takes care of the basic initializing: the
        *         frame is directly usable for inheritance.
        *   \note Virtual frames are not displayed, but they can be used as templates
        *         to create other frames through inheritance.
        */
        utils::observer_ptr<frame> create_virtual_root_frame(
            const std::string& sClassName, const std::string& sName,
            const std::vector<utils::observer_ptr<const uiobject>>& lInheritance = {})
        {
            return create_root_frame_(get_virtual_registry(), nullptr,
                sClassName, sName, true, lInheritance);
        }

        /// Creates a new virtual frame, ready for use, and owned by this manager.
        /** \param sName        The name of this frame
        *   \param lInheritance The objects to inherit from
        *   \return The new frame
        *   \note This function takes care of the basic initializing: the
        *         frame is directly usable for inheritance.
        *   \note Virtual frames are not displayed, but they can be used as templates
        *         to create other frames through inheritance.
        */
        template<typename frame_type, typename enable =
            typename std::enable_if<std::is_base_of<gui::frame, frame_type>::value>::type>
        utils::observer_ptr<frame> create_virtual_root_frame(const std::string& sName,
            const std::vector<utils::observer_ptr<const uiobject>>& lInheritance = {})
        {
            return utils::static_pointer_cast<frame_type>(
                create_root_frame_(get_virtual_registry(), nullptr,
                    frame_type::CLASS_NAME, sName, true, lInheritance));
        }

        /// Make a frame owned by this frame_container.
        /** \param pFrame The frame to add to the root frame list
        *   \return Raw pointer to the frame
        */
        utils::observer_ptr<frame> add_root_frame(utils::owner_ptr<frame> pFrame);

        /// Remove a frame from the list of frames owned by this frame_container.
        /** \param pFrame The frame to be released
        *   \return A unique_ptr to the previously owned frame, ignore it to destroy it.
        */
        utils::owner_ptr<frame> remove_root_frame(
            const utils::observer_ptr<frame>& pFrame);

        /// Returns the root frame list.
        /** \return The root frame list
        */
        root_frame_list_view get_root_frames();

        /// Returns the root frame list.
        /** \return The root frame list
        */
        const_root_frame_list_view get_root_frames() const;

        /// Clean deleted entries from the frame list.
        /** \note This must not be called while the root frames are being iterated on.
        *         All this does is free up "nullptr" entries from the list, to speed up
        *         future iteration.
        */
        void garbage_collect();

        /// Returns this widget's manager.
        /** \return This widget's manager
        */
        manager& get_manager() { return mManager_; }

        /// Returns this widget's manager.
        /** \return This widget's manager
        */
        const manager& get_manager() const { return mManager_; }

        /// Returns the UI object registry, which keeps track of all objects in the UI.
        /** \return The registry object
        */
        registry& get_registry();

        /// Returns the UI object registry, which keeps track of all objects in the UI.
        /** \return The registry object
        */
        const registry& get_registry() const;

        /// Returns the UI object registry, which keeps track of all objects in the UI.
        /** \return The registry object
        */
        registry& get_virtual_registry();

        /// Returns the UI object registry, which keeps track of all objects in the UI.
        /** \return The registry object
        */
        const registry& get_virtual_registry() const;

    protected :

        virtual utils::observer_ptr<frame> create_root_frame_(
            registry& mRegistry, frame_renderer* pRenderer, const std::string& sClassName,
            const std::string& sName, bool bVirtual,
            const std::vector<utils::observer_ptr<const uiobject>>& lInheritance);

    private :

        manager& mManager_;
        frame_renderer* pRenderer_;

        root_frame_list lRootFrameList_;
    };
}
}


#endif
