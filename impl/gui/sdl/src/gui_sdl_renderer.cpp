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

renderer::renderer(SDL_Renderer* p_renderer, bool b_initialise_sdl_image) : p_renderer_(p_renderer) {
    int i_window_width, i_window_height;
    SDL_GetRendererOutputSize(p_renderer_, &i_window_width, &i_window_height);
    m_window_dimensions_ = vector2ui(i_window_width, i_window_height);

    render_target::check_availability(p_renderer);

    if (b_initialise_sdl_image) {
        int i_img_flags = IMG_INIT_PNG;
        if ((IMG_Init(i_img_flags) & i_img_flags) == 0) {
            throw gui::exception(
                "gui::sdl::renderer",
                "Could not initialise SDL_image: " + std::string(IMG_GetError()) + ".");
        }
    }

    // Get maximum texture size
    SDL_RendererInfo m_info;
    if (SDL_GetRendererInfo(p_renderer, &m_info) != 0) {
        throw gui::exception("gui::sdl::renderer", "Could not get renderer information.");
    }

    ui_texture_max_size_ = std::min(m_info.max_texture_width, m_info.max_texture_height);
    if (ui_texture_max_size_ == 0)
        ui_texture_max_size_ = 1024u;

    // Check if we can do pre-multiplied alpha
    SDL_Texture* p_texture = SDL_CreateTexture(
        p_renderer_, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 4u, 4u);

    b_pre_multiplied_alpha_supported_ =
        (SDL_SetTextureBlendMode(
             p_texture, (SDL_BlendMode)material::get_premultiplied_alpha_blend_mode()) == 0);

    SDL_DestroyTexture(p_texture);
}

std::string renderer::get_name() const {
    SDL_RendererInfo m_renderer_info;
    SDL_GetRendererInfo(p_renderer_, &m_renderer_info);
    return std::string("SDL (") + m_renderer_info.name + ")";
}

void renderer::begin_(std::shared_ptr<gui::render_target> p_target) {
    if (p_current_target_)
        throw gui::exception("gui::sdl::renderer", "Missing call to end()");

    if (p_target) {
        p_current_target_ = std::static_pointer_cast<sdl::render_target>(p_target);
        p_current_target_->begin();

        m_target_view_matrix_ = p_current_target_->get_view_matrix();
    } else {
        m_target_view_matrix_ = matrix4f::view(vector2f(m_window_dimensions_));
    }

    m_view_matrix_ = m_target_view_matrix_;
}

void renderer::end_() {
    if (p_current_target_)
        p_current_target_->end();

    p_current_target_ = nullptr;
}

void renderer::set_view_(const matrix4f& m_view_matrix) {
    m_raw_view_matrix_ = m_view_matrix;
    m_view_matrix_    = m_view_matrix * matrix4f::invert(m_target_view_matrix_);
}

matrix4f renderer::get_view() const {
    return m_raw_view_matrix_;
}

color premultiply_alpha(const color& m_color, bool b_pre_multiplied_alpha_supported) {
    if (b_pre_multiplied_alpha_supported)
        return color(m_color.r * m_color.a, m_color.g * m_color.a, m_color.b * m_color.a, m_color.a);
    else
        return m_color;
}

color interpolate_color(const color& m_color1, const color& m_color2, float f_interp) {
    return color(
        m_color1.r * (1.0f - f_interp) + m_color2.r * f_interp,
        m_color1.g * (1.0f - f_interp) + m_color2.g * f_interp,
        m_color1.b * (1.0f - f_interp) + m_color2.b * f_interp,
        m_color1.a * (1.0f - f_interp) + m_color2.a * f_interp);
}

ub32color to_ub32color(const color& m_color) {
    return {
        static_cast<unsigned char>(m_color.r * 255), static_cast<unsigned char>(m_color.g * 255),
        static_cast<unsigned char>(m_color.b * 255), static_cast<unsigned char>(m_color.a * 255)};
}

struct sdl_render_data {
    SDL_Rect         m_dest_quad        = {};
    SDL_Rect         m_dest_display_quad = {};
    SDL_Rect         m_src_quad         = {};
    SDL_Point        m_center          = {};
    int              i_angle           = 0;
    SDL_RendererFlip m_flip            = SDL_FLIP_NONE;
};

