#ifndef LXGUI_GUI_SDL_ATLAS_HPP
#define LXGUI_GUI_SDL_ATLAS_HPP

#include <lxgui/utils.hpp>
#include <lxgui/gui_material.hpp>
#include <lxgui/gui_atlas.hpp>

#include "lxgui/impl/gui_sdl_material.hpp"

#include <vector>
#include <memory>
#include <variant>

struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;

namespace lxgui {
namespace gui {
namespace sdl
{
    class renderer;

    /// A single texture holding multiple materials for efficient rendering
    /** This is an abstract class that must be implemented
    *   and created by the corresponding gui::renderer.
    */
    class atlas_page final : public gui::atlas_page
    {
    public :

        /// Constructor.
        explicit atlas_page(const renderer& mRenderer, material::filter mFilter);

        /// Destructor.
        ~atlas_page() override;

    protected :

        /// Adds a new material to this page, at the provided location
        /** \param mMat      The material to add
        *   \param mLocation The position at which to insert this material
        *   \return A new material pointing to inside this page
        */
        std::shared_ptr<gui::material> add_material_(const gui::material& mMat,
            const quad2f& mLocation) override;

        /// Return the width of this page (in pixels).
        /** \return The width of this page (in pixels)
        */
        float get_width() const override;

        /// Return the height of this page (in pixels).
        /** \return The height of this page (in pixels)
        */
        float get_height() const override;

    private :

        const renderer& mRenderer_;
        SDL_Texture*    pTexture_ = nullptr;
        uint            uiSize_ = 0u;
    };

    /// A class that holds rendering data
    /** This implementation can contain either a plain color
    *   or a real SDL_Texture. It is also used by the
    *   gui::sdl::render_target class to store the output data.
    */
    class atlas final : public gui::atlas
    {
    public :

        /// Constructor for textures.
        /** \parem mRenderer The renderer with witch to create this atlas
        *   \param mFilter   Use texture filtering or not (see set_filter())
        */
        explicit atlas(const renderer& mRenderer, material::filter mFilter);

        atlas(const atlas& tex) = delete;
        atlas(atlas&& tex) = delete;
        atlas& operator = (const atlas& tex) = delete;
        atlas& operator = (atlas&& tex) = delete;

    protected :

        /// Create a new page in this atlas.
        /** \return The new page, added at the back of the page list
        */
        std::unique_ptr<gui::atlas_page> create_page_() const override;

    private :

        const renderer& mSDLRenderer_;
    };
}
}
}

#endif
