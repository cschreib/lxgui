#ifndef LXGUI_GUI_BACKDROP_HPP
#define LXGUI_GUI_BACKDROP_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_vertexcache.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/gui_bounds2.hpp"

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
        explicit backdrop(frame& pParent);

        /// Non-copiable
        backdrop(const backdrop&) = delete;

        /// Non-movable
        backdrop(backdrop&&) = delete;

        /// Non-copiable
        backdrop& operator=(const backdrop&) = delete;

        /// Non-movable
        backdrop& operator=(backdrop&&) = delete;

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
        void set_background_tilling(bool bBackgroundTilling);

        /// Checks if tilling is enabled for the background texture.
        /** \return 'true' if tilling is enabled for the background texture
        */
        bool is_background_tilling() const;

        /// Sets the appearent tile size.
        /** \param fTileSize The new tile size
        *   \note Tile will be scaled by fTileSize/backgroundTextureSize.
        */
        void set_tile_size(float fTileSize);

        /// Returns this backdrop's tile size.
        /** \return This backdrop's tile size
        */
        float get_tile_size() const;

        /// Sets insets for the background texture.
        /** \param lInsets The insets array
        */
        void set_background_insets(const bounds2f& lInsets);

        /// Returns this backdrop's background insets.
        /** \return This backdrop's background insets
        */
        const bounds2f& get_background_insets() const;

        /// Sets insets for the edge texture.
        /** \param lInsets The insets array
        */
        void set_edge_insets(const bounds2f& lInsets);

        /// Returns this backdrop's edge insets.
        /** \return This backdrop's edge insets
        */
        const bounds2f& get_edge_insets() const;

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
        /** \param fEdgeSize The new edge size
        *   \note Edges will be scaled by fEdgeSize/edgeTextureHeight.
        */
        void set_edge_size(float fEdgeSize);

        /// Returns this backdrop's edge size.
        /** \return This backdrop's edge size
        */
        float get_edge_size() const;

        /// Sets the color to be multiplied to all drawn vertices.
        /** \param mColor The new vertex color
        */
        void set_vertex_color(const color& mColor);

        /// Renders this backdrop on the current render target.
        void render() const;

        /// Tells this backdrop that its parent frame has changed dimensions.
        void notify_borders_updated() const;

    private :

        void update_cache_() const;
        void update_background_(color mColor) const;
        void update_edge_(color mColor) const;

        frame& mParent_;

        std::string               sBackgroundFile_;
        color                     mBackgroundColor_ = color::EMPTY;
        std::shared_ptr<material> pBackgroundTexture_;
        bool                      bBackgroundTilling_ = false;
        float                     fTileSize_ = 0.0f;
        float                     fOriginalTileSize_ = 0.0f;
        bounds2f                  lBackgroundInsets_;

        std::string               sEdgeFile_;
        color                     mEdgeColor_ = color::EMPTY;
        std::shared_ptr<material> pEdgeTexture_;
        bounds2f                  lEdgeInsets_;
        float                     fEdgeSize_ = 0.0f;
        float                     fOriginalEdgeSize_ = 0.0f;

        color mVertexColor_ = color::WHITE;

        mutable bool bCacheDirty_ = true;
        mutable float fCacheAlpha_ = std::numeric_limits<float>::quiet_NaN();
        mutable std::vector<std::array<vertex,4>> lBackgroundQuads_;
        mutable std::shared_ptr<vertex_cache>     pBackgroundCache_;
        mutable std::vector<std::array<vertex,4>> lEdgeQuads_;
        mutable std::shared_ptr<vertex_cache>     pEdgeCache_;
    };
}
}

#endif
