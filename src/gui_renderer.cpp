#include "lxgui/gui_renderer.hpp"

#include "lxgui/gui_atlas.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui::gui {

void renderer::begin(std::shared_ptr<render_target> p_target) {
    ui_batch_count_ = 0;

    if (is_quad_batching_enabled()) {
        p_current_material_ = nullptr;

        if (is_vertex_cache_enabled()) {
            try {
                if (quad_cache_[0].p_cache == nullptr) {
                    for (std::size_t ui_index = 0u; ui_index < batching_cache_cycle_size;
                         ++ui_index) {
                        quad_cache_[ui_index].p_cache =
                            create_vertex_cache(vertex_cache::type::quads);
                    }
                }
            } catch (const std::exception& e) {
                gui::out << gui::warning << e.what() << std::endl;
                gui::out << gui::warning
                         << "gui::renderer : Failed to create caches for quad batching. "
                            "Vertex caches will be disabled."
                         << std::endl;

                vertex_cache_enabled_ = false;
            }
        }
    }

    begin_(std::move(p_target));
}

void renderer::end() {
    if (is_quad_batching_enabled()) {
        flush_quad_batch();
        ui_last_frame_batch_count_ = ui_batch_count_;
    }

    end_();
}

void renderer::set_view(const matrix4f& view_matrix) {
    if (is_quad_batching_enabled()) {
        flush_quad_batch();
    }

    set_view_(view_matrix);
}

void renderer::render_quad(const quad& q) {
    render_quads(q.mat.get(), {q.v});
}

bool renderer::uses_same_texture_(const material* p_mat1, const material* p_mat2) const {
    if (p_mat1 == p_mat2)
        return true;

    if (p_mat1 && p_mat2 && p_mat1->uses_same_texture(*p_mat2))
        return true;

    if (is_texture_vertex_color_supported()) {
        if (p_mat1 && p_mat1->is_in_atlas() && !p_mat2)
            return true;
        if (p_mat2 && p_mat2->is_in_atlas() && !p_mat1)
            return true;
    }

    return false;
}

void renderer::render_quads(
    const material* p_material, const std::vector<std::array<vertex, 4>>& quad_list) {
    if (quad_list.empty())
        return;

    if (!is_quad_batching_enabled()) {
        // Render immediately
        render_quads_(p_material, quad_list);
        return;
    }

    if (!uses_same_texture_(p_material, p_current_material_)) {
        // Render current batch and start a new one
        flush_quad_batch();
        p_current_material_ = p_material;
    }

    if (quad_cache_[ui_current_quad_cache_].data.empty()) {
        // Start a new batch
        p_current_material_ = p_material;
    }

    if (!p_current_material_) {
        // Previous quads had no material, override with the new one
        p_current_material_ = p_material;
    }

    // Add to the cache
    auto& cache = quad_cache_[ui_current_quad_cache_];

    if (!p_material && is_texture_atlas_enabled() && is_texture_vertex_color_supported()) {
        // To allow quads with no texture to enter the batch
        // with atlas textures, we just change their UV coordinates
        // to map to the first top-left pixel of the atlas, which is always white.
        cache.data.reserve(cache.data.size() + quad_list.size());
        for (const auto& orig_quad : quad_list) {
            cache.data.push_back(orig_quad);
            auto& quad  = cache.data.back();
            quad[0].uvs = quad[1].uvs = quad[2].uvs = quad[3].uvs = vector2f(0.0f, 0.0f);
        }
    } else {
        cache.data.insert(cache.data.end(), quad_list.begin(), quad_list.end());
    }
}

void renderer::flush_quad_batch() {
    auto& cache = quad_cache_[ui_current_quad_cache_];
    if (cache.data.empty())
        return;

    if (cache.p_cache) {
        cache.p_cache->update(cache.data[0].data(), cache.data.size() * 4);
        render_cache_(p_current_material_, *cache.p_cache, matrix4f::identity);
    } else {
        render_quads_(p_current_material_, cache.data);
    }

    cache.data.clear();
    p_current_material_ = nullptr;

    ++ui_current_quad_cache_;
    if (ui_current_quad_cache_ == batching_cache_cycle_size)
        ui_current_quad_cache_ = 0u;

    ++ui_batch_count_;
}

void renderer::render_cache(
    const material* p_material, const vertex_cache& cache, const matrix4f& model_transform) {
    if (is_quad_batching_enabled()) {
        flush_quad_batch();
    }

    render_cache_(p_material, cache, model_transform);
}

bool renderer::is_quad_batching_enabled() const {
    return quad_batching_enabled_;
}

void renderer::set_quad_batching_enabled(bool enabled) {
    quad_batching_enabled_ = enabled;
}

