#include "lxgui/impl/gui_sdl_renderer.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/impl/gui_sdl_atlas.hpp"
#include "lxgui/impl/gui_sdl_font.hpp"
#include "lxgui/impl/gui_sdl_material.hpp"
#include "lxgui/impl/gui_sdl_rendertarget.hpp"
#include "lxgui/utils_string.hpp"

#include <SDL.h>
#include <SDL_image.h>

namespace lxgui::gui::sdl {

renderer::renderer(SDL_Renderer* rdr, bool initialise_sdl_image) : renderer_(rdr) {
    int window_width, window_height;
    SDL_GetRendererOutputSize(renderer_, &window_width, &window_height);
    window_dimensions_ = vector2ui(window_width, window_height);

    render_target::check_availability(rdr);

    if (initialise_sdl_image) {
        int img_flags = IMG_INIT_PNG;
        if ((IMG_Init(img_flags) & img_flags) == 0) {
            throw gui::exception(
                "gui::sdl::renderer",
                "Could not initialise SDL_image: " + std::string(IMG_GetError()) + ".");
        }
    }

    // Get maximum texture size
    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(rdr, &info) != 0) {
        throw gui::exception("gui::sdl::renderer", "Could not get renderer information.");
    }

    texture_max_size_ = std::min(info.max_texture_width, info.max_texture_height);
    if (texture_max_size_ == 0)
        texture_max_size_ = 1024u;

    // Check if we can do pre-multiplied alpha
    SDL_Texture* tex =
        SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 4u, 4u);

    pre_multiplied_alpha_supported_ =
        (SDL_SetTextureBlendMode(
             tex, (SDL_BlendMode)material::get_premultiplied_alpha_blend_mode()) == 0);

    SDL_DestroyTexture(tex);
}

std::string renderer::get_name() const {
    SDL_RendererInfo renderer_info;
    SDL_GetRendererInfo(renderer_, &renderer_info);
    return std::string("SDL (") + renderer_info.name + ")";
}

void renderer::begin_(std::shared_ptr<gui::render_target> target) {
    if (current_target_)
        throw gui::exception("gui::sdl::renderer", "Missing call to end()");

    if (target) {
        current_target_ = std::static_pointer_cast<sdl::render_target>(target);
        current_target_->begin();

        target_view_matrix_ = current_target_->get_view_matrix();
    } else {
        target_view_matrix_ = matrix4f::view(vector2f(window_dimensions_));
    }

    view_matrix_ = target_view_matrix_;
}

void renderer::end_() {
    if (current_target_)
        current_target_->end();

    current_target_ = nullptr;
}

void renderer::set_view_(const matrix4f& view_matrix) {
    raw_view_matrix_ = view_matrix;
    view_matrix_     = view_matrix * matrix4f::invert(target_view_matrix_);
}

matrix4f renderer::get_view() const {
    return raw_view_matrix_;
}

color premultiply_alpha(const color& c, bool pre_multiplied_alpha_supported) {
    if (pre_multiplied_alpha_supported)
        return color(c.r * c.a, c.g * c.a, c.b * c.a, c.a);
    else
        return c;
}

color interpolate_color(const color& color1, const color& color2, float interp) {
    return color(
        color1.r * (1.0f - interp) + color2.r * interp,
        color1.g * (1.0f - interp) + color2.g * interp,
        color1.b * (1.0f - interp) + color2.b * interp,
        color1.a * (1.0f - interp) + color2.a * interp);
}

ub32color to_ub32color(const color& color) {
    return {
        static_cast<unsigned char>(color.r * 255), static_cast<unsigned char>(color.g * 255),
        static_cast<unsigned char>(color.b * 255), static_cast<unsigned char>(color.a * 255)};
}

struct sdl_render_data {
    SDL_Rect         dest_quad         = {};
    SDL_Rect         dest_display_quad = {};
    SDL_Rect         src_quad          = {};
    SDL_Point        center            = {};
    int              angle             = 0;
    SDL_RendererFlip flip              = SDL_FLIP_NONE;
};

