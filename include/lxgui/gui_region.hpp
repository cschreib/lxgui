#ifndef LXGUI_GUI_REGION_HPP
#define LXGUI_GUI_REGION_HPP

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
        explicit region(manager* pManager);

        /// Destructor.
        ~region();

        /// Renders this widget on the current render target.
        /** \note Does nothing.
        */
        void render() override;

        /// Checks if the provided coordinates are inside this region.
        /** \param iX The horizontal coordinate
        *   \param iY The vertical coordinate
        *   \return 'true' if the provided coordinates are inside this region
        */
        virtual bool is_in_region(int iX, int iY) const;

        /// Creates the associated Lua glue.
        void create_glue() override;

        /// Parses data from an xml::block.
        /** \param pBlock The frame's xml::block
        */
        void parse_block(xml::block* pBlock) override;

        static constexpr const char* CLASS_NAME = "Region";

    protected :

        virtual void parse_attributes_(xml::block* pBlock);
    };
}
}

#endif
