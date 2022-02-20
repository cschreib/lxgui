#ifndef LXGUI_INPUT_SDL_SOURCE_HPP
#define LXGUI_INPUT_SDL_SOURCE_HPP

#include "lxgui/gui_vector2.hpp"
#include "lxgui/input_source.hpp"
#include "lxgui/utils.hpp"

#include <chrono>
#include <memory>
#include <unordered_map>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Cursor;
union SDL_Event;

namespace lxgui::input { namespace sdl {

class source final : public input::source {
public:
    /// Initializes this input source.
    /** \param win The window from which to receive input
     *   \param rdr The SDL renderer, or null if using raw OpenGL
     *   \param initialise_sdl_image Set to 'true' if SDL Image has not been initialised yet
     */
    explicit source(SDL_Window* win, SDL_Renderer* rdr, bool initialise_sdl_image);

    source(const source&) = delete;
    source& operator=(const source&) = delete;

    utils::ustring get_clipboard_content() override;
    void           set_clipboard_content(const utils::ustring& content) override;

    void on_sdl_event(const SDL_Event& event);

    void set_mouse_cursor(const std::string& file_name, const gui::vector2i& hot_spot) override;
    void reset_mouse_cursor() override;

    float get_interface_scaling_factor_hint() const override;

private:
    gui::vector2ui get_window_pixel_size_() const;
    void           update_pixel_per_unit_();
    input::key     from_sdl_(int sdl_key) const;

    SDL_Window*   window_   = nullptr;
    SDL_Renderer* renderer_ = nullptr;

    float pixels_per_unit_ = 1.0f;

    using wrapped_cursor = std::unique_ptr<SDL_Cursor, void (*)(SDL_Cursor*)>;
    std::unordered_map<std::string, wrapped_cursor> cursor_map_;
};

}} // namespace lxgui::input::sdl

#endif