std::shared_ptr<gui::material>
renderer::create_material(const std::string& file_name, material::filter filt) {
    std::string backed_name = utils::to_string(static_cast<std::size_t>(filt)) + '|' + file_name;
    auto        iter        = texture_list_.find(backed_name);
    if (iter != texture_list_.end()) {
        if (std::shared_ptr<gui::material> p_lock = iter->second.lock())
            return p_lock;
        else
            texture_list_.erase(iter);
    }

    try {
        std::shared_ptr<gui::material> p_tex = create_material_(file_name, filt);
        texture_list_[file_name]             = p_tex;
        return p_tex;
    } catch (const std::exception& e) {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

namespace {
std::string hash_font_parameters(
    const std::string&                   font_file,
    std::size_t                          ui_size,
    std::size_t                          ui_outline,
    const std::vector<code_point_range>& code_points,
    char32_t                             ui_default_code_point) {
    std::string font_name = font_file + "|s" + utils::to_string(ui_size);
    if (ui_outline > 0u)
        font_name += "|o" + utils::to_string(ui_outline);

    for (const code_point_range& range : code_points)
        font_name +=
            "|c" + utils::to_string(range.ui_first) + "-" + utils::to_string(range.ui_last);

    font_name += "|d" + utils::to_string(ui_default_code_point);

    return font_name;
}
} // namespace

std::shared_ptr<gui::font> renderer::create_font(
    const std::string&                   font_file,
    std::size_t                          ui_size,
    std::size_t                          ui_outline,
    const std::vector<code_point_range>& code_points,
    char32_t                             ui_default_code_point) {
    const std::string font_name =
        hash_font_parameters(font_file, ui_size, ui_outline, code_points, ui_default_code_point);

    auto iter = font_list_.find(font_name);
    if (iter != font_list_.end()) {
        if (std::shared_ptr<gui::font> p_lock = iter->second.lock())
            return p_lock;
        else
            font_list_.erase(iter);
    }

    std::shared_ptr<gui::font> p_font =
        create_font_(font_file, ui_size, ui_outline, code_points, ui_default_code_point);

    font_list_[font_name] = p_font;
    return p_font;
}

bool renderer::is_texture_atlas_enabled() const {
    return texture_atlas_enabled_ && is_texture_atlas_supported();
}

void renderer::set_texture_atlas_enabled(bool enabled) {
    texture_atlas_enabled_ = enabled;
}

std::size_t renderer::get_texture_atlas_page_size() const {
    if (ui_texture_atlas_page_size_ == 0u)
        return std::min<std::size_t>(4096u, get_texture_max_size());
    else
        return ui_texture_atlas_page_size_;
}

void renderer::set_texture_atlas_page_size(std::size_t ui_page_size) {
    ui_texture_atlas_page_size_ = ui_page_size;
}

std::size_t renderer::get_num_texture_atlas_pages() const {
    std::size_t ui_count = 0;

    for (const auto& page : atlas_list_) {
        ui_count += page.second->get_num_pages();
    }

    return ui_count;
}

bool renderer::is_vertex_cache_enabled() const {
    return vertex_cache_enabled_ && is_vertex_cache_supported();
}

void renderer::set_vertex_cache_enabled(bool enabled) {
    vertex_cache_enabled_ = enabled;
}

void renderer::auto_detect_settings() {
    vertex_cache_enabled_  = true;
    texture_atlas_enabled_ = true;
    quad_batching_enabled_ = true;
}

atlas& renderer::get_atlas_(const std::string& atlas_category, material::filter filt) {
    std::shared_ptr<gui::atlas> p_atlas;

    std::string baked_atlas_name =
        utils::to_string(static_cast<std::size_t>(filt)) + '|' + atlas_category;
    auto iter = atlas_list_.find(baked_atlas_name);
    if (iter != atlas_list_.end()) {
        p_atlas = iter->second;
    }

    if (!p_atlas) {
        p_atlas                       = create_atlas_(filt);
        atlas_list_[baked_atlas_name] = p_atlas;
    }

    return *p_atlas;
}

std::shared_ptr<material> renderer::create_atlas_material(
    const std::string& atlas_category, const std::string& file_name, material::filter filt) {
    if (!is_texture_atlas_enabled())
        return create_material(file_name, filt);

    auto& atlas = get_atlas_(atlas_category, filt);

    auto p_tex = atlas.fetch_material(file_name);
    if (p_tex)
        return p_tex;

    p_tex = create_material(file_name, filt);
    if (!p_tex)
        return nullptr;

    auto p_added_tex = atlas.add_material(file_name, *p_tex);
    if (p_added_tex)
        return p_added_tex;
    else
        return p_tex;
}

std::shared_ptr<font> renderer::create_atlas_font(
    const std::string&                   atlas_category,
    const std::string&                   font_file,
    std::size_t                          ui_size,
    std::size_t                          ui_outline,
    const std::vector<code_point_range>& code_points,
    char32_t                             ui_default_code_point) {
    if (!is_texture_atlas_enabled())
        return create_font(font_file, ui_size, ui_outline, code_points, ui_default_code_point);

    auto& atlas = get_atlas_(atlas_category, material::filter::none);

    const std::string font_name =
        hash_font_parameters(font_file, ui_size, ui_outline, code_points, ui_default_code_point);

    auto p_font = atlas.fetch_font(font_name);
    if (p_font)
        return p_font;

    p_font = create_font(font_file, ui_size, ui_outline, code_points, ui_default_code_point);
    if (!p_font)
        return nullptr;

    if (atlas.add_font(font_name, p_font))
        return p_font;

    font_list_[font_name] = p_font;
    return p_font;
}

std::shared_ptr<material>
renderer::create_material(std::shared_ptr<render_target> p_render_target) {
    const auto& rect = p_render_target->get_rect();
    return create_material(std::move(p_render_target), rect);
}

void renderer::notify_window_resized(const vector2ui&) {}

} // namespace lxgui::gui
