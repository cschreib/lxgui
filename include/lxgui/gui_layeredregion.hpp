#ifndef LXGUI_GUI_LAYEREDREGION_HPP
#define LXGUI_GUI_LAYEREDREGION_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_region.hpp"

namespace lxgui {
namespace gui
{
    /// ID of a layer for rendering inside a frame.
    enum class layer_type
    {
        BACKGROUND = 0,
        BORDER = 1,
        ARTWORK = 2,
        OVERLAY = 3,
        HIGHLIGHT = 4,
        SPECIALHIGH = 5,
        ENUM_SIZE
    };

    /// Converts a string representation of a layer into the corresponding enumerator
    /** \param sLayer The layer string (e.g., "ARTWORK")
    *   \return The corresponding enumerator, or "ARTWORK" if parsing failed
    */
    layer_type parse_layer_type(const std::string& sLayer);

    /// A #uiobject that can be rendered in a layer.
    /** Layered regions can display content on the screen (texture,
    *   texts, 3D models, ...) and must be contained inside a layer,
    *   within a #lxgui::gui::frame object. The frame will then render all
    *   its layered regions, sorted by layers.
    *
    *   Layered regions cannot themselves react to events; this
    *   must be taken care of by the parent frame.
    */
    class layered_region : public region
    {
    public :

        /// Constructor.
        explicit layered_region(manager& mManager);

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
        utils::owner_ptr<uiobject> release_from_parent() override;

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
        layer_type get_draw_layer() const;

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

        /// Parses data from a layout_node.
        /** \param mNode The layout node
        */
        void parse_layout(const layout_node& mNode) override;

        /// Registers this widget class to the provided Lua state
        static void register_on_lua(sol::state& mLua);

        static constexpr const char* CLASS_NAME = "LayeredRegion";

    protected :

        void parse_attributes_(const layout_node& mNode) override;

        layer_type mLayer_ = layer_type::ARTWORK;
    };
}
}

#endif
