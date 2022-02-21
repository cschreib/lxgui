#ifndef LXGUI_GUI_SDL_MATERIAL_HPP
#define LXGUI_GUI_SDL_MATERIAL_HPP

#include "lxgui/gui_color.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/utils.hpp"

#include <memory>
#include <variant>
#include <vector>

struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;

namespace lxgui::gui::sdl {

/**
 * \brief A class that holds rendering data
 * This implementation can contain either a plain color
 * or a real SDL_Texture. It is also used by the
 * gui::sdl::render_target class to store the output data.
 */
class material final : public gui::material {
public:
    /**
     * \brief Constructor for textures.
     * \param rdr The SDL render to create the material for
     * \param dimensions The requested texture dimensions
     * \param is_render_target Create the material for a render target or only for display
     * \param wrp How to adjust texture coordinates that are outside the [0,1] range
     * \param filt Use texture filtering or not (see set_filter())
     */
    material(
        SDL_Renderer*    rdr,
        const vector2ui& dimensions,
        bool             is_render_target = false,
        wrap             wrp              = wrap::repeat,
        filter           filt             = filter::none);

    /**
     * \brief Constructor for textures.
     * \param rdr The SDL render to create the material for
     * \param file_name The file from which the texture data is loaded
     * \param is_pre_multiplied_alpha_supported 'true' if the renderer supports pre-multipled alpha
     * \param wrp How to adjust texture coordinates that are outside the [0,1] range
     * \param filt Use texture filtering or not (see set_filter())
     */
    material(
        SDL_Renderer*      rdr,
        const std::string& file_name,
        bool               is_pre_multiplied_alpha_supported,
        wrap               wrp  = wrap::repeat,
        filter             filt = filter::none);

    /**
     * \brief Constructor for atlas textures.
     * \param rdr The SDL render to create the material for
     * \param tex The texture object of the atlas
     * \param rect The position of this texture inside the atlas
     * \param filt Use texture filtering or not (see set_filter())
     */
    material(SDL_Renderer* rdr, SDL_Texture* tex, const bounds2f& rect, filter filt = filter::none);

    material(const material& tex) = delete;
    material(material&& tex)      = delete;
    material& operator=(const material& tex) = delete;
    material& operator=(material&& tex) = delete;

    /// Destructor.
    ~material() noexcept override;

    /**
     * \brief Returns the pixel rect in pixels of the canvas containing this texture (if any).
     * \return The pixel rect in pixels of the canvas containing this texture (if any)
     */
    bounds2f get_rect() const override;

    /**
     * \brief Returns the physical dimensions (in pixels) of the canvas containing this texture (if any).
     * \return The physical dimensions (in pixels) of the canvas containing this (if any)
     * \note When a texture is loaded, most of the time it will fill the entire "canvas",
     *       namely, the 2D pixel array containing the texture data. However, some old
     *       hardware don't support textures that have non power-of-two dimensions.
     *       If the user creates a material for such a texture, the gui::renderer will
     *       create a bigger canvas that has power-of-two dimensions, and store the
     *       texture in it. Likewise, if a texture is placed in a wider texture atlas,
     *       the canvas will contain more than one texture.
     */
    vector2ui get_canvas_dimensions() const override;

    /**
     * \brief Checks if another material is based on the same texture as the current material.
     * \return 'true' if both materials use the same texture, 'false' otherwise
     */
    bool uses_same_texture(const gui::material& other) const override;

    /**
     * \brief Resizes this texture.
     * \param dimensions The new texture dimensions
     * \return 'true' if the function had to re-create a new texture object
     * \note All the previous data that was stored in this texture will be lost.
     */
    bool set_dimensions(const vector2ui& dimensions);

    /**
     * \brief Premultiplies an image by its alpha component.
     * \note Premultiplied alpha is a rendering technique that allows perfect
     *       alpha blending when using render targets.
     */
    static void premultiply_alpha(SDL_Surface* data);

    /**
     * \brief Returns the SDL blend mode corresponding to pre-multiplied alpha.
     * \returns the SDL blend mode corresponding to pre-multiplied alpha
     */
    static int get_premultiplied_alpha_blend_mode();

    /**
     * \brief Sets the wrap mode of this texture.
     * \param wrp How to adjust texture coordinates that are outside the [0,1] range
     */
    void set_wrap(wrap wrp);

    /**
     * \brief Sets the filter mode of this texture.
     * \param filt Use texture filtering or not
     * \note When texture filtering is disabled, enlarged textures get pixelated.
     *       Else, the GPU uses an averaging algorithm to blur the pixels.
     */
    void set_filter(filter filt);

    /**
     * \brief Returns the filter mode of this texture.
     * \return The filter mode of this texture
     */
    filter get_filter() const;

    /**
     * \brief Return the wrap mode of this texture.
     * \return The wrap mode of this texture
     */
    wrap get_wrap() const;

    /**
     * \brief Returns the underlying SDL texture object.
     * return The underlying SDL texture object
     */
    SDL_Texture* get_texture() const;

    /**
     * \brief Returns the underlying SDL texture object (for render target).
     * return The underlying SDL texture object (for render target)
     */
    SDL_Texture* get_render_texture();

    /**
     * \brief Returns the SDL renderer object that this material was created on.
     * return The SDL renderer object that this material was created on
     */
    SDL_Renderer* get_renderer();

    /**
     * \brief Returns a pointer to the texture data, which can be modified.
     * \param pitch An output pointer to the size (in bytes) of a row (ignored if nullptr)
     * \return A pointer to the texture data, which can be modified
     * \note The pointer is owned by this class, you must not delete it.
     *       Make sure you call unlock_pointer() when you are done.
     */
    ub32color* lock_pointer(std::size_t* pitch = nullptr);

    /// \copydoc lock_pointer
    const ub32color* lock_pointer(std::size_t* pitch = nullptr) const;

    /// Stops modifying the texture data and update the texture in GPU memory.
    void unlock_pointer() const;

private:
    SDL_Renderer* renderer_ = nullptr;

    vector2ui dimensions_;
    vector2ui canvas_dimensions_;
    bounds2f  rect_;
    wrap      wrap_             = wrap::repeat;
    filter    filter_           = filter::none;
    bool      is_render_target_ = false;

    SDL_Texture* texture_  = nullptr;
    bool         is_owner_ = false;
};

} // namespace lxgui::gui::sdl

#endif
