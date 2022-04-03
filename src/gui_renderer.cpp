#include "lxgui/gui_renderer.hpp"

#include "lxgui/gui_atlas.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/gui_render_target.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui::gui {

void renderer::begin(std::shared_ptr<render_target> target) {
    if (is_quad_batching_enabled()) {
        current_material_ = nullptr;

        if (is_vertex_cache_enabled()) {
            try {
                if (quad_cache_[0].cache == nullptr) {
                    for (std::size_t index = 0u; index < batching_cache_cycle_size; ++index) {
                        quad_cache_[index].cache = create_vertex_cache(vertex_cache::type::quads);
                    }
                }
            } catch (const std::exception& e) {
                gui::out << gui::warning << e.what() << std::endl;
                gui::out << gui::warning
                         << "gui::renderer: Failed to create caches for quad batching. Vertex "
                            "caches will be disabled."
                         << std::endl;

                vertex_cache_enabled_ = false;
            }
        }
    }

    begin_(std::move(target));
}

void renderer::end() {
    if (is_quad_batching_enabled()) {
        flush_quad_batch();
    }

    end_();
}

void renderer::reset_counters() {
    last_frame_batch_count_  = batch_count_;
    last_frame_vertex_count_ = vertex_count_;
    batch_count_             = 0;
    vertex_count_            = 0;
}

std::size_t renderer::get_batch_count() const {
    return last_frame_batch_count_;
}

