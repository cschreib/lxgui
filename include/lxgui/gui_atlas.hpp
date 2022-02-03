#ifndef LXGUI_GUI_ATLAS_HPP
#define LXGUI_GUI_ATLAS_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_bounds2.hpp"

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <optional>

namespace lxgui {
namespace gui
{
    class renderer;
    class font;

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

        /// Non-copiable
        atlas_page(const atlas_page&) = delete;

        /// Non-movable
        atlas_page(atlas_page&&) = delete;

        /// Non-copiable
        atlas_page& operator=(const atlas_page&) = delete;

        /// Non-movable
        atlas_page& operator=(atlas_page&&) = delete;

        /// Find a material in this page (nullptr if not found).
        /** \param sFileName The name of the file
        *   \return The material (nullptr if not found)
        */
        std::shared_ptr<material> fetch_material(const std::string& sFileName) const;

        /// Creates a new material from a texture file.
        /** \param sFileName The name of the file
        *   \param mMat      The material to add to this page
        *   \return The new material (or nullptr if the material could not fit)
        */
        std::shared_ptr<material> add_material(const std::string& sFileName, const material& mMat);

        /// Find a font in this page (nullptr if not found).
        /** \param sFontName The name+size of the font
        *   \return The font (nullptr if not found)
        */
        std::shared_ptr<font> fetch_font(const std::string& sFontName) const;

        /// Creates a new font from a texture file.
        /** \param sFontName The name of the file
        *   \param pFont     The font to add to this page
        *   \return The new font (or nullptr if the font could not fit)
        */
        bool add_font(const std::string& sFontName, std::shared_ptr<gui::font> pFont);

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
            const bounds2f& mLocation) = 0;

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
        std::optional<bounds2f> find_location_(float fWidth, float fHeight) const;

        std::unordered_map<std::string, std::weak_ptr<gui::material>> lTextureList_;
        std::unordered_map<std::string, std::weak_ptr<gui::font>>     lFontList_;
    };

    /// A class that holds multiple materials for efficient rendering
    /** This is an abstract class that must be implemented
    *   and created by the corresponding gui::renderer.
    */
    class atlas
    {
    public :

        /// Constructor.
        explicit atlas(renderer& mRenderer, material::filter mFilter);

        /// Destructor.
        virtual ~atlas() = default;

        /// Non-copiable
        atlas(const atlas&) = delete;

        /// Non-movable
        atlas(atlas&&) = delete;

        /// Non-copiable
        atlas& operator=(const atlas&) = delete;

        /// Non-movable
        atlas& operator=(atlas&&) = delete;

        /// Find a material in this atlas (nullptr if not found).
        /** \param sFileName The name of the file
        *   \return The material (nullptr if not found)
        */
        std::shared_ptr<material> fetch_material(const std::string& sFileName) const;

        /// Add a new material to the atlas.
        /** \param sFileName The name of the file
        *   \param mMat      The material to add to this atlas
        *   \return The new material
        */
        std::shared_ptr<material> add_material(const std::string& sFileName, const material& mMat);

        /// Find a font in this atlas (nullptr if not found).
        /** \param sFontName The name of the font+size
        *   \return The font (nullptr if not found)
        */
        std::shared_ptr<font> fetch_font(const std::string& sFontName) const;

        /// Add a new font to the atlas.
        /** \param sFontName The name of the font+size
        *   \param pFont     The font to add to this atlas
        *   \return 'true' if the font was added to this atlas, 'false' otherwise
        */
        bool add_font(const std::string& sFontName, std::shared_ptr<gui::font> pFont);

        /// Return the number of pages in this atlas.
        /** \return The number of pages in this atlas
        */
        std::size_t get_num_pages() const;

    protected :

        /// Create a new page in this atlas.
        /** \return The new page, added at the back of the page list
        */
        virtual std::unique_ptr<atlas_page> create_page_() = 0;

        renderer& mRenderer_;
        material::filter mFilter_ = material::filter::NONE;

    private :

        /// Create a new page in this atlas.
        /** \return The new page, added at the back of the page list
        */
        void add_page_();

        struct page_item
        {
            std::unique_ptr<atlas_page> pPage;
            std::shared_ptr<material>   pNoTextureMat;
        };

        std::vector<page_item> lPageList_;
    };
}
}

#endif
