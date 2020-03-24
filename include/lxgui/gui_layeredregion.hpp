#ifndef GUI_LAYEREDREGION_HPP
#define GUI_LAYEREDREGION_HPP

#include <lxgui/utils.hpp>
#include "lxgui/gui_region.hpp"

namespace gui
{
    /// Abstract GUI renderable.
    /** \note layered regions are contained inside frames.<br>
    *         They are sorted by layers, hence their name.
    */
    class layered_region : public region
    {
    public :

        /// Constructor.
        explicit layered_region(manager* pManager);

        /// Destructor.
        virtual ~layered_region();

        /// Prints all relevant information about this widget in a string.
        /** \param sTab The offset to give to all lines
        *   \return All relevant information about this widget
        */
        virtual std::string serialize(const std::string& sTab) const;

        /// Creates the associated Lua glue.
        virtual void create_glue();

        /// Changes this widget's parent.
        /** \param pParent The new parent
        *   \note Default is nullptr.
        */
        virtual void set_parent(uiobject* pParent);

        /// shows this widget.
        /** \note Its parent must be shown for it to appear on
        *         the screen.
        */
        virtual void show();

        /// hides this widget.
        /** \note All its children won't be visible on the screen
        *   anymore, even if they are still marked as shown.
        */
        virtual void hide();

        /// Checks if this widget can be seen on the screen.
        /** \return 'true' if this widget can be seen on the screen
        */
        virtual bool is_visible() const;

        /// Returns this layered_region's draw layer.
        /** \return this layered_region's draw layer
        */
        layer_type get_draw_layer();

        /// Sets this layered_region's draw layer.
        /** \param mLayer The new layer
        */
        virtual void set_draw_layer(layer_type mLayer);

        /// Sets this layered_region's draw layer.
        /** \param sLayer The new layer
        */
        virtual void set_draw_layer(const std::string& sLayer);

        /// Notifies the renderer of this widget that it needs to be redrawn.
        /** \note Automatically called by any shape changing function.
        */
        virtual void notify_renderer_need_redraw() const;

        /// Parses data from an xml::block.
        /** \param pBlock The frame's xml::block
        */
        virtual void parse_block(xml::block* pBlock);

        static constexpr const char* CLASS_NAME = "LayeredRegion";

    protected :

        virtual void parse_attributes_(xml::block* pBlock);

        layer_type mLayer_;
        frame*     pFrameParent_;
    };

    /** \cond NOT_REMOVE_FROM_DOC
    */

    class lua_layered_region : public lua_uiobject
    {
    public :

        explicit lua_layered_region(lua_State* pLua);

        int _get_draw_layer(lua_State*);
        int _set_draw_layer(lua_State*);

        static const char className[];
        static const char* classList[];
        static Lunar<lua_layered_region>::RegType methods[];

    protected :

        layered_region* pLayeredRegionParent_;
    };

    /** \endcond
    */
}

#endif