sdl_render_data
make_rects(const std::array<vertex, 4>& vertex_list, float tex_width, float tex_height) {
    sdl_render_data data;

    // First, re-order vertices as top-left, top-right, bottom-right, bottom-left
    std::array<std::size_t, 4> ids = {0u, 1u, 2u, 3u};
    if (vertex_list[0].pos.x < vertex_list[1].pos.x) {
        if (vertex_list[1].pos.y < vertex_list[2].pos.y) {
            // No rotation and no flip
        } else {
            // No rotation and flip Y
            ids = {3u, 2u, 1u, 0u};
        }
    } else if (vertex_list[0].pos.y < vertex_list[1].pos.y) {
        if (vertex_list[1].pos.x > vertex_list[2].pos.x) {
            // Rotated 90 degrees clockwise
            ids = {3u, 0u, 1u, 2u};
        } else {
            // Rotated 90 degrees clockwise and flip X
            ids = {0u, 3u, 2u, 1u};
        }
    } else if (vertex_list[0].pos.x > vertex_list[1].pos.x) {
        if (vertex_list[1].pos.y > vertex_list[2].pos.y) {
            // Rotated 180 degrees
            ids = {2u, 3u, 0u, 1u};
        } else {
            // No rotation and flip X
            ids = {1u, 0u, 3u, 2u};
        }
    } else if (vertex_list[0].pos.y > vertex_list[1].pos.y) {
        if (vertex_list[1].pos.x < vertex_list[2].pos.x) {
            // Rotated 90 degrees counter-clockwise
            ids = {1u, 2u, 3u, 0u};
        } else {
            // Rotated 90 degrees counter-clockwise and flip X
            ids = {2u, 1u, 0u, 3u};
        }
    }

    // Now, re-order UV coordinates as top-left, top-right, bottom-right, bottom-left
    // and figure out the required rotation and flipping to render as requested.
    int width = static_cast<int>(std::round(vertex_list[ids[2]].pos.x - vertex_list[ids[0]].pos.x));
    int height =
        static_cast<int>(std::round(vertex_list[ids[2]].pos.y - vertex_list[ids[0]].pos.y));
    int uv_index1 = 0;
    int uv_index2 = 2;

    data.dest_display_quad.x = static_cast<int>(std::round(vertex_list[ids[0]].pos.x));
    data.dest_display_quad.y = static_cast<int>(std::round(vertex_list[ids[0]].pos.y));
    data.dest_display_quad.w = width;
    data.dest_display_quad.h = height;

    data.dest_quad.x = static_cast<int>(std::round(vertex_list[ids[0]].pos.x));
    data.dest_quad.y = static_cast<int>(std::round(vertex_list[ids[0]].pos.y));

    bool axis_swapped = false;
    if (vertex_list[ids[0]].uvs.x < vertex_list[ids[1]].uvs.x) {
        if (vertex_list[ids[1]].uvs.y < vertex_list[ids[2]].uvs.y) {
            // No rotation and no flip
        } else {
            // No rotation and flip Y
            data.flip = SDL_FLIP_VERTICAL;
            uv_index1 = 3;
            uv_index2 = 1;
        }
    } else if (vertex_list[ids[0]].uvs.y < vertex_list[ids[1]].uvs.y) {
        axis_swapped = true;

        if (vertex_list[ids[1]].uvs.x > vertex_list[ids[2]].uvs.x) {
            // Rotated 90 degrees clockwise
            data.angle = -90;
            uv_index1  = 3;
            uv_index2  = 1;
            data.dest_quad.y += height;
        } else {
            // Rotated 90 degrees clockwise and flip X
            data.flip  = SDL_FLIP_HORIZONTAL;
            data.angle = -90;
            uv_index1  = 0;
            uv_index2  = 2;
            data.dest_quad.y += height;
        }
    } else if (vertex_list[ids[0]].uvs.x > vertex_list[ids[1]].uvs.x) {
        if (vertex_list[ids[1]].uvs.y > vertex_list[ids[2]].uvs.y) {
            // Rotated 180 degrees
            data.angle = 180;
            uv_index1  = 2;
            uv_index2  = 0;
            data.dest_quad.x += width;
            data.dest_quad.y += height;
        } else {
            // No rotation and flip X
            data.flip = SDL_FLIP_HORIZONTAL;
            uv_index1 = 1;
            uv_index2 = 3;
        }
    } else if (vertex_list[ids[0]].uvs.y > vertex_list[ids[1]].uvs.y) {
        axis_swapped = true;

        if (vertex_list[ids[1]].uvs.x < vertex_list[ids[2]].uvs.x) {
            // Rotated 90 degrees counter-clockwise
            data.angle = 90;
            uv_index1  = 1;
            uv_index2  = 3;
            data.dest_quad.x += width;
        } else {
            // Rotated 90 degrees counter-clockwise and flip X
            data.flip  = SDL_FLIP_HORIZONTAL;
            data.angle = 90;
            uv_index1  = 2;
            uv_index2  = 0;
            data.dest_quad.x += width;
        }
    }

    if (axis_swapped)
        std::swap(width, height);

    data.src_quad = SDL_Rect{
        (int)std::round(vertex_list[ids[uv_index1]].uvs.x * tex_width),
        (int)std::round(vertex_list[ids[uv_index1]].uvs.y * tex_height),
        (int)std::round(
            (vertex_list[ids[uv_index2]].uvs.x - vertex_list[ids[uv_index1]].uvs.x) * tex_width),
        (int)std::round(
            (vertex_list[ids[uv_index2]].uvs.y - vertex_list[ids[uv_index1]].uvs.y) * tex_height)};

    data.dest_quad.w = width;
    data.dest_quad.h = height;

    data.center = {0, 0};

    return data;
}

