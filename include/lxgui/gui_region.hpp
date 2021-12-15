#ifndef LXGUI_GUI_REGION_HPP
#define LXGUI_GUI_REGION_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_uiobject.hpp"

namespace lxgui {
namespace gui
{
    /// Simple GUI region (not renderable).
    /** \note It is the simplest derivate of uiobject, so if you
    *         just need a widget that has a position and a size
    *         (like frame's title region), then this is the best
    *         choice.
    */
    class region : public uiobject
    {
    public :

        /// Constructor.
        explicit region(manager& mManager);

        /// Renders this widget on the current render target.
        /** \note Does nothing.
        */
        void render() const override;

        /// Checks if the provided coordinates are inside this region.
        /** \param mPosition The coordinates to test
        *   \return 'true' if the provided coordinates are inside this region
        */
        virtual bool is_in_region(const vector2f& mPosition) const;

        /// Creates the associated Lua glue.
        void create_glue() override;

        /// Parses data from a utils::layout_node.
        /** \param mNode The layout node
        */
        void parse_layout(const utils::layout_node& mNode) override;

        static constexpr const char* CLASS_NAME = "Region";

    protected :

        virtual void parse_attributes_(const utils::layout_node& mNode);
    };
}
}

#endif
