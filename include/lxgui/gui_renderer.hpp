#ifndef LXGUI_GUI_RENDERER_HPP
#define LXGUI_GUI_RENDERER_HPP

#include <lxgui/utils.hpp>
#include "lxgui/gui_strata.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_sprite.hpp"

namespace lxgui {
namespace gui
{
    class frame;
    class font;
    class color;
    class renderer_impl;

    /// GUI renderer
    /** Abstract interface for object that can render frames.
    */
    class renderer
    {
    public :

        /// Constructor.
        explicit renderer(renderer_impl* pImpl);

        /// Destructor
        virtual ~renderer() = default;

        /// Tells this renderer that one of its widget requires redraw.
        virtual void fire_redraw(frame_strata mStrata) const;

        /// Tells this renderer that it should (or not) render another frame.
        /** \param pFrame    The frame to render
        *   \param bRendered 'true' if this renderer needs to render that new object
        */
        virtual void notify_rendered_frame(frame* pFrame, bool bManuallyRendered);

        /// Tells this renderer that a frame has changed strata.
        /** \param pFrame The frame which has changed
        *   \param mOldStrata The old frame strata
        *   \param mNewStrata The new frame strata
        */
        virtual void notify_frame_strata_changed(frame* pFrame, frame_strata mOldStrata,
            frame_strata mNewStrata);

        /// Tells this renderer that a frame has changed level.
        /** \param pFrame The frame which has changed
        *   \param iOldLevel The old frame level
        *   \param iNewLevel The new frame level
        */
        virtual void notify_frame_level_changed(frame* pFrame, int iOldLevel, int iNewLevel);

        /// Renders a quad.
        /** \param mQuad The quad to render on the current render target
        *   \note This function is meant to be called between begin() and
        *         end() only.
        */
        void render_quad(const quad& mQuad) const;

        /// Renders a set of quads.
        /** \param mQuad     The base quad to use for rendering (material, blending, ...)
        *   \param lQuadList The list of the quads you want to render
        *   \note This function is meant to be called between begin() and
        *         end() only. It is always more efficient to call this method
        *         than calling render_quad repeatedly, as it allows to batch
        *         count reduction.
        */
        void render_quads(const quad& mQuad, const std::vector<std::array<vertex,4>>& lQuadList) const;

        /// Creates a new sprite.
        /** \param pMat The material with which to create the sprite
        *   \return The new sprite
        */
        sprite create_sprite(utils::refptr<material> pMat) const;

        /// Creates a new sprite.
        /** \param pMat    The material with which to create the sprite
        *   \param fWidth  The width of the sprite
        *   \param fHeight The height of the sprite
        *   \note If the width and height you provide are smaller than
        *         the texture's ones, the texture will be cut on the right
        *         and bottom edges.<br>
        *         However, if they are larger than the texture's one, the
        *         texture will be tiled.
        *   \return The new sprite
        */
        sprite create_sprite(utils::refptr<material> pMat, float fWidth, float fHeight) const;

        /// Creates a new sprite.
        /** \param pMat    The material with which to create the sprite
        *   \param fU      The top left corner of the sprite in the material
        *   \param fV      The top left corner of the sprite in the material
        *   \param fWidth  The width of the sprite
        *   \param fHeight The height of the sprite
        *   \note If the width and height you provide are smaller than
        *         the texture's ones, the texture will be cut on the right
        *         and bottom edges.<br>
        *         However, if they are larger than the texture's one, the
        *         texture will be tiled.
        *   \return The new sprite
        */
        sprite create_sprite(utils::refptr<material> pMat,
            float fU, float fV, float fWidth, float fHeight) const;

        /// Creates a new material from a texture file.
        /** \param sFileName The name of the file
        *   \param mFilter   The filtering to apply to the texture
        *   \return The new material
        *   \note Supported texture formats are defined by implementation.
        *         The gui library is completely unaware of this.
        */
        utils::refptr<material> create_material(const std::string& sFileName,
            material::filter mFilter = material::filter::NONE) const;

        /// Creates a new material from a plain color.
        /** \param mColor The color to use
        *   \return The new material
        */
        utils::refptr<material> create_material(const color& mColor) const;

        /// Creates a new material from a render target.
        /** \param pRenderTarget The render target from which to read the pixels
        *   \return The new material
        */
        utils::refptr<material> create_material(utils::refptr<render_target> pRenderTarget) const;

        /// Creates a new render target.
        /** \param uiWidth  The width of the render target
        *   \param uiHeight The height of the render target
        */
        utils::refptr<render_target> create_render_target(uint uiWidth, uint uiHeight) const;

        /// Creates a new font.
        /** \param sFontFile The file from which to read the font
        *   \param uiSize    The requested size of the characters (in points)
        *   \note Even though the gui has been designed to use vector fonts files
        *         (such as .ttf or .otf font formats), nothing prevents the implementation
        *         from using any other font type, including bitmap fonts.
        */
        utils::refptr<font> create_font(const std::string& sFontFile, uint uiSize) const;

    protected :

        void add_to_strata_list_(strata& mStrata, frame* pFrame);
        void remove_from_strata_list_(strata& mStrata, frame* pFrame);
        void add_to_level_list_(level& mLevel, frame* pFrame);
        void remove_from_level_list_(level& mLevel, frame* pFrame);
        void clear_strata_list_();
        bool has_strata_list_changed_() const;
        void reset_strata_list_changed_flag_();

        void render_strata_(const strata& mStrata) const;
        void create_strata_render_target_(strata& mStrata, uint uiWidth, uint uiHeight);

        frame* find_hovered_frame_(int iX, int iY);

        renderer_impl*       pImpl_ = nullptr;
        std::array<strata,8> lStrataList_;
        bool                 bStrataListUpdated_ = false;
    };
}
}

#endif