void renderer::render_quad_(const sdl::material* mat, const std::array<vertex, 4>& vertex_list) {
    auto view_list = vertex_list;
    for (auto& v : view_list) {
        v.pos = v.pos * view_matrix_;
    }

    if (mat) {
        SDL_Texture*    tex        = mat->get_texture();
        const vector2ui tex_dims   = mat->get_canvas_dimensions();
        const int       tex_width  = static_cast<int>(tex_dims.x);
        const int       tex_height = static_cast<int>(tex_dims.y);

        // Build the source and destination rect, figuring out rotation and flipping
        const sdl_render_data data = make_rects(view_list, tex_width, tex_height);

        if (data.dest_quad.w == 0 || data.dest_quad.h == 0)
            return;

        if (pre_multiplied_alpha_supported_) {
            if (SDL_SetTextureBlendMode(
                    tex, (SDL_BlendMode)material::get_premultiplied_alpha_blend_mode()) != 0) {
                throw gui::exception("gui::sdl::renderer", "Could not set texture blend mode.");
            }
        } else {
            if (SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND) != 0) {
                throw gui::exception("gui::sdl::renderer", "Could not set texture blend mode.");
            }
        }

        if (view_list[0].col == view_list[1].col && view_list[0].col == view_list[2].col &&
            view_list[0].col == view_list[3].col) {
            const auto color = view_list[0].col;

            if (pre_multiplied_alpha_supported_) {
                SDL_SetTextureColorMod(
                    tex, color.r * color.a * 255, color.g * color.a * 255, color.b * color.a * 255);
            } else {
                SDL_SetTextureColorMod(tex, color.r * 255, color.g * 255, color.b * 255);
            }

            SDL_SetTextureAlphaMod(tex, color.a * 255);

            bool coords_all_in_texture = data.src_quad.x >= 0 && data.src_quad.y >= 0 &&
                                         data.src_quad.x + data.src_quad.w <= tex_width &&
                                         data.src_quad.y + data.src_quad.h <= tex_height;

            bool one_pixel = (tex_height == 1 || data.dest_quad.h == 1) &&
                             (tex_width == 1 || data.dest_quad.w == 1);

            if (mat->get_wrap() == material::wrap::clamp || coords_all_in_texture || one_pixel) {
                // Single texture copy, or clamped wrap
                SDL_RenderCopyEx(
                    renderer_, tex, &data.src_quad, &data.dest_quad, data.angle, &data.center,
                    data.flip);
            } else {
                // Repeat wrap; SDL does not support this natively, so we have to
                // do the repeating ourselves.
                const bool  axis_swapped = std::abs(data.angle) == 90;
                const float x_factor     = float(data.dest_display_quad.w) /
                                       float(axis_swapped ? data.src_quad.h : data.src_quad.w);
                const float y_factor = float(data.dest_display_quad.h) /
                                       float(axis_swapped ? data.src_quad.w : data.src_quad.h);

                int sy = data.src_quad.y;
                while (sy < data.src_quad.y + data.src_quad.h) {
                    const int sy_clamped    = sy % tex_height + (sy < 0 ? tex_height : 0);
                    int       temp_s_height = tex_height - sy_clamped;
                    const int s_y1          = sy - data.src_quad.y;
                    if (s_y1 + temp_s_height > data.src_quad.h)
                        temp_s_height = data.src_quad.h - s_y1;
                    const int s_y2 = s_y1 + temp_s_height;

                    int sx = data.src_quad.x;
                    while (sx < data.src_quad.x + data.src_quad.w) {
                        const int sx_clamped   = sx % tex_width + (sx < 0 ? tex_width : 0);
                        int       temp_s_width = tex_width - sx_clamped;
                        const int s_x1         = sx - data.src_quad.x;
                        if (s_x1 + temp_s_width > data.src_quad.w)
                            temp_s_width = data.src_quad.w - s_x1;
                        const int s_x2 = s_x1 + temp_s_width;

                        int dx = data.dest_display_quad.x +
                                 static_cast<int>((axis_swapped ? s_y1 : s_x1) * x_factor);
                        int dy = data.dest_display_quad.y +
                                 static_cast<int>((axis_swapped ? s_x1 : s_y1) * y_factor);

                        int temp_d_width =
                            data.dest_display_quad.x +
                            static_cast<int>((axis_swapped ? s_y2 : s_x2) * x_factor) - dx;
                        int temp_d_height =
                            data.dest_display_quad.y +
                            static_cast<int>((axis_swapped ? s_x2 : s_y2) * y_factor) - dy;

                        if (data.angle == 90)
                            dx += temp_d_width;
                        else if (data.angle == -90)
                            dy += temp_d_height;
                        else if (data.angle == 180) {
                            dx += temp_d_width;
                            dy += temp_d_height;
                        }

                        if (axis_swapped)
                            std::swap(temp_d_width, temp_d_height);

                        const SDL_Rect src_quad{
                            sx_clamped, sy_clamped, temp_s_width, temp_s_height};
                        const SDL_Rect dest_quad{dx, dy, temp_d_width, temp_d_height};

                        SDL_RenderCopyEx(
                            renderer_, tex, &src_quad, &dest_quad, data.angle, &data.center,
                            data.flip);

                        sx += temp_s_width;
                    }

                    sy += temp_s_height;
                }
            }
        } else {
            throw gui::exception(
                "sdl::renderer", "Per-vertex color with texture is not supported.");
        }
    } else {
        // Note: SDL only supports axis-aligned rects for quad shapes
        const SDL_Rect dest_quad = {
            static_cast<int>(view_list[0].pos.x), static_cast<int>(view_list[0].pos.y),
            static_cast<int>(view_list[2].pos.x - view_list[0].pos.x),
            static_cast<int>(view_list[2].pos.y - view_list[0].pos.y)};

        if (dest_quad.w == 0 || dest_quad.h == 0)
            return;

        if (view_list[0].col == view_list[1].col && view_list[0].col == view_list[2].col &&
            view_list[0].col == view_list[3].col) {
            // Same color for all vertices
            const auto& color = view_list[0].col;
            if (pre_multiplied_alpha_supported_) {
                SDL_SetRenderDrawBlendMode(
                    renderer_, (SDL_BlendMode)material::get_premultiplied_alpha_blend_mode());
                SDL_SetRenderDrawColor(
                    renderer_, color.r * color.a * 255, color.g * color.a * 255,
                    color.b * color.a * 255, color.a * 255);
            } else {
                SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(
                    renderer_, color.r * 255, color.g * 255, color.b * 255, color.a * 255);
            }

            SDL_RenderFillRect(renderer_, &dest_quad);
        } else {
            // Different colors for each vertex; SDL does not support this natively.
            // We have to create a temporary texture, do the bilinear interpolation ourselves,
            // and draw that.
            const color color_quad[4] = {
                premultiply_alpha(view_list[0].col, pre_multiplied_alpha_supported_),
                premultiply_alpha(view_list[1].col, pre_multiplied_alpha_supported_),
                premultiply_alpha(view_list[2].col, pre_multiplied_alpha_supported_),
                premultiply_alpha(view_list[3].col, pre_multiplied_alpha_supported_)};

            sdl::material temp_mat(renderer_, vector2ui(dest_quad.w, dest_quad.h), false);
            ub32color*    pixel_data = temp_mat.lock_pointer();
            for (int y = 0; y < dest_quad.h; ++y)
                for (int x = 0; x < dest_quad.w; ++x) {
                    const color col_y1 =
                        interpolate_color(color_quad[0], color_quad[3], y / float(dest_quad.h - 1));
                    const color col_y2 =
                        interpolate_color(color_quad[1], color_quad[2], y / float(dest_quad.h - 1));
                    pixel_data[y * dest_quad.w + x] =
                        to_ub32color(interpolate_color(col_y1, col_y2, x / float(dest_quad.w - 1)));
                }
            temp_mat.unlock_pointer();

            const SDL_Rect src_quad = {0, 0, dest_quad.w, dest_quad.h};

            SDL_RenderCopy(renderer_, temp_mat.get_texture(), &src_quad, &dest_quad);
        }
    }
}

