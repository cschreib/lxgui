#ifndef LXGUI_GUI_SDL_RENDERTARGET_HPP
#define LXGUI_GUI_SDL_RENDERTARGET_HPP

#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/impl/gui_sdl_material.hpp"
#include "lxgui/utils.hpp"

#include <memory>

struct SDL_Renderer;
struct SDL_Texture;

namespace lxgui { namespace gui { namespace sdl {

/// A place to render things (the screen, a texture, ...)
class render_target final : public gui::render_target {
public:
    /// Constructor.
    /** \param pRenderer   The SDL render to create the render_target for
     *   \param mDimensions The dimensions of the render_target
     *   \param mFilter     The filtering to apply to the target texture when displayed
     */
    render_target(
        SDL_Renderer*    pRenderer,
        const vector2ui& mDimensions,
        material::filter mFilter = material::filter::NONE);

    /// Begins rendering on this target.
    void begin() override;

    /// Ends rendering on this target.
    void end() override;

    /// Clears the content of this render_target.
    /** \param mColor The color to use as background
     */
    void clear(const color& mColor) override;

    /// Returns this render target's pixel rect.
    /** \return This render target's pixel rect
     */
    bounds2f get_rect() const override;

    /// Sets this render target's dimensions.
    /** \param mDimensions The new dimensions (in pixels)
     *   \return 'true' if the function had to re-create a
     *           new render target
     */
    bool set_dimensions(const vector2ui& mDimensions) override;

    /// Returns this render target's canvas dimension.
    /** \return This render target's canvas dimension
     *   \note This is the physical size of the render target.
     *         On some systems, abitrary dimensions are not supported:
     *         they can be promoted to the nearest power of two from
     *         for example.
     */
    vector2ui get_canvas_dimensions() const override;

    /// Returns the associated texture for rendering.
    /** \return The underlying pixel buffer, that you can use to render its content
     */
    std::weak_ptr<sdl::material> get_material();

    /// Returns the view matrix of this render target.
    /** \return The view matrix of this render target
     */
    const matrix4f& get_view_matrix() const;

    /// Checks if the machine is capable of using render targets.
    /** \param pRenderer The renderer to check for availability
     *   \note If not, this function throws a gui::exception.
     */
    static void check_availability(SDL_Renderer* pRenderer);

    /// Returns the underlying SDL render texture object.
    /** return The underlying SDL render texture object
     */
    SDL_Texture* get_render_texture();

private:
    std::shared_ptr<sdl::material> pTexture_;
    matrix4f                       mViewMatrix_;
};

}}} // namespace lxgui::gui::sdl

#endif
