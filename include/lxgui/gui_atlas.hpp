#ifndef LXGUI_GUI_ATLAS_HPP
#define LXGUI_GUI_ATLAS_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_quad2.hpp"

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <optional>

namespace lxgui {
namespace gui
{
    class renderer;

    /// A single texture holding multiple materials for efficient rendering
    /** This is an abstract class that must be implemented
    *   and created by the corresponding gui::renderer.
    */
    class atlas_page
    {
    public :

        /// Constructor.
        explicit atlas_page(material::filter mFilter);

        /// Destructor.
        virtual ~atlas_page() = default;

        /// Find a material in this page (nullptr if not found).
        /** \param sFileName The name of the file
        *   \return The material (nullptr if not found)
        */
        std::shared_ptr<material> fetch_material(const std::string& sFileName) const;

        /// Creates a new material from a texture file.
        /** \param sFileName The name of the file
        *   \return The new material
        *   \note Supported texture formats are defined by implementation.
        *         The gui library is completely unaware of this.
        */
        std::shared_ptr<material> add_material(const std::string& sFileName, const material& mMat);

        /// Checks if this page is empty (contains no materials).
        /** \return 'true' if the page is empty, 'false' otherwise
        */
        bool empty() const;

    protected :

        /// Adds a new material to this page, at the provided location
        /** \param mMat      The material to add
        *   \param mLocation The position at which to insert this material
        *   \return A new material pointing to inside this page
        */
        virtual std::shared_ptr<material> add_material_(const material& mMat,
            const quad2f& mLocation) = 0;

        /// Return the width of this page (in pixels).
        /** \return The width of this page (in pixels)
        */
        virtual float get_width() const = 0;

        /// Return the height of this page (in pixels).
        /** \return The height of this page (in pixels)
        */
        virtual float get_height() const = 0;

        material::filter mFilter_ = material::filter::NONE;

    private :

        /// Try to insert a new texture into this page, and return the best position if any
        /** \param fWidth  The width of the texture to insert
        *   \param fHeight The height of the texture to insert
        *   \return The new position for this texture, or std::nullopt if it does not fit
        */
        std::optional<quad2f> find_location_(float fWidth, float fHeight) const;

        mutable std::unordered_map<std::string, std::weak_ptr<gui::material>> lTextureList_;
    };

    /// A class that holds multiple materials for efficient rendering
    /** This is an abstract class that must be implemented
    *   and created by the corresponding gui::renderer.
    */
    class atlas
    {
    public :

        /// Constructor.
        explicit atlas(const renderer& mRenderer, material::filter mFilter);

        /// Destructor.
        virtual ~atlas() = default;

        /// Find a material in this atlas (nullptr if not found).
        /** \param sFileName The name of the file
        *   \return The material (nullptr if not found)
        */
        std::shared_ptr<material> fetch_material(const std::string& sFileName) const;

        /// Creates a new material from a texture file.
        /** \param sFileName The name of the file from which the
        *   \param mMat The material to add to this atlas
        *   \return The new material
        *   \note Supported texture formats are defined by implementation.
        *         The gui library is completely unaware of this.
        */
        std::shared_ptr<material> add_material(const std::string& sFileName, const material& mMat) const;

        /// Return the number of pages in this atlas.
        /** \return The number of pages in this atlas
        */
        uint get_num_pages() const;

    protected :

        /// Create a new page in this atlas.
        /** \return The new page, added at the back of the page list
        */
        virtual std::unique_ptr<atlas_page> create_page_() const = 0;

        const renderer&  mRenderer_;
        material::filter mFilter_ = material::filter::NONE;

    private :

        /// Create a new page in this atlas.
        /** \return The new page, added at the back of the page list
        */
        void add_page_() const;

        mutable std::vector<std::unique_ptr<atlas_page>> lPageList_;
    };
}
}

#endif
