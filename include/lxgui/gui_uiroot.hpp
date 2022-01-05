#ifndef LXGUI_GUI_UIROOT_HPP
#define LXGUI_GUI_UIROOT_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/gui_eventreceiver.hpp"
#include "lxgui/gui_framerenderer.hpp"
#include "lxgui/gui_frame_container.hpp"
#include "lxgui/gui_registry.hpp"
#include "lxgui/gui_vector2.hpp"

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
    class uiroot: public frame_renderer, public event_receiver, public frame_container
    {
    public :

        /// Constructor.
        /** \param mManager The GUI manager
        */
        explicit uiroot(utils::control_block& mBlock, manager& mManager);

        /// Destructor.
        ~uiroot() override;

        uiroot(const uiroot&) = delete;
        uiroot(uiroot&&) = delete;
        uiroot& operator = (const uiroot&) = delete;
        uiroot& operator = (uiroot&&) = delete;

        /// Returns the width and height of this renderer's main render target (e.g., screen).
        /** \return The render target dimensions
        */
        vector2f get_target_dimensions() const override;

        /// Renders the UI into the current render target.
        void render() const;

        /// Enables or disables interface caching.
        /** \param bEnableCaching 'true' to enable, 'false' to disable
        *   \see toggle_caching()
        */
        void enable_caching(bool bEnable);

        /// Toggles interface caching.
        /** \note Disabled by default. Enabling this will most likely improve performances,
        *         at the expense of higher GPU memory usage. The UI will be cached into
        *         large render targets, which are only redrawn when the UI changes, rather
        *         than redrawn on each frame.
        */
        void toggle_caching();

        /// Checks if interface caching is enabled.
        /** \return 'true' if interface caching is enabled
        *   \see toggle_caching()
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

        /// Returns the UI object registry, which keeps track of all objects in the UI.
        /** \return The registry object
        */
        registry& get_registry() { return mObjectRegistry_; }

        /// Returns the UI object registry, which keeps track of all objects in the UI.
        /** \return The registry object
        */
        const registry& get_registry() const { return mObjectRegistry_; }

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

        void create_caching_render_target_();
        void create_strata_cache_render_target_(strata& mStrata);

        manager& mManager_;
        renderer& mRenderer_;
        registry mObjectRegistry_;

        vector2ui mScreenDimensions_;

        bool bEnableCaching_= false;

        std::shared_ptr<render_target> pRenderTarget_;
        quad                           mScreenQuad_;
    };
}
}


#endif
