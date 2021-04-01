// # Note # :
// This class is inspired by HGE's hgeSprite class.
// HGE is a simple and fast 2D drawing library. I have
// been using it before switching to Ogre3D, and I
// really liked the synthax.
//
// More infos about HGE :
// http://hge.relishgames.com/

#ifndef LXGUI_GUI_SPRITE_HPP
#define LXGUI_GUI_SPRITE_HPP

#include <lxgui/utils.hpp>

#include "lxgui/gui_vector2.hpp"
#include "lxgui/gui_color.hpp"
#include "lxgui/gui_material.hpp"

#include <array>
#include <vector>
#include <memory>

namespace lxgui {
namespace gui
{
    class renderer;
    class render_target;
    class material;

    struct vertex
    {
        vertex() = default;

        vertex(const vertex&) = default;
        vertex(vertex&&) = default;

        vertex(const vector2f& mPos, const vector2f& mUV, const color& mCol);

        vertex& operator=(const vertex&) = default;
        vertex& operator=(vertex&&) = default;

        vector2f pos;
        vector2f uvs;
        color    col = color::WHITE;
    };

    enum class blend_mode
    {
        NORMAL,
        ADD,
        MUL
    };

    struct quad
    {
        std::array<vertex, 4>     v;
        std::shared_ptr<material> mat;
        blend_mode                blend;
    };

    /// Use to draw a texture on the screen
    /** This class is meant to simplify rendering of
    *   2D elements. Everything can be done with a quad
    *   struct, but it's really not simple to use.<br>
    *   Thanks to this class, the only thing you have to do
    *   to acheive interesting effects is calling 2 or 3
    *   functions and let the magic do the rest.
    *   \note This is a class, that *can* be
    *         inherited from if needed, and created by
    *         manager_impl.
    */
    class sprite
    {
    public :

        /// Default constructor.
        sprite() = default;

        /// Constructor.
        sprite(const renderer* pRenderer, std::shared_ptr<material> pMat);

        /// Constructor.
        sprite(const renderer* pRenderer, std::shared_ptr<material> pMat, float fWidth, float fHeight);

        /// Constructor.
        sprite(const renderer* pRenderer, std::shared_ptr<material> pMat, float fU, float fV, float fWidth, float fHeight);

        /// Renders this sprite on the current render target.
        /** \param fX The horizontal position
        *   \param fY The vertical position
        *   \note Must be called between begin() and end().
        */
        void render(float fX, float fY) const;

        /// Deforms this sprite and render it on the current render target.
        /** \param fX      The horizontal position
        *   \param fY      The vertical position
        *   \param fRot    The rotation to apply (angle in radian)
        *   \param fHScale The horizontal scale to apply
        *   \param fVScale The vertical scale to apply
        *   \note Must be called between begin() and end().
        */
        void render_ex(float fX, float fY,
                      float fRot,
                      float fHScale = 1.0f, float fVScale = 1.0f) const;

        /// Stretches this this sprite and render it on the current render target.
        /** \param fX1 The top-left corner horizontal position
        *   \param fY1 The top-left corner vertical position
        *   \param fX3 The bottom-right corner horizontal position
        *   \param fY3 The bottom-right corner vertical position
        *   \note Must be called between begin() and end().
        */
        void render_2v(float fX1, float fY1,
                      float fX3, float fY3);

        /// Stretches this this sprite and render it on the current render target.
        /** \param fX1 The top-left corner horizontal position
        *   \param fY1 The top-left corner vertical position
        *   \param fX2 The top-right corner horizontal position
        *   \param fY2 The top-right corner vertical position
        *   \param fX3 The bottom-right corner horizontal position
        *   \param fY3 The bottom-right corner vertical position
        *   \param fX4 The bottom-left corner horizontal position
        *   \param fY4 The bottom-left corner vertical position
        *   \note Must be called between begin() and end().
        */
        void render_4v(float fX1, float fY1,
                      float fX2, float fY2,
                      float fX3, float fY3,
                      float fX4, float fY4);

        /// Renders this sprite with the same parameter than the last render call.
        /** \note This function is here for performance, when you want to set the
        *         internal quad once and for all.
        */
        void render_static() const;

        /// Sets this sprite's internal quad.
        /** \param lVertexArray The new quad
        *   \note This quad will be overwritten by any render_XXX() call.
        *         If you want to render it, call render_static().
        */
        void set_quad(const std::array<vertex,4>& lVertexArray);

        /// Changes the color of this sprite.
        /** \param mColor The new color
        *   \param uiIndex The index of the vertice to change
        *   \note If you provide an index, this function will only change
        *         a single vertex's color.<br>
        *         Index 0 is for top left, index 1 is for top right, ...
        */
        void set_color(const color& mColor, uint uiIndex = (uint)(-1));

        /// Makes this sprite colorless.
        /** \param bDesaturated 'true' to desaturate the texture/color
        *   \note Depending on the implementation, this method may resort to
        *         using pixel shaders.
        */
        void set_desaturated(bool bDesaturated);

