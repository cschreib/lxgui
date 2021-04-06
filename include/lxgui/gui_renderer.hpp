#ifndef LXGUI_GUI_RENDERER_HPP
#define LXGUI_GUI_RENDERER_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_strata.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_vertexcache.hpp"
#include "lxgui/gui_sprite.hpp"
#include "lxgui/gui_matrix4.hpp"

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

        /// Returns the display width of this renderer's main render target (e.g., screen).
        /** \return The render target width
        */
        virtual uint get_target_width() const = 0;

        /// Returns the display height of this renderer's main render target (e.g., screen).
        /** \return The render target height
        */
        virtual uint get_target_height() const = 0;

        /// Returns the physical width of this renderer's main render target (e.g., screen), in pixels.
        /** \return The render target width
        */
        virtual uint get_target_physical_pixel_width() const = 0;

        /// Returns the physical height of this renderer's main render target (e.g., screen), in pixels.
        /** \return The render target height
        */
        virtual uint get_target_physical_pixel_height() const = 0;

        /// Tells the underlying graphics engine to start rendering into a new target.
        /** \param pTarget The target to render to (nullptr to render to the screen)
        */
        void begin(std::shared_ptr<render_target> pTarget = nullptr) const;

        /// Tells the underlying graphics engine we're done rendering.
        /** \note For most engines, this is when the rendering is actually
        *         done, so don't forget to call it !
        */
        void end() const;

        /// Sets the global UI scaling factor.
        /** \param fScalingFactor The factor to use for rescaling (1: no rescaling, default)
        *   \note This value determines how to convert sizing units or position coordinates
        *         into actual number of pixels. By default, units specified for sizes and
        *         positions are 1:1 mapping with pixels on the screen. If designing the UI
        *         on a "traditional" display (say, 1080p resolution monitor), the UI will not
        *         scale correctly when running on high-DPI displays unless the scaling factor is
        *         adjusted accordingly. The value of the scaling factor should be the ratio
        *         DPI_target/DPI_dev, where DPI_dev is the DPI of the display used for
        *         development, and DPI_target is the DPI of the display used to run the program.
        *         In addition, the scaling factor can also be used to improve accessibility of
        *         the interface to users with poorer eye sight, which would benefit from larger
        *         font sizes and larger icons.
        */
        void set_interface_scaling_factor(float fScalingFactor);

        /// Returns the current UI scaling factor.
        /** \return The current UI scaling factor
        *   \see set_interface_scaling_factor()
        */
        float get_interface_scaling_factor() const;

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
        /** \param pMaterial The material to use for rendering, or null if none
        *   \param lQuadList The list of the quads you want to render
        *   \note This function is meant to be called between begin() and
        *         end() only. When multiple quads share the same material, it is
        *         always more efficient to call this method than calling render_quad
        *         repeatedly, as it allows to reduce the number of draw calls.
        */
        void render_quads(const material* pMaterial, const std::vector<std::array<vertex,4>>& lQuadList) const;

        /// Renders a vertex cache.
        /** \param pMaterial       The material to use for rendering, or null if none
        *   \param mCache          The vertex cache
        *   \param mModelTransform The transformation matrix to apply to vertices
        *   \note This function is meant to be called between begin() and
        *         end() only. When multiple quads share the same material, it is
        *         always more efficient to call this method than calling render_quad
        *         repeatedly, as it allows to reduce the number of draw calls. This method
        *         is also more efficient than render_quads(), as the vertex data is
        *         already cached to the GPU and does not need sending again. However,
        *         not all implementations support vertex caches. See has_vertex_cache().
        */
        void render_cache(const material* pMaterial, const vertex_cache& mCache,
            const matrix4f& mModelTransform = matrix4f::IDENTITY) const;

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
        /** \param mType The type of data this cache will hold
        *   \note Not all implementations support vertex caches. See has_vertex_cache().
        *         The size hint can enable the cache to be pre-allocated, which will avoid a
        *         reallocation when data is pushed to the cache.
        */
        std::shared_ptr<vertex_cache> create_vertex_cache(gui::vertex_cache::type mType) const;

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

        float fScalingFactor_ = 1.0f;
    };
}
}

#endif
