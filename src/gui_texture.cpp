#include "lxgui/gui_texture.hpp"

#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/utils_filesystem.hpp"

#include <sstream>

namespace lxgui::gui {

texture::texture(utils::control_block& m_block, manager& m_manager) :
    layered_region(m_block, m_manager), m_renderer_(m_manager.get_renderer()) {
    type_.push_back(class_name);
}

std::string texture::serialize(const std::string& sTab) const {
    std::ostringstream str;
    str << base::serialize(sTab);

    std::visit(
        [&](const auto& m_data) {
            using content_type = std::decay_t<decltype(m_data)>;

            if constexpr (std::is_same_v<content_type, std::string>) {
                str << sTab << "  # File        : " << m_data << "\n";
            } else if constexpr (std::is_same_v<content_type, gradient>) {
                str << sTab << "  # Gradient    :\n";
                str << sTab << "  #-###\n";
                str << sTab << "  |   # min color   : " << m_data.get_min_color() << "\n";
                str << sTab << "  |   # max color   : " << m_data.get_max_color() << "\n";
                str << sTab << "  |   # orientation : ";
                switch (m_data.get_orientation()) {
                case gradient::orientation::horizontal: str << "HORIZONTAL\n"; break;
                case gradient::orientation::vertical: str << "VERTICAL\n"; break;
                default: str << "<error>\n"; break;
                }
                str << sTab << "  #-###\n";
            } else if constexpr (std::is_same_v<content_type, color>) {
                str << sTab << "  # Color       : " << m_data << "\n";
            }
        },
        m_content_);

    str << sTab << "  # Tex. coord. :\n";
    str << sTab << "  #-###\n";
    str << sTab << "  |   # top-left     : (" << m_quad_.v[0].uvs << ")\n";
    str << sTab << "  |   # top-right    : (" << m_quad_.v[1].uvs << ")\n";
    str << sTab << "  |   # bottom-right : (" << m_quad_.v[2].uvs << ")\n";
    str << sTab << "  |   # bottom-left  : (" << m_quad_.v[3].uvs << ")\n";
    str << sTab << "  #-###\n";
    str << sTab << "  # Stretching : " << is_texture_stretching_enabled_ << "\n";

    str << sTab << "  # Blend mode  : ";
    switch (m_blend_mode_) {
    case blend_mode::none: str << "NONE\n"; break;
    case blend_mode::blend: str << "BLEND\n"; break;
    case blend_mode::key: str << "KEY\n"; break;
    case blend_mode::add: str << "ADD\n"; break;
    case blend_mode::mod: str << "MOD\n"; break;
    default: str << "<error>\n"; break;
    }

    str << sTab << "  # Filter      : ";
    switch (m_filter_) {
    case material::filter::none: str << "NONE\n"; break;
    case material::filter::linear: str << "LINEAR\n"; break;
    default: str << "<error>\n"; break;
    }

    str << sTab << "  # Desaturated : " << is_desaturated_ << "\n";

    return str.str();
}

void texture::render() const {
    if (!is_visible())
        return;

    float f_alpha = get_effective_alpha();

    if (f_alpha != 1.0f) {
        quad m_blended_quad = m_quad_;
        for (std::size_t i = 0; i < 4; ++i)
            m_blended_quad.v[i].col.a *= f_alpha;

        m_renderer_.render_quad(m_blended_quad);
    } else {
        m_renderer_.render_quad(m_quad_);
    }
}

void texture::create_glue() {
    create_glue_(this);
}

void texture::copy_from(const region& m_obj) {
    base::copy_from(m_obj);

    const texture* p_texture = down_cast<texture>(&m_obj);
    if (!p_texture)
        return;

    if (p_texture->has_texture_file())
        this->set_texture(p_texture->get_texture_file());
    else if (p_texture->has_gradient())
        this->set_gradient(p_texture->get_gradient());
    else if (p_texture->has_solid_color())
        this->set_solid_color(p_texture->get_solid_color());

    this->set_blend_mode(p_texture->get_blend_mode());
    this->set_tex_coord(p_texture->get_tex_coord());
    this->set_texture_stretching(p_texture->get_texture_stretching());
    this->set_desaturated(p_texture->is_desaturated());
}

texture::blend_mode texture::get_blend_mode() const {
    return m_blend_mode_;
}

material::filter texture::get_filter_mode() const {
    return m_filter_;
}

bool texture::has_solid_color() const {
    return std::holds_alternative<color>(m_content_);
}

const color& texture::get_solid_color() const {
    return std::get<color>(m_content_);
}

bool texture::has_gradient() const {
    return std::holds_alternative<gradient>(m_content_);
}

const gradient& texture::get_gradient() const {
    return std::get<gradient>(m_content_);
}

std::array<float, 8> texture::get_tex_coord() const {
    std::array<float, 8> m_coords{};

    if (m_quad_.mat) {
        for (std::size_t i = 0; i < 4; ++i) {
            const vector2f uv   = m_quad_.mat->get_local_uv(m_quad_.v[i].uvs, true);
            m_coords[2 * i + 0] = uv.x;
            m_coords[2 * i + 1] = uv.y;
        }
    } else {
        for (std::size_t i = 0; i < 4; ++i) {
            m_coords[2 * i + 0] = m_quad_.v[i].uvs.x;
            m_coords[2 * i + 1] = m_quad_.v[i].uvs.y;
        }
    }

    return m_coords;
}

bool texture::get_texture_stretching() const {
    return is_texture_stretching_enabled_;
}

bool texture::has_texture_file() const {
    return std::holds_alternative<std::string>(m_content_);
}

const std::string& texture::get_texture_file() const {
    return std::get<std::string>(m_content_);
}

color texture::get_vertex_color(std::size_t ui_index) const {
    if (ui_index >= 4) {
        gui::out << gui::error << "gui::" << type_.back() << " : "
                 << "Vertex index out of bound (" << ui_index << ")." << std::endl;
        return color::white;
    }

    return m_quad_.v[ui_index].col;
}

bool texture::is_desaturated() const {
    return is_desaturated_;
}

void texture::set_blend_mode(blend_mode m_blend_mode) {
    if (m_blend_mode != blend_mode::blend) {
        gui::out << gui::warning << "gui::" << type_.back() << " : "
                 << "texture::set_blend_mode other than \"BLEND\" is not yet implemented."
                 << std::endl;
        return;
    }

    if (m_blend_mode_ == m_blend_mode)
        return;

    m_blend_mode_ = m_blend_mode;

    notify_renderer_need_redraw();
}

void texture::set_blend_mode(const std::string& blend_mode_name) {
    blend_mode m_new_blend_mode = blend_mode::blend;

    if (blend_mode_name == "BLEND")
        m_blend_mode_ = blend_mode::blend;
    else if (blend_mode_name == "ADD")
        m_blend_mode_ = blend_mode::add;
    else if (blend_mode_name == "MOD")
        m_blend_mode_ = blend_mode::mod;
    else if (blend_mode_name == "KEY")
        m_blend_mode_ = blend_mode::key;
    else if (blend_mode_name == "NONE")
        m_blend_mode_ = blend_mode::none;
    else {
        gui::out << gui::warning << "gui::" << type_.back() << " : "
                 << "Unknown blending : \"" << blend_mode_name << "\". Using \"BLEND\"."
                 << std::endl;
    }

    set_blend_mode(m_new_blend_mode);
}

void texture::set_filter_mode(material::filter m_filter) {
    if (m_filter_ == m_filter)
        return;

    m_filter_ = m_filter;

    if (std::holds_alternative<std::string>(m_content_)) {
        // Force re-load of the material
        std::string file_name = std::get<std::string>(m_content_);
        m_content_            = std::string{};
        set_texture(file_name);
    }
}

void texture::set_filter_mode(const std::string& filter_name) {
    material::filter m_new_filter = material::filter::none;

    if (filter_name == "NONE")
        m_new_filter = material::filter::none;
    else if (filter_name == "LINEAR")
        m_new_filter = material::filter::linear;
    else {
        gui::out << gui::warning << "gui::" << type_.back() << " : "
                 << "Unknown filtering : \"" << filter_name << "\". Using \"NONE\"." << std::endl;
    }

    set_filter_mode(m_new_filter);
}

void texture::set_desaturated(bool is_desaturated) {
    if (is_desaturated_ == is_desaturated)
        return;

    is_desaturated_ = is_desaturated;
    if (is_desaturated) {
        gui::out << gui::warning << "gui::" << type_.back() << " : "
                 << "Texture de-saturation is not yet implemented." << std::endl;
    }

    notify_renderer_need_redraw();
}

void texture::set_gradient(const gradient& m_gradient) {
    m_content_ = m_gradient;

    m_quad_.mat = nullptr;

    if (m_gradient.get_orientation() == gradient::orientation::horizontal) {
        m_quad_.v[0].col = m_gradient.get_min_color();
        m_quad_.v[1].col = m_gradient.get_max_color();
        m_quad_.v[2].col = m_gradient.get_max_color();
        m_quad_.v[3].col = m_gradient.get_min_color();
    } else {
        m_quad_.v[0].col = m_gradient.get_min_color();
        m_quad_.v[1].col = m_gradient.get_min_color();
        m_quad_.v[2].col = m_gradient.get_max_color();
        m_quad_.v[3].col = m_gradient.get_max_color();
    }

    notify_renderer_need_redraw();
}

void texture::set_tex_rect(const std::array<float, 4>& texture_rect) {
    if (m_quad_.mat) {
        m_quad_.v[0].uvs =
            m_quad_.mat->get_canvas_uv(vector2f(texture_rect[0], texture_rect[1]), true);
        m_quad_.v[1].uvs =
            m_quad_.mat->get_canvas_uv(vector2f(texture_rect[2], texture_rect[1]), true);
        m_quad_.v[2].uvs =
            m_quad_.mat->get_canvas_uv(vector2f(texture_rect[2], texture_rect[3]), true);
        m_quad_.v[3].uvs =
            m_quad_.mat->get_canvas_uv(vector2f(texture_rect[0], texture_rect[3]), true);

        if (!is_texture_stretching_enabled_)
            update_dimensions_from_tex_coord_();
    } else {
        m_quad_.v[0].uvs = vector2f(texture_rect[0], texture_rect[1]);
        m_quad_.v[1].uvs = vector2f(texture_rect[2], texture_rect[1]);
        m_quad_.v[2].uvs = vector2f(texture_rect[2], texture_rect[3]);
        m_quad_.v[3].uvs = vector2f(texture_rect[0], texture_rect[3]);
    }

    notify_renderer_need_redraw();
}

void texture::set_tex_coord(const std::array<float, 8>& texture_coords) {
    if (m_quad_.mat) {
        m_quad_.v[0].uvs =
            m_quad_.mat->get_canvas_uv(vector2f(texture_coords[0], texture_coords[1]), true);
        m_quad_.v[1].uvs =
            m_quad_.mat->get_canvas_uv(vector2f(texture_coords[2], texture_coords[3]), true);
        m_quad_.v[2].uvs =
            m_quad_.mat->get_canvas_uv(vector2f(texture_coords[4], texture_coords[5]), true);
        m_quad_.v[3].uvs =
            m_quad_.mat->get_canvas_uv(vector2f(texture_coords[6], texture_coords[7]), true);

        if (!is_texture_stretching_enabled_)
            update_dimensions_from_tex_coord_();
    } else {
        m_quad_.v[0].uvs = vector2f(texture_coords[0], texture_coords[1]);
        m_quad_.v[1].uvs = vector2f(texture_coords[2], texture_coords[3]);
        m_quad_.v[2].uvs = vector2f(texture_coords[4], texture_coords[5]);
        m_quad_.v[3].uvs = vector2f(texture_coords[6], texture_coords[7]);
    }

    notify_renderer_need_redraw();
}

void texture::set_texture_stretching(bool texture_stretching) {
    if (is_texture_stretching_enabled_ != texture_stretching) {
        is_texture_stretching_enabled_ = texture_stretching;

        if (!is_texture_stretching_enabled_ && m_quad_.mat)
            update_dimensions_from_tex_coord_();
    }
}

void texture::update_dimensions_from_tex_coord_() {
    vector2f m_extent = m_quad_.v[2].uvs - m_quad_.v[0].uvs;
    set_dimensions(m_extent * vector2f(m_quad_.mat->get_canvas_dimensions()));
}

void texture::set_texture(const std::string& file_name) {
    std::string parsed_file = parse_file_name(file_name);
    m_content_              = parsed_file;

    if (parsed_file.empty())
        return;

    auto& m_renderer = get_manager().get_renderer();

    std::shared_ptr<gui::material> p_mat;
    if (utils::file_exists(parsed_file))
        p_mat = m_renderer.create_atlas_material("GUI", parsed_file, m_filter_);

    m_quad_.mat = p_mat;

    if (p_mat) {
        m_quad_.v[0].uvs = m_quad_.mat->get_canvas_uv(vector2f(0, 0), true);
        m_quad_.v[1].uvs = m_quad_.mat->get_canvas_uv(vector2f(1, 0), true);
        m_quad_.v[2].uvs = m_quad_.mat->get_canvas_uv(vector2f(1, 1), true);
        m_quad_.v[3].uvs = m_quad_.mat->get_canvas_uv(vector2f(0, 1), true);

        if (!is_apparent_width_defined())
            set_width(m_quad_.mat->get_rect().width());

        if (!is_apparent_height_defined())
            set_height(m_quad_.mat->get_rect().height());
    } else {
        gui::out << gui::error << "gui::" << type_.back() << " : "
                 << "Cannot load file \"" << parsed_file << "\" for \"" << name_
                 << "\".\nUsing white texture instead." << std::endl;
    }

    notify_renderer_need_redraw();
}

void texture::set_texture(std::shared_ptr<render_target> p_render_target) {
    m_content_ = std::string{};

    auto& m_renderer = get_manager().get_renderer();

    std::shared_ptr<gui::material> p_mat;
    if (p_render_target)
        p_mat = m_renderer.create_material(std::move(p_render_target));

    m_quad_.mat = p_mat;

    if (p_mat) {
        m_quad_.v[0].uvs = m_quad_.mat->get_canvas_uv(vector2f(0, 0), true);
        m_quad_.v[1].uvs = m_quad_.mat->get_canvas_uv(vector2f(1, 0), true);
        m_quad_.v[2].uvs = m_quad_.mat->get_canvas_uv(vector2f(1, 1), true);
        m_quad_.v[3].uvs = m_quad_.mat->get_canvas_uv(vector2f(0, 1), true);

        if (!is_apparent_width_defined())
            set_width(m_quad_.mat->get_rect().width());

        if (!is_apparent_height_defined())
            set_height(m_quad_.mat->get_rect().height());
    } else {
        gui::out << gui::error << "gui::" << type_.back() << " : "
                 << "Cannot create a texture from render target.\n"
                    "Using white texture instead."
                 << std::endl;
    }

    notify_renderer_need_redraw();
}

void texture::set_solid_color(const color& m_color) {
    m_content_ = m_color;

    m_quad_.mat      = nullptr;
    m_quad_.v[0].col = m_color;
    m_quad_.v[1].col = m_color;
    m_quad_.v[2].col = m_color;
    m_quad_.v[3].col = m_color;

    notify_renderer_need_redraw();
}

void texture::set_quad(const quad& m_quad) {
    m_content_ = std::string{};

    m_quad_           = m_quad;
    vector2f m_extent = m_quad_.v[2].pos - m_quad_.v[0].pos;
    set_dimensions(m_extent);

    notify_renderer_need_redraw();
}

void texture::set_vertex_color(const color& m_color, std::size_t ui_index) {
    if (ui_index == std::numeric_limits<std::size_t>::max()) {
        for (std::size_t i = 0; i < 4; ++i)
            m_quad_.v[i].col = m_color;

        notify_renderer_need_redraw();
        return;
    }

    if (ui_index >= 4) {
        gui::out << gui::error << "gui::" << type_.back() << " : "
                 << "Vertex index out of bound (" << ui_index << ")." << std::endl;
        return;
    }

    m_quad_.v[ui_index].col = m_color;

    notify_renderer_need_redraw();
}

void texture::update_borders_() {
    base::update_borders_();

    m_quad_.v[0].pos = border_list_.top_left();
    m_quad_.v[1].pos = border_list_.top_right();
    m_quad_.v[2].pos = border_list_.bottom_right();
    m_quad_.v[3].pos = border_list_.bottom_left();
}

} // namespace lxgui::gui