        /// Sets the blending mode of this sprite.
        /** \param mBlendMode The new blending mode
        *   \note See blend_mode.
        */
        void set_blend_mode(blend_mode mBlendMode);

        /// Changes this sprite's center.
        /** \param mHotSpot A 2D point containing the new center's position
        *   \note HotSpot is used to rotate and scale your sprite with render_ex().<br>
        *         It is also considered as the reference point when you call render()
        *         (same goes for render_ex()).
        */
        void set_hot_spot(const vector2f& mHotSpot);

        /// Changes this sprite's center.
        /** \param fX The new center's horizontal position
        *   \param fY The new center's vertical position
        *   \note HotSpot is used to rotate and scale your sprite with render_ex().<br>
        *         It is also considered as the reference point when you call render()
        *         (same goes for render_ex()).
        */
        void set_hot_spot(float fX, float fY);

        /// Changes this sprite's texture rectangle.
        /** \param lTextureRect The new texture rect
        *   \param bNormalized  'true' if the coords are already clamped to [0, 1]
        *   \note Texture rectangle is the zone of the texture you want to display.<br>
        *         Note that it doesn't need to be adjusted to this sprite's dimensions.
        *         The texture will then be stretched to fit the sprite's dimensions.
        */
        void set_texture_rect(const std::array<float,4>& lTextureRect, bool bNormalized = false);

        /// Changes this sprite's texture rectangle.
        /** \param fX1 The rect's top left horizontal position
        *   \param fY1 The rect's top left vertical position
        *   \param fX3 The rect's bottom right horizontal position
        *   \param fY3 The rect's bottom right vertical position
        *   \param bNormalized  'true' if the coords are already clamped to [0, 1]
        *   \note Texture rectangle is the zone of the texture you want to display.<br>
        *         Note that it doesn't need to be adjusted to this sprite's dimensions.
        *         The texture will then be stretched to fit the sprite's dimensions.
        */
        void set_texture_rect(float fX1, float fY1, float fX3, float fY3, bool bNormalized = false);

        /// Changes this sprite's texture coordinates.
        /** \param lTextureCoords The new texture coordinates
        *   \param bNormalized  'true' if the coords are already converted to texture space
        *   \note Texture rectangle is the zone of the texture you want to display.<br>
        *         Note that it doesn't need to be adjusted to this sprite's dimensions.
        *         The texture will then be stretched to fit the sprite's dimensions.
        */
        void set_texture_coords(const std::array<float,8>& lTextureCoords, bool bNormalized = false);

        /// Changes this sprite's texture coordinates.
        /** \param fX1 The sprites's top left horizontal position
        *   \param fY1 The sprites's top left vertical position
        *   \param fX2 The sprites's top right horizontal position
        *   \param fY2 The sprites's top right vertical position
        *   \param fX3 The sprites's bottom right horizontal position
        *   \param fY3 The sprites's bottom right vertical position
        *   \param fX4 The sprites's bottom left horizontal position
        *   \param fY4 The sprites's bottom left vertical position
        *   \param bNormalized  'true' if the coords are already converted to texture space
        *   \note Texture rectangle is the zone of the texture you want to display.<br>
        *         Note that it doesn't need to be adjusted to this sprite's dimensions.
        *         The texture will then be stretched to fit the sprite's dimensions.
        */
        void set_texture_coords(float fX1, float fY1,
                              float fX2, float fY2,
                              float fX3, float fY3,
                              float fX4, float fY4,
                              bool bNormalized = false);

        /// Changes this sprite's dimensions.
        /** \param fWidth      The new width
        *   \param fHeight     The new height
        *   \note If you adjust texture coordinates, you texture won't be deformed.
        *         Else, it will be streched to fit the new dimensions.
        */
        void set_dimensions(float fWidth, float fHeight);

        /// Returns this sprite's width.
        /** \return This sprite's width
        */
        float get_width() const;

        /// Returns this sprite's height.
        /** \return This sprite's height
        */
        float get_height() const;

        /// Returns this sprite's color.
        /** \return This sprite's color
        */
        color get_color() const;

        /// Returns this sprite's blend mode.
        /** \return This sprite's blend mode
        */
        blend_mode get_blend_mode() const;

        /// Returns this sprite's texture rectangle.
        /** \return This sprite's texture rectangle
        */
        std::array<float,4> get_texture_rect() const;

        /// Returns this sprite's texture coordinates.
        /** \param bNormalized 'true' to get coordinates converted to texture space
        *   \return This sprite's texture coordinates
        */
        std::array<float,8> get_texture_coords(bool bNormalized = false) const;

    private :

        const renderer* pRenderer_ = nullptr;

        mutable quad mQuad_;
        vector2f     mHotSpot_;
        float        fWidth_ = 0.0f, fHeight_ = 0.0f;
    };
}
}

#endif
