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
    std::ostringstream s_str;
    s_str << base::serialize(sTab);

    std::visit(
        [&](const auto& m_data) {
            using content_type = std::decay_t<decltype(m_data)>;

            if constexpr (std::is_same_v<content_type, std::string>) {
                s_str << sTab << "  # File        : " << m_data << "\n";
            } else if constexpr (std::is_same_v<content_type, gradient>) {
                s_str << sTab << "  # Gradient    :\n";
                s_str << sTab << "  #-###\n";
                s_str << sTab << "  |   # min color   : " << m_data.get_min_color() << "\n";
                s_str << sTab << "  |   # max color   : " << m_data.get_max_color() << "\n";
                s_str << sTab << "  |   # orientation : ";
                switch (m_data.get_orientation()) {
                case gradient::orientation::horizontal: s_str << "HORIZONTAL\n"; break;
                case gradient::orientation::vertical: s_str << "VERTICAL\n"; break;
                default: s_str << "<error>\n"; break;
                }
                s_str << sTab << "  #-###\n";
            } else if constexpr (std::is_same_v<content_type, color>) {
                s_str << sTab << "  # Color       : " << m_data << "\n";
            }
        },
        m_content_);

    s_str << sTab << "  # Tex. coord. :\n";
    s_str << sTab << "  #-###\n";
    s_str << sTab << "  |   # top-left     : (" << m_quad_.v[0].uvs << ")\n";
    s_str << sTab << "  |   # top-right    : (" << m_quad_.v[1].uvs << ")\n";
    s_str << sTab << "  |   # bottom-right : (" << m_quad_.v[2].uvs << ")\n";
    s_str << sTab << "  |   # bottom-left  : (" << m_quad_.v[3].uvs << ")\n";
    s_str << sTab << "  #-###\n";
    s_str << sTab << "  # TexCModRect : " << b_tex_coord_modifies_rect_ << "\n";

    s_str << sTab << "  # Blend mode  : ";
    switch (m_blend_mode_) {
    case blend_mode::none: s_str << "NONE\n"; break;
    case blend_mode::blend: s_str << "BLEND\n"; break;
    case blend_mode::key: s_str << "KEY\n"; break;
    case blend_mode::add: s_str << "ADD\n"; break;
    case blend_mode::mod: s_str << "MOD\n"; break;
    default: s_str << "<error>\n"; break;
    }

    s_str << sTab << "  # Filter      : ";
    switch (m_filter_) {
    case material::filter::none: s_str << "NONE\n"; break;
    case material::filter::linear: s_str << "LINEAR\n"; break;
    default: s_str << "<error>\n"; break;
    }

    s_str << sTab << "  # Desaturated : " << b_is_desaturated_ << "\n";

    return s_str.str();
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
    this->set_tex_coord_modifies_rect(p_texture->get_tex_coord_modifies_rect());
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

bool texture::get_tex_coord_modifies_rect() const {
    return b_tex_coord_modifies_rect_;
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
    return b_is_desaturated_;
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

void texture::set_blend_mode(const std::string& s_blend_mode) {
    blend_mode m_new_blend_mode = blend_mode::blend;

    if (s_blend_mode == "BLEND")
        m_blend_mode_ = blend_mode::blend;
    else if (s_blend_mode == "ADD")
        m_blend_mode_ = blend_mode::add;
    else if (s_blend_mode == "MOD")
        m_blend_mode_ = blend_mode::mod;
    else if (s_blend_mode == "KEY")
        m_blend_mode_ = blend_mode::key;
    else if (s_blend_mode == "NONE")
        m_blend_mode_ = blend_mode::none;
    else {
        gui::out << gui::warning << "gui::" << type_.back() << " : "
                 << "Unknown blending : \"" << s_blend_mode << "\". Using \"BLEND\"." << std::endl;
    }

    set_blend_mode(m_new_blend_mode);
}

void texture::set_filter_mode(material::filter m_filter) {
    if (m_filter_ == m_filter)
        return;

    m_filter_ = m_filter;

    if (std::holds_alternative<std::string>(m_content_)) {
        // Force re-load of the material
        std::string s_file_name = std::get<std::string>(m_content_);
        m_content_              = std::string{};
        set_texture(s_file_name);
    }
}

void texture::set_filter_mode(const std::string& s_filter) {
    material::filter m_new_filter = material::filter::none;

    if (s_filter == "NONE")
        m_new_filter = material::filter::none;
    else if (s_filter == "LINEAR")
        m_new_filter = material::filter::linear;
    else {
        gui::out << gui::warning << "gui::" << type_.back() << " : "
                 << "Unknown filtering : \"" << s_filter << "\". Using \"NONE\"." << std::endl;
    }

    set_filter_mode(m_new_filter);
}

void texture::set_desaturated(bool b_is_desaturated) {
    if (b_is_desaturated_ == b_is_desaturated)
        return;

    b_is_desaturated_ = b_is_desaturated;
    if (b_is_desaturated) {
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

        if (b_tex_coord_modifies_rect_)
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

        if (b_tex_coord_modifies_rect_)
            update_dimensions_from_tex_coord_();
    } else {
        m_quad_.v[0].uvs = vector2f(texture_coords[0], texture_coords[1]);
        m_quad_.v[1].uvs = vector2f(texture_coords[2], texture_coords[3]);
        m_quad_.v[2].uvs = vector2f(texture_coords[4], texture_coords[5]);
        m_quad_.v[3].uvs = vector2f(texture_coords[6], texture_coords[7]);
    }

    notify_renderer_need_redraw();
}

void texture::set_tex_coord_modifies_rect(bool b_tex_coord_modifies_rect) {
    if (b_tex_coord_modifies_rect_ != b_tex_coord_modifies_rect) {
        b_tex_coord_modifies_rect_ = b_tex_coord_modifies_rect;

        if (b_tex_coord_modifies_rect_ && m_quad_.mat)
            update_dimensions_from_tex_coord_();
    }
}

void texture::update_dimensions_from_tex_coord_() {
    vector2f m_extent = m_quad_.v[2].uvs - m_quad_.v[0].uvs;
    set_dimensions(m_extent * vector2f(m_quad_.mat->get_canvas_dimensions()));
}

void texture::set_texture(const std::string& s_file) {
    std::string s_parsed_file = parse_file_name(s_file);
    m_content_                = s_parsed_file;

    if (s_parsed_file.empty())
        return;

    auto& m_renderer = get_manager().get_renderer();

    std::shared_ptr<gui::material> p_mat;
    if (utils::file_exists(s_parsed_file))
        p_mat = m_renderer.create_atlas_material("GUI", s_parsed_file, m_filter_);

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
                 << "Cannot load file \"" << s_parsed_file << "\" for \"" << s_name_
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