std::size_t renderer::get_vertex_count() const {
    return last_frame_vertex_count_;
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

bool renderer::uses_same_texture_(const material* mat1, const material* mat2) const {
    if (mat1 == mat2)
        return true;

    if (mat1 && mat2 && mat1->uses_same_texture(*mat2))
        return true;

    if (is_texture_vertex_color_supported()) {
        if (mat1 && mat1->is_in_atlas() && !mat2)
            return true;
        if (mat2 && mat2->is_in_atlas() && !mat1)
            return true;
    }

    return false;
}

void renderer::render_quads(
    const material* mat, const std::vector<std::array<vertex, 4>>& quad_list) {
    if (quad_list.empty())
        return;

    if (!is_quad_batching_enabled()) {
        // Render immediately
        vertex_count_ += quad_list.size() * 4;
        render_quads_(mat, quad_list);
        return;
    }

    if (!uses_same_texture_(mat, current_material_)) {
        // Render current batch and start a new one
        flush_quad_batch();
        current_material_ = mat;
    }

    if (quad_cache_[current_quad_cache_].data.empty()) {
        // Start a new batch
        current_material_ = mat;
    }

    if (!current_material_) {
        // Previous quads had no material, override with the new one
        current_material_ = mat;
    }

    // Add to the cache
    auto& cache = quad_cache_[current_quad_cache_];

    if (!mat && is_texture_atlas_enabled() && is_texture_vertex_color_supported()) {
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
    auto& cache = quad_cache_[current_quad_cache_];
    if (cache.data.empty())
        return;

    vertex_count_ += cache.data.size() * 4;

    if (cache.cache) {
        cache.cache->update(cache.data[0].data(), cache.data.size() * 4);
        render_cache_(current_material_, *cache.cache, matrix4f::identity);
    } else {
        render_quads_(current_material_, cache.data);
    }

    cache.data.clear();
    current_material_ = nullptr;

    ++current_quad_cache_;
    if (current_quad_cache_ == batching_cache_cycle_size)
        current_quad_cache_ = 0u;

    ++batch_count_;
}

void renderer::render_cache(
    const material* mat, const vertex_cache& cache, const matrix4f& model_transform) {
    if (is_quad_batching_enabled()) {
        flush_quad_batch();
    }

    vertex_count_ += cache.get_vertex_count() * 4;

    render_cache_(mat, cache, model_transform);
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
        if (std::shared_ptr<gui::material> lock = iter->second.lock())
            return lock;
        else
            texture_list_.erase(iter);
    }

    try {
        std::shared_ptr<gui::material> tex = create_material_(file_name, filt);
        texture_list_[file_name]           = tex;
        return tex;
    } catch (const std::exception& e) {
        gui::out << gui::warning << e.what() << std::endl;
        return nullptr;
    }
}

namespace {
std::string hash_font_parameters(
    const std::string&                   font_file,
    std::size_t                          size,
    std::size_t                          outline,
    const std::vector<code_point_range>& code_points,
    char32_t                             default_code_point) {
    std::string font_name = font_file + "|s" + utils::to_string(size);
    if (outline > 0u)
        font_name += "|o" + utils::to_string(outline);

    for (const code_point_range& range : code_points)
        font_name += "|c" + utils::to_string(range.first) + "-" + utils::to_string(range.last);

    font_name += "|d" + utils::to_string(default_code_point);

    return font_name;
}
} // namespace

std::shared_ptr<gui::font> renderer::create_font(
    const std::string&                   font_file,
    std::size_t                          size,
    std::size_t                          outline,
    const std::vector<code_point_range>& code_points,
    char32_t                             default_code_point) {
    const std::string font_name =
        hash_font_parameters(font_file, size, outline, code_points, default_code_point);

    auto iter = font_list_.find(font_name);
    if (iter != font_list_.end()) {
        if (std::shared_ptr<gui::font> lock = iter->second.lock())
            return lock;
        else
            font_list_.erase(iter);
    }

    std::shared_ptr<gui::font> fnt =
        create_font_(font_file, size, outline, code_points, default_code_point);

    font_list_[font_name] = fnt;
    return fnt;
}

bool renderer::is_texture_atlas_enabled() const {
    return texture_atlas_enabled_ && is_texture_atlas_supported();
}

void renderer::set_texture_atlas_enabled(bool enabled) {
    texture_atlas_enabled_ = enabled;
}

std::size_t renderer::get_texture_atlas_page_size() const {
    if (texture_atlas_page_size_ == 0u)
        return std::min<std::size_t>(4096u, get_texture_max_size());
    else
        return texture_atlas_page_size_;
}

void renderer::set_texture_atlas_page_size(std::size_t page_size) {
    texture_atlas_page_size_ = page_size;
}

std::size_t renderer::get_texture_atlas_page_count() const {
    std::size_t count = 0;

    for (const auto& page : atlas_list_) {
        count += page.second->get_page_count();
    }

    return count;
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
    std::shared_ptr<gui::atlas> atlas;

    std::string baked_atlas_name =
        utils::to_string(static_cast<std::size_t>(filt)) + '|' + atlas_category;
    auto iter = atlas_list_.find(baked_atlas_name);
    if (iter != atlas_list_.end()) {
        atlas = iter->second;
    }

    if (!atlas) {
        atlas                         = create_atlas_(filt);
        atlas_list_[baked_atlas_name] = atlas;
    }

    return *atlas;
}

std::shared_ptr<material> renderer::create_atlas_material(
    const std::string& atlas_category, const std::string& file_name, material::filter filt) {
    if (!is_texture_atlas_enabled())
        return create_material(file_name, filt);

    auto& atlas = get_atlas_(atlas_category, filt);

    auto tex = atlas.fetch_material(file_name);
    if (tex)
        return tex;

    tex = create_material(file_name, filt);
    if (!tex)
        return nullptr;

    auto added_tex = atlas.add_material(file_name, *tex);
    if (added_tex)
        return added_tex;
    else
        return tex;
}

std::shared_ptr<font> renderer::create_atlas_font(
    const std::string&                   atlas_category,
    const std::string&                   font_file,
    std::size_t                          size,
    std::size_t                          outline,
    const std::vector<code_point_range>& code_points,
    char32_t                             default_code_point) {
    if (!is_texture_atlas_enabled())
        return create_font(font_file, size, outline, code_points, default_code_point);

    auto& atlas = get_atlas_(atlas_category, material::filter::none);

    const std::string font_name =
        hash_font_parameters(font_file, size, outline, code_points, default_code_point);

    auto fnt = atlas.fetch_font(font_name);
    if (fnt)
        return fnt;

    fnt = create_font(font_file, size, outline, code_points, default_code_point);
    if (!fnt)
        return nullptr;

    if (atlas.add_font(font_name, fnt))
        return fnt;

    font_list_[font_name] = fnt;
    return fnt;
}

std::shared_ptr<material> renderer::create_material(std::shared_ptr<render_target> target) {
    const auto& rect = target->get_rect();
    return create_material(std::move(target), rect);
}

void renderer::notify_window_resized(const vector2ui&) {}

} // namespace lxgui::gui
