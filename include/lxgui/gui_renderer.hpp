#ifndef LXGUI_GUI_RENDERER_HPP
#define LXGUI_GUI_RENDERER_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_strata.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_sprite.hpp"

namespace lxgui {
namespace gui
{
    class frame;
    class font;
    class vertex_cache;
    class color;
    class renderer_impl;
    struct matrix4f;

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

        /// Returns the width of this renderer's main render target (e.g., screen).
        /** \return The render target width
        */
        virtual uint get_target_width() const = 0;

        /// Returns the height of this renderer's main render target (e.g., screen).
        /** \return The render target height
        */
        virtual uint get_target_height() const = 0;

        /// Tells the underlying graphics engine to start rendering into a new target.
        /** \param pTarget The target to render to (nullptr to render to the screen)
        */
        void begin(std::shared_ptr<render_target> pTarget = nullptr) const;

        /// Tells the underlying graphics engine we're done rendering.
        /** \note For most engines, this is when the rendering is actually
        *         done, so don't forget to call it !
        */
        void end() const;

        /// Sets the view matrix to use when rendering (viewport).
        /** \param mViewMatrix The view matrix
        *   \note This function is called by default in begin(), which resets the
        *         view to span the entire render target (or the entire screen). Therefore
        *         it is only necessary to use this function when a custom view is required.
        *         The view matrix converts custom "world" coordinates into screen-space
        *         coordinates, where the X and Y coordinates represent the horizontal and
        *         vertical dimensions on the screen, respectively, and range from -1 to +1.
        *         In screen-space coordinates, the top-left corner of the screen has
        *         coordinates (-1,-1), and the bottom-left corner of the screen is (+1,+1).
        *   \warning Although the view is specified here as a matrix for maximum flexibility,
        *            some backends do not actually support arbitrary view matrices. For such
        *            backends, the view matrix will be simplified to a simpler 2D translate +
        *            rotate + scale transform, or even just translate + scale.
        */
        void set_view(const matrix4f& mViewMatrix) const;

        /// Renders a quad.
        /** \param mQuad The quad to render on the current render target
        *   \note This function is meant to be called between begin() and
        *         end() only.
        */
        void render_quad(const quad& mQuad) const;

        /// Renders a set of quads.
        /** \param mMaterial The material to use for rendering (texture, color, blending, ...)
        *   \param lQuadList The list of the quads you want to render
        *   \note This function is meant to be called between begin() and
        *         end() only. It is always more efficient to call this method
        *         than calling render_quad repeatedly, as it allows to batch
        *         count reduction.
        */
        void render_quads(const material& mMaterial, const std::vector<std::array<vertex,4>>& lQuadList) const;

        /// Creates a new sprite.
        /** \param pMat The material with which to create the sprite
        *   \return The new sprite
        */
        sprite create_sprite(std::shared_ptr<material> pMat) const;

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
        sprite create_sprite(std::shared_ptr<material> pMat, float fWidth, float fHeight) const;

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
        sprite create_sprite(std::shared_ptr<material> pMat,
            float fU, float fV, float fWidth, float fHeight) const;

        /// Creates a new material from a texture file.
        /** \param sFileName The name of the file
        *   \param mFilter   The filtering to apply to the texture
        *   \return The new material
        *   \note Supported texture formats are defined by implementation.
        *         The gui library is completely unaware of this.
        */
        std::shared_ptr<material> create_material(const std::string& sFileName,
            material::filter mFilter = material::filter::NONE) const;

        /// Creates a new material from a plain color.
        /** \param mColor The color to use
        *   \return The new material
        */
        std::shared_ptr<material> create_material(const color& mColor) const;

        /// Creates a new material from a render target.
        /** \param pRenderTarget The render target from which to read the pixels
        *   \return The new material
        */
        std::shared_ptr<material> create_material(std::shared_ptr<render_target> pRenderTarget) const;

        /// Creates a new render target.
        /** \param uiWidth  The width of the render target
        *   \param uiHeight The height of the render target
        */
        std::shared_ptr<render_target> create_render_target(uint uiWidth, uint uiHeight) const;

        /// Creates a new font.
        /** \param sFontFile The file from which to read the font
        *   \param uiSize    The requested size of the characters (in points)
        *   \note Even though the gui has been designed to use vector fonts files
        *         (such as .ttf or .otf font formats), nothing prevents the implementation
        *         from using any other font type, including bitmap fonts.
        */
        std::shared_ptr<font> create_font(const std::string& sFontFile, uint uiSize) const;

        /// Checks if the renderer supports vertex caches.
        /** \return 'true' if supported, 'false' otherwise
        */
        bool has_vertex_cache() const;

        /// Creates a new empty vertex cache.
        /** \param pMaterial The material to use to render the vertices
        *   \note Not all implementations support vertex caches. See has_vertex_cache().
        */
        std::shared_ptr<vertex_cache> create_vertex_cache(std::shared_ptr<material> pMaterial) const;

    protected :

        void add_to_strata_list_(strata& mStrata, frame* pFrame);
        void remove_from_strata_list_(strata& mStrata, frame* pFrame);
        void add_to_level_list_(level& mLevel, frame* pFrame);
        void remove_from_level_list_(level& mLevel, frame* pFrame);
        void clear_strata_list_();
        bool has_strata_list_changed_() const;
        void reset_strata_list_changed_flag_();

        void render_strata_(const strata& mStrata) const;
        void create_strata_cache_render_target_(strata& mStrata);

        frame* find_hovered_frame_(int iX, int iY);

        renderer_impl*       pImpl_ = nullptr;
        std::array<strata,8> lStrataList_;
        bool                 bStrataListUpdated_ = false;
    };
}
}

#endif