sdl_render_data
make_rects(const std::array<vertex, 4>& l_vertex_list, float f_tex_width, float f_tex_height) {
    sdl_render_data m_data;

    // First, re-order vertices as top-left, top-right, bottom-right, bottom-left
    std::array<std::size_t, 4> l_i_ds = {0u, 1u, 2u, 3u};
    if (l_vertex_list[0].pos.x < l_vertex_list[1].pos.x) {
        if (l_vertex_list[1].pos.y < l_vertex_list[2].pos.y) {
            // No rotation and no flip
        } else {
            // No rotation and flip Y
            l_i_ds = {3u, 2u, 1u, 0u};
        }
    } else if (l_vertex_list[0].pos.y < l_vertex_list[1].pos.y) {
        if (l_vertex_list[1].pos.x > l_vertex_list[2].pos.x) {
            // Rotated 90 degrees clockwise
            l_i_ds = {3u, 0u, 1u, 2u};
        } else {
            // Rotated 90 degrees clockwise and flip X
            l_i_ds = {0u, 3u, 2u, 1u};
        }
    } else if (l_vertex_list[0].pos.x > l_vertex_list[1].pos.x) {
        if (l_vertex_list[1].pos.y > l_vertex_list[2].pos.y) {
            // Rotated 180 degrees
            l_i_ds = {2u, 3u, 0u, 1u};
        } else {
            // No rotation and flip X
            l_i_ds = {1u, 0u, 3u, 2u};
        }
    } else if (l_vertex_list[0].pos.y > l_vertex_list[1].pos.y) {
        if (l_vertex_list[1].pos.x < l_vertex_list[2].pos.x) {
            // Rotated 90 degrees counter-clockwise
            l_i_ds = {1u, 2u, 3u, 0u};
        } else {
            // Rotated 90 degrees counter-clockwise and flip X
            l_i_ds = {2u, 1u, 0u, 3u};
        }
    }

    // Now, re-order UV coordinates as top-left, top-right, bottom-right, bottom-left
    // and figure out the required rotation and flipping to render as requested.
    int i_width =
        static_cast<int>(std::round(l_vertex_list[l_i_ds[2]].pos.x - l_vertex_list[l_i_ds[0]].pos.x));
    int i_height =
        static_cast<int>(std::round(l_vertex_list[l_i_ds[2]].pos.y - l_vertex_list[l_i_ds[0]].pos.y));
    int i_uv_index1 = 0;
    int i_uv_index2 = 2;

    m_data.m_dest_display_quad.x = static_cast<int>(std::round(l_vertex_list[l_i_ds[0]].pos.x));
    m_data.m_dest_display_quad.y = static_cast<int>(std::round(l_vertex_list[l_i_ds[0]].pos.y));
    m_data.m_dest_display_quad.w = i_width;
    m_data.m_dest_display_quad.h = i_height;

    m_data.m_dest_quad.x = static_cast<int>(std::round(l_vertex_list[l_i_ds[0]].pos.x));
    m_data.m_dest_quad.y = static_cast<int>(std::round(l_vertex_list[l_i_ds[0]].pos.y));

    bool b_axis_swapped = false;
    if (l_vertex_list[l_i_ds[0]].uvs.x < l_vertex_list[l_i_ds[1]].uvs.x) {
        if (l_vertex_list[l_i_ds[1]].uvs.y < l_vertex_list[l_i_ds[2]].uvs.y) {
            // No rotation and no flip
        } else {
            // No rotation and flip Y
            m_data.m_flip = SDL_FLIP_VERTICAL;
            i_uv_index1   = 3;
            i_uv_index2   = 1;
        }
    } else if (l_vertex_list[l_i_ds[0]].uvs.y < l_vertex_list[l_i_ds[1]].uvs.y) {
        b_axis_swapped = true;

        if (l_vertex_list[l_i_ds[1]].uvs.x > l_vertex_list[l_i_ds[2]].uvs.x) {
            // Rotated 90 degrees clockwise
            m_data.i_angle = -90;
            i_uv_index1    = 3;
            i_uv_index2    = 1;
            m_data.m_dest_quad.y += i_height;
        } else {
            // Rotated 90 degrees clockwise and flip X
            m_data.m_flip  = SDL_FLIP_HORIZONTAL;
            m_data.i_angle = -90;
            i_uv_index1    = 0;
            i_uv_index2    = 2;
            m_data.m_dest_quad.y += i_height;
        }
    } else if (l_vertex_list[l_i_ds[0]].uvs.x > l_vertex_list[l_i_ds[1]].uvs.x) {
        if (l_vertex_list[l_i_ds[1]].uvs.y > l_vertex_list[l_i_ds[2]].uvs.y) {
            // Rotated 180 degrees
            m_data.i_angle = 180;
            i_uv_index1    = 2;
            i_uv_index2    = 0;
            m_data.m_dest_quad.x += i_width;
            m_data.m_dest_quad.y += i_height;
        } else {
            // No rotation and flip X
            m_data.m_flip = SDL_FLIP_HORIZONTAL;
            i_uv_index1   = 1;
            i_uv_index2   = 3;
        }
    } else if (l_vertex_list[l_i_ds[0]].uvs.y > l_vertex_list[l_i_ds[1]].uvs.y) {
        b_axis_swapped = true;

        if (l_vertex_list[l_i_ds[1]].uvs.x < l_vertex_list[l_i_ds[2]].uvs.x) {
            // Rotated 90 degrees counter-clockwise
            m_data.i_angle = 90;
            i_uv_index1    = 1;
            i_uv_index2    = 3;
            m_data.m_dest_quad.x += i_width;
        } else {
            // Rotated 90 degrees counter-clockwise and flip X
            m_data.m_flip  = SDL_FLIP_HORIZONTAL;
            m_data.i_angle = 90;
            i_uv_index1    = 2;
            i_uv_index2    = 0;
            m_data.m_dest_quad.x += i_width;
        }
    }

    if (b_axis_swapped)
        std::swap(i_width, i_height);

    m_data.m_src_quad = SDL_Rect{
        (int)std::round(l_vertex_list[l_i_ds[i_uv_index1]].uvs.x * f_tex_width),
        (int)std::round(l_vertex_list[l_i_ds[i_uv_index1]].uvs.y * f_tex_height),
        (int)std::round(
            (l_vertex_list[l_i_ds[i_uv_index2]].uvs.x - l_vertex_list[l_i_ds[i_uv_index1]].uvs.x) * f_tex_width),
        (int)std::round(
            (l_vertex_list[l_i_ds[i_uv_index2]].uvs.y - l_vertex_list[l_i_ds[i_uv_index1]].uvs.y) *
            f_tex_height)};

    m_data.m_dest_quad.w = i_width;
    m_data.m_dest_quad.h = i_height;

    m_data.m_center = {0, 0};

    return m_data;
}

