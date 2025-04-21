#include "lxgui/gui_backdrop.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/utils_file_system.hpp"

namespace lxgui::gui {

backdrop::backdrop(frame& parent) : parent_(parent) {}

void backdrop::copy_from(const backdrop& other) {
    this->set_background(other.get_background_file());
    this->set_edge(other.get_edge_file());

    this->set_background_tilling(other.is_background_tilling());
    this->set_tile_size(other.get_tile_size());

    if (other.background_file_.empty())
        this->set_background_color(other.get_background_color());

    this->set_background_insets(other.get_background_insets());

    if (other.edge_file_.empty())
        this->set_edge_color(other.get_edge_color());

    this->set_edge_size(other.get_edge_size());
    this->set_edge_insets(other.get_edge_insets());
}

void backdrop::set_background(const std::string& background_file) {
    if (background_file_ == background_file)
        return;

    is_cache_dirty_   = true;
    background_color_ = color::empty;

    if (background_file.empty()) {
        background_texture_ = nullptr;
        background_file_    = "";
        return;
    }

    if (!utils::file_exists(background_file)) {
        background_texture_ = nullptr;
        background_file_    = "";

        gui::out << gui::warning << "backdrop: "
                 << "Cannot find file: \"" << background_file << "\" for " << parent_.get_name()
                 << "'s backdrop background file. No background will be drawn." << std::endl;

        return;
    }

    auto& renderer      = parent_.get_manager().get_renderer();
    background_texture_ = renderer.create_atlas_material("GUI", background_file);
    if (!background_texture_) {
        return;
    }

    tile_size_ = original_tile_size_ = static_cast<float>(background_texture_->get_rect().width());
    background_file_                 = background_file;
}

const std::string& backdrop::get_background_file() const {
    return background_file_;
}

void backdrop::set_background_color(const color& c) {
    if (background_color_ == c)
        return;

    is_cache_dirty_ = true;

    background_texture_ = nullptr;
    background_color_   = c;
    background_file_    = "";

    tile_size_ = original_tile_size_ = 256.0f;
}

color backdrop::get_background_color() const {
    return background_color_;
}

void backdrop::set_background_tilling(bool is_tilling) {
    if (is_background_tilling_ == is_tilling)
        return;

    is_background_tilling_ = is_tilling;
    is_cache_dirty_        = true;
}

bool backdrop::is_background_tilling() const {
    return is_background_tilling_;
}

void backdrop::set_tile_size(float tile_size) {
    if (tile_size_ == tile_size)
        return;

    tile_size_      = tile_size;
    is_cache_dirty_ = true;
}

float backdrop::get_tile_size() const {
    return tile_size_;
}

void backdrop::set_background_insets(const bounds2f& insets) {
    if (background_insets_ == insets)
        return;

    background_insets_ = insets;
    is_cache_dirty_    = true;
}

const bounds2f& backdrop::get_background_insets() const {
    return background_insets_;
}

void backdrop::set_edge_insets(const bounds2f& insets) {
    if (edge_insets_ == insets)
        return;

    edge_insets_    = insets;
    is_cache_dirty_ = true;
}

const bounds2f& backdrop::get_edge_insets() const {
    return edge_insets_;
}

void backdrop::set_edge(const std::string& edge_file) {
    if (edge_file == edge_file_)
        return;

    is_cache_dirty_ = true;
    edge_color_     = color::empty;

    if (edge_file.empty()) {
        edge_texture_ = nullptr;
        edge_file_    = "";
        return;
    }

    if (!utils::file_exists(edge_file)) {
        edge_texture_ = nullptr;
        edge_file_    = "";

        gui::out << gui::warning << "backdrop: "
                 << "Cannot find file: \"" << edge_file << "\" for " << parent_.get_name()
                 << "'s backdrop edge. No edge will be drawn." << std::endl;

        return;
    }

    auto& renderer = parent_.get_manager().get_renderer();
    edge_texture_  = renderer.create_atlas_material("GUI", edge_file);
    if (!edge_texture_) {
        return;
    }

    if (edge_texture_->get_rect().width() / edge_texture_->get_rect().height() != 8.0f) {
        edge_texture_ = nullptr;
        edge_file_    = "";

        gui::out << gui::error << "backdrop: "
                 << "An edge texture width must be exactly 8 times greater than its height "
                 << "(in " << edge_file << "). No edge will be drawn for " << parent_.get_name()
                 << "'s backdrop." << std::endl;

        return;
    }

    edge_size_ = original_edge_size_ = edge_texture_->get_rect().height();
    edge_file_                       = edge_file;
}

const std::string& backdrop::get_edge_file() const {
    return edge_file_;
}

void backdrop::set_edge_color(const color& c) {
    if (edge_color_ == c)
        return;

    is_cache_dirty_ = true;
    edge_texture_   = nullptr;
    edge_color_     = c;
    edge_file_      = "";

    if (edge_size_ == 0.0f)
        edge_size_ = 1.0f;

    original_edge_size_ = 1.0f;
}

color backdrop::get_edge_color() const {
    return edge_color_;
}

void backdrop::set_edge_size(float edge_size) {
    if (edge_size_ == edge_size)
        return;

    edge_size_      = edge_size;
    is_cache_dirty_ = true;
}

float backdrop::get_edge_size() const {
    return edge_size_;
}

void backdrop::set_vertex_color(const color& c) {
    if (vertex_color_ == c)
        return;

    vertex_color_   = c;
    is_cache_dirty_ = true;
}

void backdrop::render() const {
    float alpha = parent_.get_effective_alpha();
    if (alpha != cache_alpha_)
        is_cache_dirty_ = true;

    auto& renderer = parent_.get_manager().get_renderer();
    bool  use_vertex_cache =
        renderer.is_vertex_cache_enabled() && !renderer.is_quad_batching_enabled();

    bool has_background = background_texture_ || background_color_ != color::empty;
    bool has_edge       = edge_texture_ || edge_color_ != color::empty;

    if (has_background) {
        if ((use_vertex_cache && !background_cache_) ||
            (!use_vertex_cache && background_quads_.empty()))
            is_cache_dirty_ = true;
    }

    if (has_edge) {
        if ((use_vertex_cache && !edge_cache_) || (!use_vertex_cache && edge_quads_.empty()))
            is_cache_dirty_ = true;
    }

    update_cache_();

    if (has_background) {
        if (use_vertex_cache && background_cache_)
            renderer.render_cache(background_texture_.get(), *background_cache_);
        else
            renderer.render_quads(background_texture_.get(), background_quads_);
    }

    if (has_edge) {
        if (use_vertex_cache && edge_cache_)
            renderer.render_cache(edge_texture_.get(), *edge_cache_);
        else
            renderer.render_quads(edge_texture_.get(), edge_quads_);
    }
}

void backdrop::notify_borders_updated() const {
    is_cache_dirty_ = true;
}

void backdrop::update_cache_() const {
    if (!is_cache_dirty_)
        return;

    background_quads_.clear();
    edge_quads_.clear();

    color color = vertex_color_;

    float alpha = parent_.get_effective_alpha();
    color.a *= alpha;

    update_background_(color);
    update_edge_(color);

    cache_alpha_    = alpha;
    is_cache_dirty_ = false;
}

void repeat_wrap(
    const frame&                        parent,
    std::vector<std::array<vertex, 4>>& output,
    const bounds2f&                     source_uvs,
    float                               tile_size,
    bool                                is_rotated,
    const color                         color,
    const bounds2f&                     destination) {
    const auto  d_top_left  = destination.top_left();
    const auto  s_top_left  = source_uvs.top_left();
    const float dest_width  = destination.width();
    const float dest_height = destination.height();

    float sy = 0.0f;
    while (sy < dest_height) {
        float d_height = tile_size;
        float s_height = source_uvs.height();
        if (sy + tile_size > dest_height)
            d_height = dest_height - sy;

        float sx = 0.0f;
        while (sx < dest_width) {
            float d_width = tile_size;
            float s_width = source_uvs.width();
            if (sx + tile_size > dest_width)
                d_width = dest_width - sx;

            output.emplace_back();
            auto& quad = output.back();

            quad[0].pos = parent.round_to_pixel(d_top_left + vector2f(sx, sy));
            quad[1].pos = parent.round_to_pixel(d_top_left + vector2f(sx + d_width, sy));
            quad[2].pos = parent.round_to_pixel(d_top_left + vector2f(sx + d_width, sy + d_height));
            quad[3].pos = parent.round_to_pixel(d_top_left + vector2f(sx, sy + d_height));

            if (is_rotated) {
                s_height *= d_width / tile_size;
                s_width *= d_height / tile_size;
                quad[0].uvs = s_top_left + vector2f(0.0f, s_height);
                quad[1].uvs = s_top_left + vector2f(0.0f, 0.0f);
                quad[2].uvs = s_top_left + vector2f(s_width, 0.0f);
                quad[3].uvs = s_top_left + vector2f(s_width, s_height);
            } else {
                s_width *= d_width / tile_size;
                s_height *= d_height / tile_size;
                quad[0].uvs = s_top_left + vector2f(0.0f, 0.0f);
                quad[1].uvs = s_top_left + vector2f(s_width, 0.0f);
                quad[2].uvs = s_top_left + vector2f(s_width, s_height);
                quad[3].uvs = s_top_left + vector2f(0.0f, s_height);
            }

            quad[0].col = quad[1].col = quad[2].col = quad[3].col = color;

            sx += d_width;
        }

        sy += d_height;
    }
}

void backdrop::update_background_(color c) const {
    if (!background_texture_ && background_color_ == color::empty)
        return;

    if (!background_texture_)
        c *= background_color_;

    auto borders = parent_.get_borders();
    borders.left += background_insets_.left;
    borders.right -= background_insets_.right;
    borders.top += background_insets_.top;
    borders.bottom -= background_insets_.bottom;

    auto& renderer = parent_.get_manager().get_renderer();

    if (background_texture_) {
        const vector2f canvas_tl  = background_texture_->get_canvas_uv(vector2f(0.0f, 0.0f), true);
        const vector2f canvas_br  = background_texture_->get_canvas_uv(vector2f(1.0f, 1.0f), true);
        const bounds2f canvas_uvs = bounds2f(canvas_tl.x, canvas_br.x, canvas_tl.y, canvas_br.y);

        float rounded_tile_size =
            parent_.round_to_pixel(tile_size_, utils::rounding_method::nearest_not_zero);

        if (background_texture_->is_in_atlas() && is_background_tilling_ &&
            rounded_tile_size > 1.0f) {
            repeat_wrap(
                parent_, background_quads_, canvas_uvs, rounded_tile_size, false, c, borders);
        } else {
            background_quads_.emplace_back();
            auto& quad = background_quads_.back();

            quad[0].pos = parent_.round_to_pixel(borders.top_left());
            quad[1].pos = parent_.round_to_pixel(borders.top_right());
            quad[2].pos = parent_.round_to_pixel(borders.bottom_right());
            quad[3].pos = parent_.round_to_pixel(borders.bottom_left());
            quad[0].uvs = canvas_uvs.top_left();
            quad[1].uvs = canvas_uvs.top_right();
            quad[2].uvs = canvas_uvs.bottom_right();
            quad[3].uvs = canvas_uvs.bottom_left();
            quad[0].col = quad[1].col = quad[2].col = quad[3].col = c;
        }
    } else {
        background_quads_.emplace_back();
        auto& quad = background_quads_.back();

        quad[0].pos = parent_.round_to_pixel(borders.top_left());
        quad[1].pos = parent_.round_to_pixel(borders.top_right());
        quad[2].pos = parent_.round_to_pixel(borders.bottom_right());
        quad[3].pos = parent_.round_to_pixel(borders.bottom_left());
        quad[0].uvs = vector2f(0.0f, 0.0f);
        quad[1].uvs = vector2f(0.0f, 0.0f);
        quad[2].uvs = vector2f(0.0f, 0.0f);
        quad[3].uvs = vector2f(0.0f, 0.0f);

        quad[0].col = quad[1].col = quad[2].col = quad[3].col = c;
    }

    if (renderer.is_vertex_cache_enabled() && !renderer.is_quad_batching_enabled()) {
        if (!background_cache_)
            background_cache_ = renderer.create_vertex_cache(vertex_cache::type::quads);

        background_cache_->update(background_quads_[0].data(), background_quads_.size() * 4);
        background_quads_.clear();
    }
}

void backdrop::update_edge_(color c) const {
    if (!edge_texture_ && edge_color_ == color::empty)
        return;

    if (!edge_texture_)
        c *= edge_color_;

    constexpr float uv_step = 1.0f / 8.0f;
    auto            borders = parent_.get_borders();
    borders.left += edge_insets_.left;
    borders.right -= edge_insets_.right;
    borders.top += edge_insets_.top;
    borders.bottom -= edge_insets_.bottom;

    auto&       renderer = parent_.get_manager().get_renderer();
    const float rounded_edge_size =
        parent_.round_to_pixel(edge_size_, utils::rounding_method::nearest_not_zero);

    auto repeat_wrap_edge = [&](const bounds2f& source_uvs, bool is_rotated,
                                const bounds2f& destination) {
        if (edge_texture_) {
            const vector2f canvas_tl = edge_texture_->get_canvas_uv(source_uvs.top_left(), true);
            const vector2f canvas_br =
                edge_texture_->get_canvas_uv(source_uvs.bottom_right(), true);
            const bounds2f canvas_uvs =
                bounds2f(canvas_tl.x, canvas_br.x, canvas_tl.y, canvas_br.y);

            if (edge_texture_->is_in_atlas() && rounded_edge_size > 1.0f) {
                repeat_wrap(
                    parent_, edge_quads_, canvas_uvs, rounded_edge_size, is_rotated, c,
                    destination);
            } else {
                edge_quads_.emplace_back();
                auto& quad = edge_quads_.back();

                quad[0].pos = parent_.round_to_pixel(destination.top_left());
                quad[1].pos = parent_.round_to_pixel(destination.top_right());
                quad[2].pos = parent_.round_to_pixel(destination.bottom_right());
                quad[3].pos = parent_.round_to_pixel(destination.bottom_left());

                if (rounded_edge_size <= 1.0f) {
                    quad[0].uvs = canvas_uvs.top_left();
                    quad[1].uvs = canvas_uvs.top_right();
                    quad[2].uvs = canvas_uvs.bottom_right();
                    quad[3].uvs = canvas_uvs.bottom_left();
                } else {
                    if (is_rotated) {
                        float factor = destination.width() / rounded_edge_size;
                        quad[0].uvs =
                            canvas_uvs.top_left() + vector2f(0.0, factor * canvas_uvs.height());
                        quad[1].uvs = canvas_uvs.top_left();
                        quad[2].uvs = canvas_uvs.top_right();
                        quad[3].uvs =
                            canvas_uvs.top_right() + vector2f(0.0, factor * canvas_uvs.height());
                    } else {
                        float factor = destination.height() / rounded_edge_size;
                        quad[0].uvs  = canvas_uvs.top_left();
                        quad[1].uvs  = canvas_uvs.top_right();
                        quad[2].uvs =
                            canvas_uvs.top_right() + vector2f(0.0, factor * canvas_uvs.height());
                        quad[3].uvs =
                            canvas_uvs.top_left() + vector2f(0.0, factor * canvas_uvs.height());
                    }
                }

                quad[0].col = quad[1].col = quad[2].col = quad[3].col = c;
            }
        } else {
            edge_quads_.emplace_back();
            auto& quad = edge_quads_.back();

            quad[0].pos = parent_.round_to_pixel(destination.top_left());
            quad[1].pos = parent_.round_to_pixel(destination.top_right());
            quad[2].pos = parent_.round_to_pixel(destination.bottom_right());
            quad[3].pos = parent_.round_to_pixel(destination.bottom_left());
            quad[0].uvs = vector2f(0.0f, 0.0f);
            quad[1].uvs = vector2f(0.0f, 0.0f);
            quad[2].uvs = vector2f(0.0f, 0.0f);
            quad[3].uvs = vector2f(0.0f, 0.0f);
            quad[0].col = quad[1].col = quad[2].col = quad[3].col = c;
        }
    };

    // Left edge
    repeat_wrap_edge(
        bounds2f(0.0f, uv_step, 0.0f, 1.0f), false,
        bounds2f(
            borders.left, borders.left + rounded_edge_size, borders.top + rounded_edge_size,
            borders.bottom - rounded_edge_size));

    // Right edge
    repeat_wrap_edge(
        bounds2f(uv_step, 2.0f * uv_step, 0.0f, 1.0f), false,
        bounds2f(
            borders.right - rounded_edge_size, borders.right, borders.top + rounded_edge_size,
            borders.bottom - rounded_edge_size));

    // Top edge
    repeat_wrap_edge(
        bounds2f(2.0f * uv_step, 3.0f * uv_step, 0.0f, 1.0f), true,
        bounds2f(
            borders.left + rounded_edge_size, borders.right - rounded_edge_size, borders.top,
            borders.top + rounded_edge_size));

    // Bottom edge
    repeat_wrap_edge(
        bounds2f(3.0f * uv_step, 4.0f * uv_step, 0.0f, 1.0f), true,
        bounds2f(
            borders.left + rounded_edge_size, borders.right - rounded_edge_size,
            borders.bottom - rounded_edge_size, borders.bottom));

    // Top-left corner
    repeat_wrap_edge(
        bounds2f(4.0f * uv_step, 5.0f * uv_step, 0.0f, 1.0f), false,
        bounds2f(
            borders.left, borders.left + rounded_edge_size, borders.top,
            borders.top + rounded_edge_size));

    // Top-right corner
    repeat_wrap_edge(
        bounds2f(5.0f * uv_step, 6.0f * uv_step, 0.0f, 1.0f), false,
        bounds2f(
            borders.right - rounded_edge_size, borders.right, borders.top,
            borders.top + rounded_edge_size));

    // Bottom-left corner
    repeat_wrap_edge(
        bounds2f(6.0f * uv_step, 7.0f * uv_step, 0.0f, 1.0f), false,
        bounds2f(
            borders.left, borders.left + rounded_edge_size, borders.bottom - rounded_edge_size,
            borders.bottom));

    // Bottom-right corner
    repeat_wrap_edge(
        bounds2f(7.0f * uv_step, 8.0f * uv_step, 0.0f, 1.0f), false,
        bounds2f(
            borders.right - rounded_edge_size, borders.right, borders.bottom - rounded_edge_size,
            borders.bottom));

    if (renderer.is_vertex_cache_enabled() && !renderer.is_quad_batching_enabled()) {
        if (!edge_cache_)
            edge_cache_ = renderer.create_vertex_cache(vertex_cache::type::quads);

        edge_cache_->update(edge_quads_[0].data(), edge_quads_.size() * 4);
        edge_quads_.clear();
    }
}

} // namespace lxgui::gui
