#ifndef LXGUI_GUI_SDL_RENDER_TARGET_HPP
#define LXGUI_GUI_SDL_RENDER_TARGET_HPP

#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_render_target.hpp"
#include "lxgui/impl/gui_sdl_material.hpp"
#include "lxgui/utils.hpp"

#include <memory>

struct SDL_Renderer;
struct SDL_Texture;

namespace lxgui::gui::sdl {

/// A place to render things (the screen, a texture, ...)
class render_target final : public gui::render_target {
public:
    /**
     * \brief Constructor.
     * \param rdr The SDL render to create the render_target for
     * \param dimensions The dimensions of the render_target
     * \param filt The filtering to apply to the target texture when displayed
     */
    render_target(
        SDL_Renderer*    rdr,
        const vector2ui& dimensions,
        material::filter filt = material::filter::none);

    /// Begins rendering on this target.
    void begin() override;

    /// Ends rendering on this target.
    void end() override;

    /**
     * \brief Clears the content of this render_target.
     * \param c The color to use as background
     */
    void clear(const color& c) override;

    /**
     * \brief Returns this render target's pixel rect.
     * \return This render target's pixel rect
     */
    bounds2f get_rect() const override;

    /**
     * \brief Sets this render target's dimensions.
     * \param dimensions The new dimensions (in pixels)
     * \return 'true' if the function had to re-create a
     * new render target
     */
    bool set_dimensions(const vector2ui& dimensions) override;

    /**
     * \brief Returns this render target's canvas dimension.
     * \return This render target's canvas dimension
     * \note This is the physical size of the render target.
     * On some systems, abitrary dimensions are not supported:
     * they can be promoted to the nearest power of two from
     * for example.
     */
    vector2ui get_canvas_dimensions() const override;

    /**
     * \brief Returns the associated texture for rendering.
     * \return The underlying pixel buffer, that you can use to render its content
     */
    std::weak_ptr<sdl::material> get_material();

    /**
     * \brief Returns the view matrix of this render target.
     * \return The view matrix of this render target
     */
    const matrix4f& get_view_matrix() const;

    /**
     * \brief Checks if the machine is capable of using render targets.
     * \param rdr The renderer to check for availability
     * \note If not, this function throws a gui::exception.
     */
    static void check_availability(SDL_Renderer* rdr);

    /**
     * \brief Returns the underlying SDL render texture object.
     * return The underlying SDL render texture object
     */
    SDL_Texture* get_render_texture();

private:
    std::shared_ptr<sdl::material> texture_;
    matrix4f                       view_matrix_;
};

} // namespace lxgui::gui::sdl

#endif