void renderer::render_quad_(const sdl::material* p_mat, const std::array<vertex, 4>& l_vertex_list) {
    auto l_view_list = l_vertex_list;
    for (auto& v : l_view_list) {
        v.pos = v.pos * m_view_matrix_;
    }

    if (p_mat) {
        SDL_Texture*    p_texture   = p_mat->get_texture();
        const vector2ui m_tex_dims   = p_mat->get_canvas_dimensions();
        const int       i_tex_width  = static_cast<int>(m_tex_dims.x);
        const int       i_tex_height = static_cast<int>(m_tex_dims.y);

        // Build the source and destination rect, figuring out rotation and flipping
        const sdl_render_data m_data = make_rects(l_view_list, i_tex_width, i_tex_height);

        if (m_data.m_dest_quad.w == 0 || m_data.m_dest_quad.h == 0)
            return;

        if (b_pre_multiplied_alpha_supported_) {
            if (SDL_SetTextureBlendMode(
                    p_texture, (SDL_BlendMode)material::get_premultiplied_alpha_blend_mode()) != 0) {
                throw gui::exception("gui::sdl::renderer", "Could not set texture blend mode.");
            }
        } else {
            if (SDL_SetTextureBlendMode(p_texture, SDL_BLENDMODE_BLEND) != 0) {
                throw gui::exception("gui::sdl::renderer", "Could not set texture blend mode.");
            }
        }

        if (l_view_list[0].col == l_view_list[1].col && l_view_list[0].col == l_view_list[2].col &&
            l_view_list[0].col == l_view_list[3].col) {
            const auto m_color = l_view_list[0].col;

            if (b_pre_multiplied_alpha_supported_) {
                SDL_SetTextureColorMod(
                    p_texture, m_color.r * m_color.a * 255, m_color.g * m_color.a * 255,
                    m_color.b * m_color.a * 255);
            } else {
                SDL_SetTextureColorMod(p_texture, m_color.r * 255, m_color.g * 255, m_color.b * 255);
            }

            SDL_SetTextureAlphaMod(p_texture, m_color.a * 255);

            bool b_coords_all_in_texture = m_data.m_src_quad.x >= 0 && m_data.m_src_quad.y >= 0 &&
                                       m_data.m_src_quad.x + m_data.m_src_quad.w <= i_tex_width &&
                                       m_data.m_src_quad.y + m_data.m_src_quad.h <= i_tex_height;

            bool b_one_pixel = (i_tex_height == 1 || m_data.m_dest_quad.h == 1) &&
                             (i_tex_width == 1 || m_data.m_dest_quad.w == 1);

            if (p_mat->get_wrap() == material::wrap::clamp || b_coords_all_in_texture || b_one_pixel) {
                // Single texture copy, or clamped wrap
                SDL_RenderCopyEx(
                    p_renderer_, p_texture, &m_data.m_src_quad, &m_data.m_dest_quad, m_data.i_angle,
                    &m_data.m_center, m_data.m_flip);
            } else {
                // Repeat wrap; SDL does not support this natively, so we have to
                // do the repeating ourselves.
                const bool  b_axis_swapped = std::abs(m_data.i_angle) == 90;
                const float f_x_factor     = float(m_data.m_dest_display_quad.w) /
                                       float(b_axis_swapped ? m_data.m_src_quad.h : m_data.m_src_quad.w);
                const float f_y_factor = float(m_data.m_dest_display_quad.h) /
                                       float(b_axis_swapped ? m_data.m_src_quad.w : m_data.m_src_quad.h);

                int i_sy = m_data.m_src_quad.y;
                while (i_sy < m_data.m_src_quad.y + m_data.m_src_quad.h) {
                    const int i_sy_clamped   = i_sy % i_tex_height + (i_sy < 0 ? i_tex_height : 0);
                    int       i_temp_s_height = i_tex_height - i_sy_clamped;
                    const int i_s_y1         = i_sy - m_data.m_src_quad.y;
                    if (i_s_y1 + i_temp_s_height > m_data.m_src_quad.h)
                        i_temp_s_height = m_data.m_src_quad.h - i_s_y1;
                    const int i_s_y2 = i_s_y1 + i_temp_s_height;

                    int i_sx = m_data.m_src_quad.x;
                    while (i_sx < m_data.m_src_quad.x + m_data.m_src_quad.w) {
                        const int i_sx_clamped  = i_sx % i_tex_width + (i_sx < 0 ? i_tex_width : 0);
                        int       i_temp_s_width = i_tex_width - i_sx_clamped;
                        const int i_s_x1        = i_sx - m_data.m_src_quad.x;
                        if (i_s_x1 + i_temp_s_width > m_data.m_src_quad.w)
                            i_temp_s_width = m_data.m_src_quad.w - i_s_x1;
                        const int i_s_x2 = i_s_x1 + i_temp_s_width;

                        int i_dx = m_data.m_dest_display_quad.x +
                                  static_cast<int>((b_axis_swapped ? i_s_y1 : i_s_x1) * f_x_factor);
                        int i_dy = m_data.m_dest_display_quad.y +
                                  static_cast<int>((b_axis_swapped ? i_s_x1 : i_s_y1) * f_y_factor);

                        int i_temp_d_width =
                            m_data.m_dest_display_quad.x +
                            static_cast<int>((b_axis_swapped ? i_s_y2 : i_s_x2) * f_x_factor) - i_dx;
                        int i_temp_d_height =
                            m_data.m_dest_display_quad.y +
                            static_cast<int>((b_axis_swapped ? i_s_x2 : i_s_y2) * f_y_factor) - i_dy;

                        if (m_data.i_angle == 90)
                            i_dx += i_temp_d_width;
                        else if (m_data.i_angle == -90)
                            i_dy += i_temp_d_height;
                        else if (m_data.i_angle == 180) {
                            i_dx += i_temp_d_width;
                            i_dy += i_temp_d_height;
                        }

                        if (b_axis_swapped)
                            std::swap(i_temp_d_width, i_temp_d_height);

                        const SDL_Rect m_src_quad{i_sx_clamped, i_sy_clamped, i_temp_s_width, i_temp_s_height};
                        const SDL_Rect m_dest_quad{i_dx, i_dy, i_temp_d_width, i_temp_d_height};

                        SDL_RenderCopyEx(
                            p_renderer_, p_texture, &m_src_quad, &m_dest_quad, m_data.i_angle,
                            &m_data.m_center, m_data.m_flip);

                        i_sx += i_temp_s_width;
                    }

                    i_sy += i_temp_s_height;
                }
            }
        } else {
            throw gui::exception(
                "sdl::renderer", "Per-vertex color with texture is not supported.");
        }
    } else {
        // Note: SDL only supports axis-aligned rects for quad shapes
        const SDL_Rect m_dest_quad = {
            static_cast<int>(l_view_list[0].pos.x), static_cast<int>(l_view_list[0].pos.y),
            static_cast<int>(l_view_list[2].pos.x - l_view_list[0].pos.x),
            static_cast<int>(l_view_list[2].pos.y - l_view_list[0].pos.y)};

        if (m_dest_quad.w == 0 || m_dest_quad.h == 0)
            return;

        if (l_view_list[0].col == l_view_list[1].col && l_view_list[0].col == l_view_list[2].col &&
            l_view_list[0].col == l_view_list[3].col) {
            // Same color for all vertices
            const auto& m_color = l_view_list[0].col;
            if (b_pre_multiplied_alpha_supported_) {
                SDL_SetRenderDrawBlendMode(
                    p_renderer_, (SDL_BlendMode)material::get_premultiplied_alpha_blend_mode());
                SDL_SetRenderDrawColor(
                    p_renderer_, m_color.r * m_color.a * 255, m_color.g * m_color.a * 255,
                    m_color.b * m_color.a * 255, m_color.a * 255);
            } else {
                SDL_SetRenderDrawBlendMode(p_renderer_, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(
                    p_renderer_, m_color.r * 255, m_color.g * 255, m_color.b * 255, m_color.a * 255);
            }

            SDL_RenderFillRect(p_renderer_, &m_dest_quad);
        } else {
            // Different colors for each vertex; SDL does not support this natively.
            // We have to create a temporary texture, do the bilinear interpolation ourselves,
            // and draw that.
            const color l_color_quad[4] = {
                premultiply_alpha(l_view_list[0].col, b_pre_multiplied_alpha_supported_),
                premultiply_alpha(l_view_list[1].col, b_pre_multiplied_alpha_supported_),
                premultiply_alpha(l_view_list[2].col, b_pre_multiplied_alpha_supported_),
                premultiply_alpha(l_view_list[3].col, b_pre_multiplied_alpha_supported_)};

            sdl::material m_temp_mat(p_renderer_, vector2ui(m_dest_quad.w, m_dest_quad.h), false);
            ub32color*    p_pixel_data = m_temp_mat.lock_pointer();
            for (int y = 0; y < m_dest_quad.h; ++y)
                for (int x = 0; x < m_dest_quad.w; ++x) {
                    const color l_col_y1 =
                        interpolate_color(l_color_quad[0], l_color_quad[3], y / float(m_dest_quad.h - 1));
                    const color l_col_y2 =
                        interpolate_color(l_color_quad[1], l_color_quad[2], y / float(m_dest_quad.h - 1));
                    p_pixel_data[y * m_dest_quad.w + x] =
                        to_ub32color(interpolate_color(l_col_y1, l_col_y2, x / float(m_dest_quad.w - 1)));
                }
            m_temp_mat.unlock_pointer();

            const SDL_Rect m_src_quad = {0, 0, m_dest_quad.w, m_dest_quad.h};

            SDL_RenderCopy(p_renderer_, m_temp_mat.get_texture(), &m_src_quad, &m_dest_quad);
        }
    }
}

void renderer::render_quads_(
    const gui::material* p_material, const std::vector<std::array<vertex, 4>>& l_quad_list) {
    const sdl::material* p_mat = static_cast<const sdl::material*>(p_material);

    for (std::size_t k = 0; k < l_quad_list.size(); ++k) {
        render_quad_(p_mat, l_quad_list[k]);
    }
}

void renderer::render_cache_(const gui::material*, const gui::vertex_cache&, const matrix4f&) {
    throw gui::exception("gui::sdl::renderer", "SDL does not support vertex caches.");
}

SDL_Renderer* renderer::get_sdl_renderer() const {
    return p_renderer_;
}

std::shared_ptr<gui::material>
renderer::create_material_(const std::string& s_file_name, material::filter m_filter) {
    return std::make_shared<sdl::material>(
        p_renderer_, s_file_name, b_pre_multiplied_alpha_supported_, material::wrap::repeat, m_filter);
}

std::shared_ptr<gui::atlas> renderer::create_atlas_(material::filter m_filter) {
    return std::make_shared<sdl::atlas>(*this, m_filter);
}

std::size_t renderer::get_texture_max_size() const {
    return ui_texture_max_size_;
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
    const vector2ui& m_dimensions, const ub32color* p_pixel_data, material::filter m_filter) {
    std::shared_ptr<sdl::material> p_tex = std::make_shared<sdl::material>(
        p_renderer_, m_dimensions, false, material::wrap::repeat, m_filter);

    std::size_t ui_pitch  = 0u;
    ub32color*  p_tex_data = p_tex->lock_pointer(&ui_pitch);

    for (std::size_t ui_y = 0u; ui_y < m_dimensions.y; ++ui_y) {
        const ub32color* p_pixel_data_row = p_pixel_data + ui_y * m_dimensions.x;
        ub32color*       p_tex_data_row   = p_tex_data + ui_y * ui_pitch;
        std::copy(p_pixel_data_row, p_pixel_data_row + m_dimensions.x, p_tex_data_row);
    }

    p_tex->unlock_pointer();

    return std::move(p_tex);
}

std::shared_ptr<gui::material> renderer::create_material(
    std::shared_ptr<gui::render_target> p_render_target, const bounds2f& m_location) {
    auto p_tex = std::static_pointer_cast<sdl::render_target>(p_render_target)->get_material().lock();
    if (m_location == p_render_target->get_rect()) {
        return std::move(p_tex);
    } else {
        return std::make_shared<sdl::material>(
            p_renderer_, p_tex->get_render_texture(), m_location, p_tex->get_filter());
    }
}

std::shared_ptr<gui::render_target>
renderer::create_render_target(const vector2ui& m_dimensions, material::filter m_filter) {
    return std::make_shared<sdl::render_target>(p_renderer_, m_dimensions, m_filter);
}

std::shared_ptr<gui::font> renderer::create_font_(
    const std::string&                   s_font_file,
    std::size_t                          ui_size,
    std::size_t                          ui_outline,
    const std::vector<code_point_range>& l_code_points,
    char32_t                             ui_default_code_point) {
    return std::make_shared<sdl::font>(
        p_renderer_, s_font_file, ui_size, ui_outline, l_code_points, ui_default_code_point,
        b_pre_multiplied_alpha_supported_);
}

std::shared_ptr<gui::vertex_cache> renderer::create_vertex_cache(gui::vertex_cache::type) {
    throw gui::exception("gui::sdl::renderer", "SDL does not support vertex caches.");
}

void renderer::notify_window_resized(const vector2ui& m_new_dimensions) {
    m_window_dimensions_ = m_new_dimensions;
}

} // namespace lxgui::gui::sdl
