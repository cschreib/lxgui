#ifndef LXGUI_GUI_BACKDROP_HPP
#define LXGUI_GUI_BACKDROP_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_sprite.hpp"
#include "lxgui/gui_quad2.hpp"

#include <string>

namespace lxgui {
namespace gui
{
    class frame;

    /// Draws borders and background of a frame.
    class backdrop
    {
    public :

        /// Constructor.
        /** \param pParent The frame it is linked to
        */
        explicit backdrop(frame* pParent);

        /// Copies a backdrop's parameters into this one (inheritance).
        /** \param mbackdrop The backdrop to copy
        */
        void copy_from(const backdrop& mbackdrop);

        /// Sets the background texture.
        /** \param sBackgroundFile The background texture
        */
        void set_background(const std::string& sBackgroundFile);

        /// Returns this backdrop's background file.
        /** \return This backdrop's background file
        */
        const std::string& get_background_file() const;

        /// Sets the background color.
        /** \param mColor The background color
        *   \note This color can be used to tint the background texture if any
        *         or simply render a plain color background.
        */
        void set_background_color(const color& mColor);

        /// Returns the background color.
        /** \return The background color
        */
        color get_background_color() const;

        /// Enables tilling for the background texture.
        /** \param bBackgroundTilling 'true' to enable tilling
        */
        void set_backgrond_tilling(bool bBackgroundTilling);

        /// Checks if tilling is enabled for the background texture.
        /** \return 'true' if tilling is enabled for the background texture
        */
        bool is_background_tilling() const;

        /// Sets the appearent tile size.
        /** \param uiTileSize The new edget size
        *   \note Tile will be scaled by uiTileSize/backgroundTextureSize.
        */
        void set_tile_size(uint uiTileSize);

        /// Returns this backdrop's tile size.
        /** \return This backdrop's tile size
        */
        uint get_tile_size() const;

        /// Sets insets for the background texture.
        /** \param lInsets The insets array
        */
        void set_background_insets(const quad2i& lInsets);

        /// Sets insets for the background texture.
        /** \param iLeft   The left inset
        *   \param iRight  The right inset
        *   \param iTop    The top inset
        *   \param iBottom The bottom inset
        */
        void set_background_insets(int iLeft, int iRight, int iTop, int iBottom);

        /// Returns this backdrop's background insets.
        /** \return This backdrop's background insets
        */
        const quad2i& get_background_insets() const;

        /// Sets insets for the edge texture.
        /** \param lInsets The insets array
        */
        void set_edge_insets(const quad2i& lInsets);

        /// Sets insets for the edge texture.
        /** \param iLeft   The left inset
        *   \param iRight  The right inset
        *   \param iTop    The top inset
        *   \param iBottom The bottom inset
        */
        void set_edge_insets(int iLeft, int iRight, int iTop, int iBottom);

        /// Returns this backdrop's edge insets.
        /** \return This backdrop's edge insets
        */
        const quad2i& get_edge_insets() const;

        /// Sets the edge/corner texture.
        /** \param sEdgeFile The edge/corner texture
        *   \note This texture's width must be 8 times greater than its
        *         height.<br><br>
        *         texture parts are interpreted as :<br>
        *         - [  0, 1/8] : left edge
        *         - [1/8, 1/4] : right edge
        *         - [1/4, 3/8] : top edge (rotated 90 degrees ccw)
        *         - [3/8, 1/2] : bottom edge (rotated 90 degrees ccw)
        *         - [1/2, 5/8] : top-left corner
        *         - [5/8, 3/4] : top-right corner
        *         - [3/4, 7/8] : bottom-left corner
        *         - [7/8,   1] : bottom-right corner
        */
        void set_edge(const std::string& sEdgeFile);

        /// Returns this backdrop's edge file.
        /** \return This backdrop's edge file
        */
        const std::string& get_edge_file() const;

        /// Sets the edge color.
        /** \param mColor The edge color
        *   \note This color can be used to tint the edge texture if any
        *         or simply render a plain color edge.
        */
        void set_edge_color(const color& mColor);

        /// Returns the edge color.
        /** \return The edge color
        */
        color get_edge_color() const;

        /// Sets the appearent edge size.
        /** \param uiEdgeSize The new edget size
        *   \note Edges will be scaled by uiEdgeSize/edgeTextureHeight.
        */
        void set_edge_size(uint uiEdgeSize);

        /// Returns this backdrop's edge size.
        /** \return This backdrop's edge size
        */
        uint get_edge_size() const;

        /// Sets the color to be multiplied to all drawn vertices.
        /** \param mColor The new vertex color
        */
        void set_vertex_color(const color& mColor);

        /// Renders this backdrop on the current render target.
        void render() const;

    private :

        /// Defines the position of each edge sprite.
        enum class edge_type
        {
            LEFT = 0,
            RIGHT,
            TOP,
            BOTTOM,
            TOPLEFT,
            TOPRIGHT,
            BOTTOMLEFT,
            BOTTOMRIGHT
        };

        /// Return the sprite for a given edge.
        sprite& get_edge(edge_type mEdge) const;

        frame*      pParent_ = nullptr;

        bool        bHasBackground_ = false;
        std::string sBackgroundFile_;
        color       mBackgroundColor_ = color::EMPTY;

        bool        bHasEdge_ = false;
        std::string sEdgeFile_;
        color       mEdgeColor_ = color::EMPTY;

        mutable sprite               mBackground_;
        mutable std::array<sprite,8> lEdgeList_;

        bool   bBackgroundTilling_ = false;
        uint   uiTileSize_ = 0u;
        uint   uiOriginalTileSize_ = 0u;
        quad2i lBackgroundInsets_;
        quad2i lEdgeInsets_;
        uint   uiEdgeSize_ = 0u;
        uint   uiOriginalEdgeSize_ = 0u;
    };
}
}

#endif