void renderer::render_quads_(
    const gui::material* mat, const std::vector<std::array<vertex, 4>>& quad_list) {

    const sdl::material* sdl_mat = static_cast<const sdl::material*>(mat);

    for (std::size_t k = 0; k < quad_list.size(); ++k) {
        render_quad_(sdl_mat, quad_list[k]);
    }
}

void renderer::render_cache_(const gui::material*, const gui::vertex_cache&, const matrix4f&) {
    throw gui::exception("gui::sdl::renderer", "SDL does not support vertex caches.");
}

SDL_Renderer* renderer::get_sdl_renderer() const {
    return renderer_;
}

std::shared_ptr<gui::material>
renderer::create_material_(const std::string& file_name, material::filter filt) {
    return std::make_shared<sdl::material>(
        renderer_, file_name, pre_multiplied_alpha_supported_, material::wrap::repeat, filt);
}

std::shared_ptr<gui::atlas> renderer::create_atlas_(material::filter filt) {
    return std::make_shared<sdl::atlas>(*this, filt);
}

std::size_t renderer::get_texture_max_size() const {
    return texture_max_size_;
}

bool renderer::is_texture_atlas_supported() const {
    return true;
}

bool renderer::is_texture_vertex_color_supported() const {
    return false;
}

bool renderer::is_vertex_cache_supported() const {
    return false;
}

