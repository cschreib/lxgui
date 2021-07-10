#ifndef LXGUI_GUI_ATLAS_DEFAULT_HPP
#define LXGUI_GUI_ATLAS_DEFAULT_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_atlas.hpp"
#include "lxgui/gui_rendertarget.hpp"

namespace lxgui {
namespace gui
{
    /// Default implementation for atlas_page.
    /** This is a default implementation using render_target.
    *   Rendering back-ends can provide their own faster implementation.
    */
    class atlas_page_default final : public atlas_page
    {
    public :

        /// Constructor.
        explicit atlas_page_default(const renderer& mRenderer, material::filter mFilter);

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

        const renderer&                     mRenderer_;
        std::shared_ptr<gui::render_target> pTarget_;
    };


    /// Default implementation for atlas.
    /** This is a default implementation using render_target.
    *   Rendering back-ends can provide their own faster implementation.
    */
    class atlas_default final : public atlas
    {
    public :

        /// Constructor.
        explicit atlas_default(const renderer& mRenderer, material::filter mFilter);

    protected :

        /// Create a new page in this atlas.
        /** \return The new page, added at the back of the page list
        */
        std::unique_ptr<atlas_page> create_page_() const override;
    };
}
}

#endif
