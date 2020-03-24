#ifndef GUI_GL_MANAGER_HPP
#define GUI_GL_MANAGER_HPP

#include "lxgui/impl/gui_gl_matrix4.hpp"
#include "lxgui/impl/gui_gl_rendertarget.hpp"

#include <lxgui/utils.hpp>
#include <lxgui/gui_manager.hpp>

namespace gui {
namespace gl
{
    /// Open implementation of rendering
    class manager : public gui::manager_impl
    {
    public :

        /// Constructor.
        manager(bool bInitGLEW = true);

        /// Destructor.
        ~manager();

        /// Begins rendering on a particular render target.
        /** \param pTarget The render target (main screen if nullptr)
        */
        void begin(utils::refptr<gui::render_target> pTarget = nullptr) const override;

        /// Ends rendering.
        void end() const override;

        /// Renders a quad.
        /** \param mQuad The quad to render on the current render target
        *   \note This function is meant to be called between begin() and
        *         end() only.
        */
        virtual void render_quad(const quad& mQuad) const override;

        /// Renders a set of quads.
        /** \param mQuad     The base quad to use for rendering (material, blending, ...)
        *   \param lQuadList The list of the quads you want to render
        *   \note This function is meant to be called between begin() and
        *         end() only. It is always more efficient to call this method
        *         than calling render_quad repeatedly, as it allows to batch
        *         count reduction.
        */
        virtual void render_quads(const quad& mQuad, const std::vector<std::array<vertex,4>>& lQuadList) const override;

        /// Creates a new material from a texture file.
        /** \param sFileName The name of the file
        *   \param mFilter   The filtering to apply to the texture
        *   \return The new material
        *   \note Only PNG textures are supported by this implementation (parsed by libpng).
        */
        utils::refptr<gui::material> create_material(const std::string& sFileName, filter mFilter = FILTER_NONE) const override;

        /// Creates a new material from a plain color.
        /** \param mColor The color to use
        *   \return The new material
        */
        utils::refptr<gui::material> create_material(const color& mColor) const override;

        /// Creates a new material from a render target.
        /** \param pRenderTarget The render target from which to read the pixels
        *   \return The new material
        */
        utils::refptr<gui::material> create_material(utils::refptr<gui::render_target> pRenderTarget) const override;

        /// Creates a new render target.
        /** \param uiWidth  The width of the render target
        *   \param uiHeight The height of the render target
        */
        utils::refptr<gui::render_target> create_render_target(uint uiWidth, uint uiHeight) const override;

        /// Creates a new font.
        /** \param sFontFile The file from which to read the font
        *   \param uiSize    The requested size of the characters (in points)
        *   \note This implementation uses FreeType to load vector fonts and rasterize them.
        *         Bitmap fonts are not yet supported.
        */
        utils::refptr<gui::font> create_font(const std::string& sFontFile, uint uiSize) const override;

        /// Checks if a given OpenGL extension is supported by the machine.
        /** \return 'true' if that is the case, 'false' else.
        */
        static bool is_gl_extension_supported(const std::string& sExtension);

    private :

        void update_view_matrix_() const;

        utils::refptr<gui::material> create_material_png(const std::string& sFileName, filter mFilter) const;

        mutable std::map<std::string, utils::wptr<gui::material>> lTextureList_;
        mutable std::map<std::string, utils::wptr<gui::font>>     lFontList_;

        mutable bool    bUpdateViewMatrix_;
        mutable matrix4 mViewMatrix_;

        mutable utils::refptr<gui::gl::render_target> pCurrentTarget_;
    };
}
}

#endif
