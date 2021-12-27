#ifndef LXGUI_GUI_UIROOT_HPP
#define LXGUI_GUI_UIROOT_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_framerenderer.hpp"
#include "lxgui/gui_vector2.hpp"

#include <lxgui/utils_view.hpp>
#include <lxgui/utils_observer.hpp>

#include <list>
#include <memory>

namespace lxgui {

namespace gui
{
    class uiobject;
    class frame;
    class manager;
    class renderer;

    /// Root of the UI object hierarchy.
    /** This class contains and owns all "root" frames (frames with no parents)
    *   and is responsible for their lifetime, update, and rendering.
    */
    class uiroot: public frame_renderer, public event_receiver
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
        /** \param pInputSource The input source to use
        *   \param pRenderer    The renderer implementation
        */
        uiroot(manager& mManager);

        uiroot(const uiroot&) = delete;
        uiroot(uiroot&&) = delete;
        uiroot& operator = (const uiroot&) = delete;
        uiroot& operator = (uiroot&&) = delete;

        /// Returns the width and height of of this renderer's main render target (e.g., screen).
        /** \return The render target dimensions
        */
        vector2f get_target_dimensions() const override;

        /// Creates a new frame, ready for use, and owned by this uiroot.
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
            return create_root_frame_(sClassName, sName, false, lInheritance);
        }

        /// Creates a new frame, ready for use, and owned by this uiroot.
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
                create_root_frame_(frame_type::CLASS_NAME, sName, false, lInheritance));
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
            return create_root_frame_(sClassName, sName, true, lInheritance);
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
                create_root_frame_(frame_type::CLASS_NAME, sName, true, lInheritance));
        }

        /// Make a frame owned by this uiroot.
        /** \param pFrame The frame to add to the root frame list
        *   \return Raw pointer to the frame
        */
        utils::observer_ptr<frame> add_root_frame(utils::owner_ptr<frame> pFrame);

        /// Remove a frame from the list of frames owned by this uiroot.
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

        /// Renders the UI into the current render target.
        void render() const;

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

        /// updates this uiroot and its widgets.
        /** \param fDelta The time elapsed since the last call
        */
        void update(float fDelta);

        /// Called whenever an Event occurs.
        /** \param mEvent The Event which has occured
        */
        void on_event(const event& mEvent) override;

        /// Tells this object that the global interface scaling factor has changed.
        void notify_scaling_factor_updated();

        /// Returns this widget's manager.
        /** \return This widget's manager
        */
        manager& get_manager() { return mManager_; }

        /// Returns this widget's manager.
        /** \return This widget's manager
        */
        const manager& get_manager() const { return mManager_; }

        /// Return an observer pointer to 'this'.
        /** \return A new observer pointer pointing to 'this'.
        */
        utils::observer_ptr<const uiroot> observer_from_this() const
        {
            return utils::static_pointer_cast<const uiroot>(event_receiver::observer_from_this());
        }

        /// Return an observer pointer to 'this'.
        /** \return A new observer pointer pointing to 'this'.
        */
        utils::observer_ptr<uiroot> observer_from_this()
        {
            return utils::static_pointer_cast<uiroot>(event_receiver::observer_from_this());
        }

    private :

        utils::observer_ptr<frame> create_root_frame_(const std::string& sClassName,
            const std::string& sName, bool bVirtual,
            const std::vector<utils::observer_ptr<const uiobject>>& lInheritance);

        void create_caching_render_target_();
        void create_strata_cache_render_target_(strata& mStrata);

        manager& mManager_;
        renderer& mRenderer_;

        vector2ui mScreenDimensions_;

        root_frame_list lRootFrameList_;

        bool bEnableCaching_= true;

        std::shared_ptr<render_target> pRenderTarget_;
        quad                           mScreenQuad_;
    };
}
}


#endif
