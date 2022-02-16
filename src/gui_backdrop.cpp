#include "lxgui/gui_backdrop.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/utils_filesystem.hpp"

namespace lxgui::gui {

backdrop::backdrop(frame& m_parent) : m_parent_(m_parent) {}

void backdrop::copy_from(const backdrop& m_backdrop) {
    this->set_background(m_backdrop.get_background_file());
    this->set_edge(m_backdrop.get_edge_file());

    this->set_background_tilling(m_backdrop.is_background_tilling());
    this->set_tile_size(m_backdrop.get_tile_size());

    if (m_backdrop.background_file_.empty())
        this->set_background_color(m_backdrop.get_background_color());

    this->set_background_insets(m_backdrop.get_background_insets());

    if (m_backdrop.edge_file_.empty())
        this->set_edge_color(m_backdrop.get_edge_color());

    this->set_edge_size(m_backdrop.get_edge_size());
    this->set_edge_insets(m_backdrop.get_edge_insets());
}

void backdrop::set_background(const std::string& background_file) {
    if (background_file_ == background_file)
        return;

    b_cache_dirty_      = true;
    m_background_color_ = color::empty;

    if (background_file.empty()) {
        p_background_texture_ = nullptr;
        background_file_      = "";
        return;
    }

    if (!utils::file_exists(background_file)) {
        p_background_texture_ = nullptr;
        background_file_      = "";

        gui::out << gui::warning << "backdrop : "
                 << "Cannot find file : \"" << background_file << "\" for " << m_parent_.get_name()
                 << "'s backdrop background file.\n"
                 << "No background will be drawn." << std::endl;

        return;
    }

    auto& m_renderer      = m_parent_.get_manager().get_renderer();
    p_background_texture_ = m_renderer.create_atlas_material("GUI", background_file);

    f_tile_size_ = f_original_tile_size_ =
        static_cast<float>(p_background_texture_->get_rect().width());
    background_file_ = background_file;
}

const std::string& backdrop::get_background_file() const {
    return background_file_;
}

void backdrop::set_background_color(const color& m_color) {
    if (m_background_color_ == m_color)
        return;

    b_cache_dirty_ = true;

    p_background_texture_ = nullptr;
    m_background_color_   = m_color;
    background_file_      = "";

    f_tile_size_ = f_original_tile_size_ = 256.0f;
}

color backdrop::get_background_color() const {
    return m_background_color_;
}

void backdrop::set_background_tilling(bool b_background_tilling) {
    if (b_background_tilling_ == b_background_tilling)
        return;

    b_background_tilling_ = b_background_tilling;
    b_cache_dirty_        = true;
}

bool backdrop::is_background_tilling() const {
    return b_background_tilling_;
}

void backdrop::set_tile_size(float f_tile_size) {
    if (f_tile_size_ == f_tile_size)
        return;

    f_tile_size_   = f_tile_size;
    b_cache_dirty_ = true;
}

float backdrop::get_tile_size() const {
    return f_tile_size_;
}

void backdrop::set_background_insets(const bounds2f& insets) {
    if (background_insets_ == insets)
        return;

    background_insets_ = insets;
    b_cache_dirty_     = true;
}

const bounds2f& backdrop::get_background_insets() const {
    return background_insets_;
}

void backdrop::set_edge_insets(const bounds2f& insets) {
    if (edge_insets_ == insets)
        return;

    edge_insets_   = insets;
    b_cache_dirty_ = true;
}

const bounds2f& backdrop::get_edge_insets() const {
    return edge_insets_;
}

void backdrop::set_edge(const std::string& edge_file) {
    if (edge_file == edge_file_)
        return;

    b_cache_dirty_ = true;
    m_edge_color_  = color::empty;

    if (edge_file.empty()) {
        p_edge_texture_ = nullptr;
        edge_file_      = "";
        return;
    }

    if (!utils::file_exists(edge_file)) {
        p_edge_texture_ = nullptr;
        edge_file_      = "";

        gui::out << gui::warning << "backdrop : "
                 << "Cannot find file : \"" << edge_file << "\" for " << m_parent_.get_name()
                 << "'s backdrop edge.\nNo edge will be drawn." << std::endl;

        return;
    }

    auto& m_renderer = m_parent_.get_manager().get_renderer();
    p_edge_texture_  = m_renderer.create_atlas_material("GUI", edge_file);

    if (p_edge_texture_->get_rect().width() / p_edge_texture_->get_rect().height() != 8.0f) {
        p_edge_texture_ = nullptr;
        edge_file_      = "";

        gui::out << gui::error << "backdrop : "
                 << "An edge texture width must be exactly 8 times greater than its height "
                 << "(in " << edge_file << ").\nNo edge will be drawn for " << m_parent_.get_name()
                 << "'s backdrop." << std::endl;

        return;
    }

    f_edge_size_ = f_original_edge_size_ = p_edge_texture_->get_rect().height();
    edge_file_                           = edge_file;
}

const std::string& backdrop::get_edge_file() const {
    return edge_file_;
}

void backdrop::set_edge_color(const color& m_color) {
    if (m_edge_color_ == m_color)
        return;

    b_cache_dirty_  = true;
    p_edge_texture_ = nullptr;
    m_edge_color_   = m_color;
    edge_file_      = "";

    if (f_edge_size_ == 0.0f)
        f_edge_size_ = 1.0f;

    f_original_edge_size_ = 1.0f;
}

color backdrop::get_edge_color() const {
    return m_edge_color_;
}

void backdrop::set_edge_size(float f_edge_size) {
    if (f_edge_size_ == f_edge_size)
        return;

    f_edge_size_   = f_edge_size;
    b_cache_dirty_ = true;
}

float backdrop::get_edge_size() const {
    return f_edge_size_;
}

void backdrop::set_vertex_color(const color& m_color) {
    if (m_vertex_color_ == m_color)
        return;

    m_vertex_color_ = m_color;
    b_cache_dirty_  = true;
}

void backdrop::render() const {
    float f_alpha = m_parent_.get_effective_alpha();
    if (f_alpha != f_cache_alpha_)
        b_cache_dirty_ = true;

    auto& m_renderer = m_parent_.get_manager().get_renderer();
    bool  b_use_vertex_cache =
        m_renderer.is_vertex_cache_enabled() && !m_renderer.is_quad_batching_enabled();

    bool b_has_background = p_background_texture_ || m_background_color_ != color::empty;
    bool b_has_edge       = p_edge_texture_ || m_edge_color_ != color::empty;

    if (b_has_background) {
        if ((b_use_vertex_cache && !p_background_cache_) ||
            (!b_use_vertex_cache && background_quads_.empty()))
            b_cache_dirty_ = true;
    }

    if (b_has_edge) {
        if ((b_use_vertex_cache && !p_edge_cache_) || (!b_use_vertex_cache && edge_quads_.empty()))
            b_cache_dirty_ = true;
    }

    update_cache_();

    if (b_has_background) {
        if (b_use_vertex_cache && p_background_cache_)
            m_renderer.render_cache(p_background_texture_.get(), *p_background_cache_);
        else
            m_renderer.render_quads(p_background_texture_.get(), background_quads_);
    }

    if (b_has_edge) {
        if (b_use_vertex_cache && p_edge_cache_)
            m_renderer.render_cache(p_edge_texture_.get(), *p_edge_cache_);
        else
            m_renderer.render_quads(p_edge_texture_.get(), edge_quads_);
    }
}

void backdrop::notify_borders_updated() const {
    b_cache_dirty_ = true;
}

void backdrop::update_cache_() const {
    if (!b_cache_dirty_)
        return;

    background_quads_.clear();
    edge_quads_.clear();

    color m_color = m_vertex_color_;

    float f_alpha = m_parent_.get_effective_alpha();
    m_color.a *= f_alpha;

    update_background_(m_color);
    update_edge_(m_color);

    f_cache_alpha_ = f_alpha;
    b_cache_dirty_ = false;
}

void repeat_wrap(
    const frame&                        m_parent,
    std::vector<std::array<vertex, 4>>& output,
    const bounds2f&                     m_source_uvs,
    float                               f_tile_size,
    bool                                b_rotated,
    const color                         m_color,
    const bounds2f&                     m_destination) {
    const auto  m_d_top_left  = m_destination.top_left();
    const auto  m_s_top_left  = m_source_uvs.top_left();
    const float f_dest_width  = m_destination.width();
    const float f_dest_height = m_destination.height();

    float f_sy = 0.0f;
    while (f_sy < f_dest_height) {
        float f_d_height = f_tile_size;
        float f_s_height = m_source_uvs.height();
        if (f_sy + f_tile_size > f_dest_height)
            f_d_height = f_dest_height - f_sy;

        float f_sx = 0.0f;
        while (f_sx < f_dest_width) {
            float f_d_width = f_tile_size;
            float f_s_width = m_source_uvs.width();
            if (f_sx + f_tile_size > f_dest_width)
                f_d_width = f_dest_width - f_sx;

            output.emplace_back();
            auto& m_quad = output.back();

            m_quad[0].pos = m_parent.round_to_pixel(m_d_top_left + vector2f(f_sx, f_sy));
            m_quad[1].pos =
                m_parent.round_to_pixel(m_d_top_left + vector2f(f_sx + f_d_width, f_sy));
            m_quad[2].pos = m_parent.round_to_pixel(
                m_d_top_left + vector2f(f_sx + f_d_width, f_sy + f_d_height));
            m_quad[3].pos =
                m_parent.round_to_pixel(m_d_top_left + vector2f(f_sx, f_sy + f_d_height));

            if (b_rotated) {
                f_s_height *= f_d_width / f_tile_size;
                f_s_width *= f_d_height / f_tile_size;
                m_quad[0].uvs = m_s_top_left + vector2f(0.0f, f_s_height);
                m_quad[1].uvs = m_s_top_left + vector2f(0.0f, 0.0f);
                m_quad[2].uvs = m_s_top_left + vector2f(f_s_width, 0.0f);
                m_quad[3].uvs = m_s_top_left + vector2f(f_s_width, f_s_height);
            } else {
                f_s_width *= f_d_width / f_tile_size;
                f_s_height *= f_d_height / f_tile_size;
                m_quad[0].uvs = m_s_top_left + vector2f(0.0f, 0.0f);
                m_quad[1].uvs = m_s_top_left + vector2f(f_s_width, 0.0f);
                m_quad[2].uvs = m_s_top_left + vector2f(f_s_width, f_s_height);
                m_quad[3].uvs = m_s_top_left + vector2f(0.0f, f_s_height);
            }

            m_quad[0].col = m_quad[1].col = m_quad[2].col = m_quad[3].col = m_color;

            f_sx += f_d_width;
        }

        f_sy += f_d_height;
    }
}

void backdrop::update_background_(color m_color) const {
    if (!p_background_texture_ && m_background_color_ == color::empty)
        return;

    if (!p_background_texture_)
        m_color *= m_background_color_;

    auto m_borders = m_parent_.get_borders();
    m_borders.left += background_insets_.left;
    m_borders.right -= background_insets_.right;
    m_borders.top += background_insets_.top;
    m_borders.bottom -= background_insets_.bottom;

    auto& m_renderer = m_parent_.get_manager().get_renderer();

    if (p_background_texture_) {
        const vector2f m_canvas_tl =
            p_background_texture_->get_canvas_uv(vector2f(0.0f, 0.0f), true);
        const vector2f m_canvas_br =
            p_background_texture_->get_canvas_uv(vector2f(1.0f, 1.0f), true);
        const bounds2f m_canvas_uvs =
            bounds2f(m_canvas_tl.x, m_canvas_br.x, m_canvas_tl.y, m_canvas_br.y);

        float f_rounded_tile_size =
            m_parent_.round_to_pixel(f_tile_size_, utils::rounding_method::nearest_not_zero);

        if (p_background_texture_->is_in_atlas() && b_background_tilling_ &&
            f_rounded_tile_size > 1.0f) {
            repeat_wrap(
                m_parent_, background_quads_, m_canvas_uvs, f_rounded_tile_size, false, m_color,
                m_borders);
        } else {
            background_quads_.emplace_back();
            auto& m_quad = background_quads_.back();

            m_quad[0].pos = m_parent_.round_to_pixel(m_borders.top_left());
            m_quad[1].pos = m_parent_.round_to_pixel(m_borders.top_right());
            m_quad[2].pos = m_parent_.round_to_pixel(m_borders.bottom_right());
            m_quad[3].pos = m_parent_.round_to_pixel(m_borders.bottom_left());
            m_quad[0].uvs = m_canvas_uvs.top_left();
            m_quad[1].uvs = m_canvas_uvs.top_right();
            m_quad[2].uvs = m_canvas_uvs.bottom_right();
            m_quad[3].uvs = m_canvas_uvs.bottom_left();
            m_quad[0].col = m_quad[1].col = m_quad[2].col = m_quad[3].col = m_color;
        }
    } else {
        background_quads_.emplace_back();
        auto& m_quad = background_quads_.back();

        m_quad[0].pos = m_parent_.round_to_pixel(m_borders.top_left());
        m_quad[1].pos = m_parent_.round_to_pixel(m_borders.top_right());
        m_quad[2].pos = m_parent_.round_to_pixel(m_borders.bottom_right());
        m_quad[3].pos = m_parent_.round_to_pixel(m_borders.bottom_left());
        m_quad[0].uvs = vector2f(0.0f, 0.0f);
        m_quad[1].uvs = vector2f(0.0f, 0.0f);
        m_quad[2].uvs = vector2f(0.0f, 0.0f);
        m_quad[3].uvs = vector2f(0.0f, 0.0f);

        m_quad[0].col = m_quad[1].col = m_quad[2].col = m_quad[3].col = m_color;
    }

    if (m_renderer.is_vertex_cache_enabled() && !m_renderer.is_quad_batching_enabled()) {
        if (!p_background_cache_)
            p_background_cache_ = m_renderer.create_vertex_cache(vertex_cache::type::quads);

        p_background_cache_->update(background_quads_[0].data(), background_quads_.size() * 4);
        background_quads_.clear();
    }
}

void backdrop::update_edge_(color mColor) const {
    if (!p_edge_texture_ && m_edge_color_ == color::empty)
        return;

    if (!p_edge_texture_)
        mColor *= m_edge_color_;

    constexpr float f_uv_step = 1.0f / 8.0f;
    auto            m_borders = m_parent_.get_borders();
    m_borders.left += edge_insets_.left;
    m_borders.right -= edge_insets_.right;
    m_borders.top += edge_insets_.top;
    m_borders.bottom -= edge_insets_.bottom;

    auto&       m_renderer = m_parent_.get_manager().get_renderer();
    const float f_rounded_edge_size =
        m_parent_.round_to_pixel(f_edge_size_, utils::rounding_method::nearest_not_zero);

    auto repeat_wrap_edge = [&](const bounds2f& m_source_uvs, bool b_rotated,
                                const bounds2f& m_destination) {
        if (p_edge_texture_) {
            const vector2f m_canvas_tl =
                p_edge_texture_->get_canvas_uv(m_source_uvs.top_left(), true);
            const vector2f m_canvas_br =
                p_edge_texture_->get_canvas_uv(m_source_uvs.bottom_right(), true);
            const bounds2f m_canvas_uvs =
                bounds2f(m_canvas_tl.x, m_canvas_br.x, m_canvas_tl.y, m_canvas_br.y);

            if (p_edge_texture_->is_in_atlas() && f_rounded_edge_size > 1.0f) {
                repeat_wrap(
                    m_parent_, edge_quads_, m_canvas_uvs, f_rounded_edge_size, b_rotated, mColor,
                    m_destination);
            } else {
                edge_quads_.emplace_back();
                auto& m_quad = edge_quads_.back();

                m_quad[0].pos = m_parent_.round_to_pixel(m_destination.top_left());
                m_quad[1].pos = m_parent_.round_to_pixel(m_destination.top_right());
                m_quad[2].pos = m_parent_.round_to_pixel(m_destination.bottom_right());
                m_quad[3].pos = m_parent_.round_to_pixel(m_destination.bottom_left());

                if (f_rounded_edge_size <= 1.0f) {
                    m_quad[0].uvs = m_canvas_uvs.top_left();
                    m_quad[1].uvs = m_canvas_uvs.top_right();
                    m_quad[2].uvs = m_canvas_uvs.bottom_right();
                    m_quad[3].uvs = m_canvas_uvs.bottom_left();
                } else {
                    if (b_rotated) {
                        float f_factor = m_destination.width() / f_rounded_edge_size;
                        m_quad[0].uvs  = m_canvas_uvs.top_left() +
                                        vector2f(0.0, f_factor * m_canvas_uvs.height());
                        m_quad[1].uvs = m_canvas_uvs.top_left();
                        m_quad[2].uvs = m_canvas_uvs.top_right();
                        m_quad[3].uvs = m_canvas_uvs.top_right() +
                                        vector2f(0.0, f_factor * m_canvas_uvs.height());
                    } else {
                        float f_factor = m_destination.height() / f_rounded_edge_size;
                        m_quad[0].uvs  = m_canvas_uvs.top_left();
                        m_quad[1].uvs  = m_canvas_uvs.top_right();
                        m_quad[2].uvs  = m_canvas_uvs.top_right() +
                                        vector2f(0.0, f_factor * m_canvas_uvs.height());
                        m_quad[3].uvs = m_canvas_uvs.top_left() +
                                        vector2f(0.0, f_factor * m_canvas_uvs.height());
                    }
                }

                m_quad[0].col = m_quad[1].col = m_quad[2].col = m_quad[3].col = mColor;
            }
        } else {
            edge_quads_.emplace_back();
            auto& m_quad = edge_quads_.back();

            m_quad[0].pos = m_parent_.round_to_pixel(m_destination.top_left());
            m_quad[1].pos = m_parent_.round_to_pixel(m_destination.top_right());
            m_quad[2].pos = m_parent_.round_to_pixel(m_destination.bottom_right());
            m_quad[3].pos = m_parent_.round_to_pixel(m_destination.bottom_left());
            m_quad[0].uvs = vector2f(0.0f, 0.0f);
            m_quad[1].uvs = vector2f(0.0f, 0.0f);
            m_quad[2].uvs = vector2f(0.0f, 0.0f);
            m_quad[3].uvs = vector2f(0.0f, 0.0f);
            m_quad[0].col = m_quad[1].col = m_quad[2].col = m_quad[3].col = mColor;
        }
    };

    // Left edge
    repeat_wrap_edge(
        bounds2f(0.0f, f_uv_step, 0.0f, 1.0f), false,
        bounds2f(
            m_borders.left, m_borders.left + f_rounded_edge_size,
            m_borders.top + f_rounded_edge_size, m_borders.bottom - f_rounded_edge_size));

    // Right edge
    repeat_wrap_edge(
        bounds2f(f_uv_step, 2.0f * f_uv_step, 0.0f, 1.0f), false,
        bounds2f(
            m_borders.right - f_rounded_edge_size, m_borders.right,
            m_borders.top + f_rounded_edge_size, m_borders.bottom - f_rounded_edge_size));

    // Top edge
    repeat_wrap_edge(
        bounds2f(2.0f * f_uv_step, 3.0f * f_uv_step, 0.0f, 1.0f), true,
        bounds2f(
            m_borders.left + f_rounded_edge_size, m_borders.right - f_rounded_edge_size,
            m_borders.top, m_borders.top + f_rounded_edge_size));

    // Bottom edge
    repeat_wrap_edge(
        bounds2f(3.0f * f_uv_step, 4.0f * f_uv_step, 0.0f, 1.0f), true,
        bounds2f(
            m_borders.left + f_rounded_edge_size, m_borders.right - f_rounded_edge_size,
            m_borders.bottom - f_rounded_edge_size, m_borders.bottom));

    // Top-left corner
    repeat_wrap_edge(
        bounds2f(4.0f * f_uv_step, 5.0f * f_uv_step, 0.0f, 1.0f), false,
        bounds2f(
            m_borders.left, m_borders.left + f_rounded_edge_size, m_borders.top,
            m_borders.top + f_rounded_edge_size));

    // Top-right corner
    repeat_wrap_edge(
        bounds2f(5.0f * f_uv_step, 6.0f * f_uv_step, 0.0f, 1.0f), false,
        bounds2f(
            m_borders.right - f_rounded_edge_size, m_borders.right, m_borders.top,
            m_borders.top + f_rounded_edge_size));

    // Bottom-left corner
    repeat_wrap_edge(
        bounds2f(6.0f * f_uv_step, 7.0f * f_uv_step, 0.0f, 1.0f), false,
        bounds2f(
            m_borders.left, m_borders.left + f_rounded_edge_size,
            m_borders.bottom - f_rounded_edge_size, m_borders.bottom));

    // Bottom-right corner
    repeat_wrap_edge(
        bounds2f(7.0f * f_uv_step, 8.0f * f_uv_step, 0.0f, 1.0f), false,
        bounds2f(
            m_borders.right - f_rounded_edge_size, m_borders.right,
            m_borders.bottom - f_rounded_edge_size, m_borders.bottom));

    if (m_renderer.is_vertex_cache_enabled() && !m_renderer.is_quad_batching_enabled()) {
        if (!p_edge_cache_)
            p_edge_cache_ = m_renderer.create_vertex_cache(vertex_cache::type::quads);

        p_edge_cache_->update(edge_quads_[0].data(), edge_quads_.size() * 4);
        edge_quads_.clear();
    }
}

} // namespace lxgui::gui
