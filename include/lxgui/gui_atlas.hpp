#ifndef LXGUI_GUI_ATLAS_HPP
#define LXGUI_GUI_ATLAS_HPP

#include "lxgui/gui_bounds2.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace lxgui::gui {

class renderer;
class font;

/// A single texture holding multiple materials for efficient rendering
/** This is an abstract class that must be implemented
 * and created by the corresponding gui::renderer.
 */
class atlas_page {
public:
    /// Constructor.
    /** \param filt Use texture filtering or not (see material::set_filter())
     */
    explicit atlas_page(material::filter filt);

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
    /** \param file_name The name of the file
     * \return The material (nullptr if not found)
     */
    std::shared_ptr<material> fetch_material(const std::string& file_name) const;

    /// Creates a new material from a texture file.
    /** \param file_name The name of the file
     * \param mat The material to add to this page
     * \return The new material (or nullptr if the material could not fit)
     */
    std::shared_ptr<material> add_material(const std::string& file_name, const material& mat);

    /// Find a font in this page (nullptr if not found).
    /** \param font_name The name+size of the font
     * \return The font (nullptr if not found)
     */
    std::shared_ptr<font> fetch_font(const std::string& font_name) const;

    /// Creates a new font from a texture file.
    /** \param font_name The name of the file
     * \param fnt The font to add to this page
     * \return The new font (or nullptr if the font could not fit)
     */
    bool add_font(const std::string& font_name, std::shared_ptr<gui::font> fnt);

    /// Checks if this page is empty (contains no materials).
    /** \return 'true' if the page is empty, 'false' otherwise
     */
    bool empty() const;

protected:
    /// Adds a new material to this page, at the provided location
    /** \param mat The material to add
     * \param location The position at which to insert this material
     * \return A new material pointing to inside this page
     */
    virtual std::shared_ptr<material>
    add_material_(const material& mat, const bounds2f& location) = 0;

    /// Return the width of this page (in pixels).
    /** \return The width of this page (in pixels)
     */
    virtual float get_width_() const = 0;

    /// Return the height of this page (in pixels).
    /** \return The height of this page (in pixels)
     */
    virtual float get_height_() const = 0;

    material::filter filter_ = material::filter::none;

private:
    /// Try to insert a new texture into this page, and return the best position if any
    /** \param width The width of the texture to insert
     * \param height The height of the texture to insert
     * \return The new position for this texture, or std::nullopt if it does not fit
     */
    std::optional<bounds2f> find_location_(float width, float height) const;

    std::unordered_map<std::string, std::weak_ptr<gui::material>> texture_list_;
    std::unordered_map<std::string, std::weak_ptr<gui::font>>     font_list_;
};

/// A class that holds multiple materials for efficient rendering
/** This is an abstract class that must be implemented
 * and created by the corresponding gui::renderer.
 */
class atlas {
public:
    /// Constructor.
    /** \param rdr The renderer with witch to create this atlas
     * \param filt Use texture filtering or not (see material::set_filter())
     */
    explicit atlas(renderer& rdr, material::filter filt);

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
    /** \param file_name The name of the file
     * \return The material (nullptr if not found)
     */
    std::shared_ptr<material> fetch_material(const std::string& file_name) const;

    /// Add a new material to the atlas.
    /** \param file_name The name of the file
     * \param mat The material to add to this atlas
     * \return The new material
     */
    std::shared_ptr<material> add_material(const std::string& file_name, const material& mat);

    /// Find a font in this atlas (nullptr if not found).
    /** \param font_name The name of the font+size
     * \return The font (nullptr if not found)
     */
    std::shared_ptr<font> fetch_font(const std::string& font_name) const;

    /// Add a new font to the atlas.
    /** \param font_name The name of the font+size
     * \param fnt The font to add to this atlas
     * \return 'true' if the font was added to this atlas, 'false' otherwise
     */
    bool add_font(const std::string& font_name, std::shared_ptr<gui::font> fnt);

    /// Return the number of pages in this atlas.
    /** \return The number of pages in this atlas
     */
    std::size_t get_num_pages() const;

protected:
    /// Create a new page in this atlas.
    /** \return The new page, added at the back of the page list
     */
    virtual std::unique_ptr<atlas_page> create_page_() = 0;

    renderer&        renderer_;
    material::filter filter_ = material::filter::none;

private:
    /// Create a new page in this atlas.
    /** \return The new page, added at the back of the page list
     */
    void add_page_();

    struct page_item {
        std::unique_ptr<atlas_page> page;
        std::shared_ptr<material>   no_texture_mat;
    };

    std::vector<page_item> page_list_;
};

} // namespace lxgui::gui

#endif
