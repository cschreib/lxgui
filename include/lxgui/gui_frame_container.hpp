#ifndef LXGUI_GUI_FRAME_CONTAINER_HPP
#define LXGUI_GUI_FRAME_CONTAINER_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_uiobject_attributes.hpp"

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
        /** \param mManager  The GUI manager
        *   \param mRegistry The registry in which new frames should be registered
        *   \param pRenderer The frame_renderer that will render these frames (nullptr if none).
        */
        explicit frame_container(manager& mManager, registry& mRegistry, frame_renderer* pRenderer);

        virtual ~frame_container() = default;
        frame_container(const frame_container&) = delete;
        frame_container(frame_container&&) = delete;
        frame_container& operator = (const frame_container&) = delete;
        frame_container& operator = (frame_container&&) = delete;

        /// Creates a new frame, ready for use, and owned by this frame_container.
        /** \param mAttr The core attributes of the frame (pParent will be ignored)
        *   \return The new frame
        *   \note This function takes care of the basic initializing: the
        *         frame is directly usable. However, you still need to call
        *         notify_loaded() when you are done with any extra initialization
        *         you require on this frame. If you do not, the frame's OnLoad
        *         callback will not fire.
        */
        utils::observer_ptr<frame> create_root_frame(uiobject_core_attributes mAttr)
        {
            mAttr.pParent = nullptr;

            return create_root_frame_(mAttr);
        }

        /// Creates a new frame, ready for use, and owned by this frame_container.
        /** \param mAttr The core attributes of the frame (sObjectType and pParent will be ignored)
        *   \return The new frame
        *   \note This function takes care of the basic initializing: the
        *         frame is directly usable. However, you still need to call
        *         notify_loaded() when you are done with any extra initialization
        *         you require on this frame. If you do not, the frame's OnLoad
        *         callback will not fire.
        */
        template<typename frame_type, typename enable =
            typename std::enable_if<std::is_base_of<gui::frame, frame_type>::value>::type>
        utils::observer_ptr<frame> create_root_frame(uiobject_core_attributes mAttr)
        {
            mAttr.sObjectType = frame_type::CLASS_NAME;
            mAttr.pParent = nullptr;

            return utils::static_pointer_cast<frame_type>(create_root_frame_(mAttr));
        }

        /// Creates a new frame, ready for use, and owned by this frame_container.
        /** \param sName The name of this frame
        *   \return The new frame
        *   \note This function takes care of the basic initializing: the
        *         frame is directly usable. However, you still need to call
        *         notify_loaded() when you are done with any extra initialization
        *         you require on this frame. If you do not, the frame's OnLoad
        *         callback will not fire.
        */
        template<typename frame_type, typename enable =
            typename std::enable_if<std::is_base_of<gui::frame, frame_type>::value>::type>
        utils::observer_ptr<frame> create_root_frame(const std::string& sName)
        {
            uiobject_core_attributes mAttr;
            mAttr.sName = sName;
            mAttr.sObjectType = frame_type::CLASS_NAME;

            return utils::static_pointer_cast<frame_type>(create_root_frame_(mAttr));
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
        registry& get_registry() { return mRegistry_; }

        /// Returns the UI object registry, which keeps track of all objects in the UI.
        /** \return The registry object
        */
        const registry& get_registry() const { return mRegistry_; }

    protected :

        virtual utils::observer_ptr<frame> create_root_frame_(
            const uiobject_core_attributes& mAttr);

        void clear_frames_();

    private :

        manager& mManager_;
        registry& mRegistry_;
        frame_renderer* pRenderer_;

        root_frame_list lRootFrameList_;
    };
}
}


#endif
