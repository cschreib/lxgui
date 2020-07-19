#ifndef LXGUI_GUI_LAYEREDREGION_HPP
#define LXGUI_GUI_LAYEREDREGION_HPP

#include <lxgui/utils.hpp>
#include "lxgui/gui_region.hpp"

namespace lxgui {
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

        /// Prints all relevant information about this widget in a string.
        /** \param sTab The offset to give to all lines
        *   \return All relevant information about this widget
        */
        std::string serialize(const std::string& sTab) const override;

        /// Creates the associated Lua glue.
        void create_glue() override;

        /// Removes this widget from its parent and return an owning pointer.
        /** \return An owning pointer to this widget
        */
        std::unique_ptr<uiobject> release_from_parent() override;

        /// shows this widget.
        /** \note Its parent must be shown for it to appear on
        *         the screen.
        */
        void show() override;

        /// hides this widget.
        /** \note All its children won't be visible on the screen
        *   anymore, even if they are still marked as shown.
        */
        void hide() override;

        /// Checks if this widget can be seen on the screen.
        /** \return 'true' if this widget can be seen on the screen
        */
        bool is_visible() const override;

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
        void notify_renderer_need_redraw() const override;

        /// Parses data from an xml::block.
        /** \param pBlock The frame's xml::block
        */
        void parse_block(xml::block* pBlock) override;

        static constexpr const char* CLASS_NAME = "LayeredRegion";

    protected :

        void parse_attributes_(xml::block* pBlock) override;

        layer_type mLayer_ = layer_type::ARTWORK;
    };

    /** \cond NOT_REMOVE_FROM_DOC
    */

    class lua_layered_region : public lua_uiobject
    {
    public :

        explicit lua_layered_region(lua_State* pLua);
        layered_region* get_parent();

        int _get_draw_layer(lua_State*);
        int _set_draw_layer(lua_State*);

        static const char className[];
        static const char* classList[];
        static lua::lunar_binding<lua_layered_region> methods[];

    protected :

        layered_region* pLayeredRegionParent_;
    };

    /** \endcond
    */
}
}

#endif
