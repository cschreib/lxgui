#ifndef LXGUI_GUI_SDL_RENDERER_HPP
#define LXGUI_GUI_SDL_RENDERER_HPP

#include "lxgui/impl/gui_sdl_rendertarget.hpp"

#include <lxgui/gui_renderer_impl.hpp>
#include <lxgui/utils.hpp>

#include <map>
#include <memory>

struct SDL_Renderer;

namespace lxgui {
namespace gui {
namespace sdl
{
    class material;

    /// SDL implementation of rendering
    class renderer : public gui::renderer_impl
    {
    public :

        /// Constructor.
        explicit renderer(SDL_Renderer* pRenderer);

        /// Begins rendering on a particular render target.
        /** \param pTarget The render target (main screen if nullptr)
        */
        void begin(std::shared_ptr<gui::render_target> pTarget = nullptr) const override;

        /// Ends rendering.
        void end() const override;

        /// Renders a quad from a material and array of vertices.
        /** \param pMat        The material to use to to render the quad
        *   \param lVertexList The lsit of 4 vertices making up the quad
        *   \note This function is meant to be called between begin() and
        *         end() only.
        */
        void render_quad(std::shared_ptr<sdl::material> pMat,
            const std::array<vertex,4>& lVertexList) const;

        /// Renders a quad.
        /** \param mQuad The quad to render on the current render target
        *   \note This function is meant to be called between begin() and
        *         end() only.
        */
        void render_quad(const quad& mQuad) const override;

        /// Renders a set of quads.
        /** \param mQuad     The base quad to use for rendering (material, blending, ...)
        *   \param lQuadList The list of the quads you want to render
        *   \note This function is meant to be called between begin() and
        *         end() only. It is always more efficient to call this method
        *         than calling render_quad repeatedly, as it allows to batch
        *         count reduction.
        */
        void render_quads(const quad& mQuad, const std::vector<std::array<vertex,4>>& lQuadList) const override;

        /// Creates a new material from a texture file.
        /** \param sFileName The name of the file
        *   \param mFilter   The filtering to apply to the texture
        *   \return The new material
        *   \note Only PNG textures are supported by this implementation (parsed by libpng).
        */
        std::shared_ptr<gui::material> create_material(const std::string& sFileName,
            material::filter mFilter = material::filter::NONE) const override;

        /// Creates a new material from a plain color.
        /** \param mColor The color to use
        *   \return The new material
        */
        std::shared_ptr<gui::material> create_material(const color& mColor) const override;

        /// Creates a new material from a render target.
        /** \param pRenderTarget The render target from which to read the pixels
        *   \return The new material
        */
        std::shared_ptr<gui::material> create_material(std::shared_ptr<gui::render_target> pRenderTarget) const override;

        /// Creates a new render target.
        /** \param uiWidth  The width of the render target
        *   \param uiHeight The height of the render target
        */
        std::shared_ptr<gui::render_target> create_render_target(uint uiWidth, uint uiHeight) const override;

        /// Creates a new font.
        /** \param sFontFile The file from which to read the font
        *   \param uiSize    The requested size of the characters (in points)
        *   \note This implementation uses FreeType to load vector fonts and rasterize them.
        *         Bitmap fonts are not yet supported.
        */
        std::shared_ptr<gui::font> create_font(const std::string& sFontFile, uint uiSize) const override;

    private :

        SDL_Renderer* pRenderer_ = nullptr;

        mutable std::map<std::string, std::weak_ptr<gui::material>> lTextureList_;
        mutable std::map<std::string, std::weak_ptr<gui::font>>     lFontList_;

        mutable std::shared_ptr<gui::sdl::render_target> pCurrentTarget_;
    };
}
}
}

#endif
