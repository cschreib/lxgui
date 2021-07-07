#ifndef LXGUI_GUI_GL_ATLAS_HPP
#define LXGUI_GUI_GL_ATLAS_HPP

#include <lxgui/utils.hpp>
#include <lxgui/gui_material.hpp>
#include <lxgui/gui_atlas.hpp>

#include <vector>
#include <memory>

namespace lxgui {
namespace gui {
namespace gl
{
    /// A single texture holding multiple materials for efficient rendering
    /** This is an abstract class that must be implemented
    *   and created by the corresponding gui::renderer.
    */
    class atlas_page final : public gui::atlas_page
    {
    public :

        /// Constructor.
        explicit atlas_page(material::filter mFilter);

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
    };

    /// A class that holds rendering data
    /** This implementation can contain either a plain color
    *   or a real OpenGL texture. It is also used by the
    *   gui::gl::render_target class to store the output data.
    */
    class atlas final : public gui::atlas
    {
    public :

        /// Constructor for textures.
        /** \param mFilter Use texture filtering or not (see set_filter())
        */
        explicit atlas(material::filter mFilter);

        atlas(const atlas& tex) = delete;
        atlas(atlas&& tex) = delete;
        atlas& operator = (const atlas& tex) = delete;
        atlas& operator = (atlas&& tex) = delete;

        /// Destructor.
        ~atlas() override;

    protected :

        /// Create a new page in this atlas.
        /** \return The new page, added at the back of the page list
        */
        std::unique_ptr<gui::atlas_page> create_page_() const override;
    };
}
}
}

#endif