std::shared_ptr<gui::material> renderer::create_material(
    const vector2ui& dimensions, const ub32color* pixel_data, material::filter filt) {
    std::shared_ptr<sdl::material> tex =
        std::make_shared<sdl::material>(renderer_, dimensions, false, material::wrap::repeat, filt);

    std::size_t pitch    = 0u;
    ub32color*  tex_data = tex->lock_pointer(&pitch);

    for (std::size_t y = 0u; y < dimensions.y; ++y) {
        const ub32color* pixel_data_row = pixel_data + y * dimensions.x;
        ub32color*       tex_data_row   = tex_data + y * pitch;
        std::copy(pixel_data_row, pixel_data_row + dimensions.x, tex_data_row);
    }

    tex->unlock_pointer();

    return std::move(tex);
}

std::shared_ptr<gui::material>
renderer::create_material(std::shared_ptr<gui::render_target> target, const bounds2f& location) {
    auto tex = std::static_pointer_cast<sdl::render_target>(target)->get_material().lock();
    if (location == target->get_rect()) {
        return std::move(tex);
    } else {
        return std::make_shared<sdl::material>(
            renderer_, tex->get_render_texture(), location, tex->get_filter());
    }
}

std::shared_ptr<gui::render_target>
renderer::create_render_target(const vector2ui& dimensions, material::filter filt) {
    return std::make_shared<sdl::render_target>(renderer_, dimensions, filt);
}

std::shared_ptr<gui::font> renderer::create_font_(
    const std::string&                   font_file,
    std::size_t                          size,
    std::size_t                          outline,
    const std::vector<code_point_range>& code_points,
    char32_t                             default_code_point) {
    return std::make_shared<sdl::font>(
        renderer_, font_file, size, outline, code_points, default_code_point,
        pre_multiplied_alpha_supported_);
}

std::shared_ptr<gui::vertex_cache> renderer::create_vertex_cache(gui::vertex_cache::type) {
    throw gui::exception("gui::sdl::renderer", "SDL does not support vertex caches.");
}

void renderer::notify_window_resized(const vector2ui& new_dimensions) {
    window_dimensions_ = new_dimensions;
}

} // namespace lxgui::gui::sdl
