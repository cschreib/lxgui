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

texture::texture(utils::control_block& block, manager& mgr) :
    layered_region(block, mgr), renderer_(mgr.get_renderer()) {
    type_.push_back(class_name);
}

std::string texture::serialize(const std::string& tab) const {
    std::ostringstream str;
    str << base::serialize(tab);

    std::visit(
        [&](const auto& data) {
            using content_type = std::decay_t<decltype(data)>;

            if constexpr (std::is_same_v<content_type, std::string>) {
                str << tab << "  # File        : " << data << "\n";
            } else if constexpr (std::is_same_v<content_type, gradient>) {
                str << tab << "  # Gradient    :\n";
                str << tab << "  #-###\n";
                str << tab << "  |   # min color   : " << data.min_color << "\n";
                str << tab << "  |   # max color   : " << data.max_color << "\n";
                str << tab << "  |   # orientation : ";
                switch (data.orient) {
                case gradient::orientation::horizontal: str << "HORIZONTAL\n"; break;
                case gradient::orientation::vertical: str << "VERTICAL\n"; break;
                default: str << "<error>\n"; break;
                }
                str << tab << "  #-###\n";
            } else if constexpr (std::is_same_v<content_type, color>) {
                str << tab << "  # Color       : " << data << "\n";
            }
        },
        content_);

    str << tab << "  # Tex. coord. :\n";
    str << tab << "  #-###\n";
    str << tab << "  |   # top-left     : (" << quad_.v[0].uvs << ")\n";
    str << tab << "  |   # top-right    : (" << quad_.v[1].uvs << ")\n";
    str << tab << "  |   # bottom-right : (" << quad_.v[2].uvs << ")\n";
    str << tab << "  |   # bottom-left  : (" << quad_.v[3].uvs << ")\n";
    str << tab << "  #-###\n";
    str << tab << "  # Stretching : " << is_texture_stretching_enabled_ << "\n";

    str << tab << "  # Blend mode  : ";
    switch (blend_mode_) {
    case blend_mode::none: str << "NONE\n"; break;
    case blend_mode::blend: str << "BLEND\n"; break;
    case blend_mode::key: str << "KEY\n"; break;
    case blend_mode::add: str << "ADD\n"; break;
    case blend_mode::mod: str << "MOD\n"; break;
    default: str << "<error>\n"; break;
    }

    str << tab << "  # Filter      : ";
    switch (filter_) {
    case material::filter::none: str << "NONE\n"; break;
    case material::filter::linear: str << "LINEAR\n"; break;
    default: str << "<error>\n"; break;
    }

    str << tab << "  # Desaturated : " << is_desaturated_ << "\n";

    return str.str();
}

void texture::render() const {
    if (!is_visible())
        return;

    float alpha = get_effective_alpha();

    if (alpha != 1.0f) {
        quad blended_quad = quad_;
        for (std::size_t i = 0; i < 4; ++i)
            blended_quad.v[i].col.a *= alpha;

        renderer_.render_quad(blended_quad);
    } else {
        renderer_.render_quad(quad_);
    }
}

void texture::create_glue() {
    create_glue_(this);
}

void texture::copy_from(const region& obj) {
    base::copy_from(obj);

    const texture* tex_obj = down_cast<texture>(&obj);
    if (!tex_obj)
        return;

    if (tex_obj->has_texture_file())
        this->set_texture(tex_obj->get_texture_file());
    else if (tex_obj->has_gradient())
        this->set_gradient(tex_obj->get_gradient());
    else if (tex_obj->has_solid_color())
        this->set_solid_color(tex_obj->get_solid_color());

    this->set_blend_mode(tex_obj->get_blend_mode());
    this->set_tex_coord(tex_obj->get_tex_coord());
    this->set_texture_stretching(tex_obj->get_texture_stretching());
    this->set_desaturated(tex_obj->is_desaturated());
}

texture::blend_mode texture::get_blend_mode() const {
    return blend_mode_;
}

material::filter texture::get_filter_mode() const {
    return filter_;
}

bool texture::has_solid_color() const {
    return std::holds_alternative<color>(content_);
}

const color& texture::get_solid_color() const {
    return std::get<color>(content_);
}

bool texture::has_gradient() const {
    return std::holds_alternative<gradient>(content_);
}

const gradient& texture::get_gradient() const {
    return std::get<gradient>(content_);
}

std::array<float, 8> texture::get_tex_coord() const {
    std::array<float, 8> coords{};

    if (quad_.mat) {
        for (std::size_t i = 0; i < 4; ++i) {
            const vector2f uv = quad_.mat->get_local_uv(quad_.v[i].uvs, true);
            coords[2 * i + 0] = uv.x;
            coords[2 * i + 1] = uv.y;
        }
    } else {
        for (std::size_t i = 0; i < 4; ++i) {
            coords[2 * i + 0] = quad_.v[i].uvs.x;
            coords[2 * i + 1] = quad_.v[i].uvs.y;
        }
    }

    return coords;
}

bool texture::get_texture_stretching() const {
    return is_texture_stretching_enabled_;
}

bool texture::has_texture_file() const {
    return std::holds_alternative<std::string>(content_);
}

const std::string& texture::get_texture_file() const {
    return std::get<std::string>(content_);
}

color texture::get_vertex_color(std::size_t index) const {
    if (index >= 4) {
        gui::out << gui::error << "gui::" << type_.back() << " : "
                 << "Vertex index out of bound (" << index << ")." << std::endl;
        return color::white;
    }

    return quad_.v[index].col;
}

bool texture::is_desaturated() const {
    return is_desaturated_;
}

void texture::set_blend_mode(blend_mode mode) {
    if (mode != blend_mode::blend) {
        gui::out << gui::warning << "gui::" << type_.back() << " : "
                 << "texture::set_blend_mode other than \"BLEND\" is not yet implemented."
                 << std::endl;
        return;
    }

    if (blend_mode_ == mode)
        return;

    blend_mode_ = mode;

    notify_renderer_need_redraw();
}

void texture::set_blend_mode(const std::string& blend_mode_name) {
    blend_mode mode = blend_mode::blend;

    if (blend_mode_name == "BLEND")
        mode = blend_mode::blend;
    else if (blend_mode_name == "ADD")
        mode = blend_mode::add;
    else if (blend_mode_name == "MOD")
        mode = blend_mode::mod;
    else if (blend_mode_name == "KEY")
        mode = blend_mode::key;
    else if (blend_mode_name == "NONE")
        mode = blend_mode::none;
    else {
        gui::out << gui::warning << "gui::" << type_.back() << " : "
                 << "Unknown blending : \"" << blend_mode_name << "\". Using \"BLEND\"."
                 << std::endl;
    }

    set_blend_mode(mode);
}

void texture::set_filter_mode(material::filter filt) {
    if (filter_ == filt)
        return;

    filter_ = filt;

    if (std::holds_alternative<std::string>(content_)) {
        // Force re-load of the material
        std::string file_name = std::get<std::string>(content_);
        content_              = std::string{};
        set_texture(file_name);
    }
}

void texture::set_filter_mode(const std::string& filter_name) {
    material::filter filt = material::filter::none;

    if (filter_name == "NONE")
        filt = material::filter::none;
    else if (filter_name == "LINEAR")
        filt = material::filter::linear;
    else {
        gui::out << gui::warning << "gui::" << type_.back() << " : "
                 << "Unknown filtering : \"" << filter_name << "\". Using \"NONE\"." << std::endl;
    }

    set_filter_mode(filt);
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

void texture::set_gradient(const gradient& g) {
    content_ = g;

    quad_.mat = nullptr;

    if (g.orient == gradient::orientation::horizontal) {
        quad_.v[0].col = g.min_color;
        quad_.v[1].col = g.max_color;
        quad_.v[2].col = g.max_color;
        quad_.v[3].col = g.min_color;
    } else {
        quad_.v[0].col = g.min_color;
        quad_.v[1].col = g.min_color;
        quad_.v[2].col = g.max_color;
        quad_.v[3].col = g.max_color;
    }

    notify_renderer_need_redraw();
}

void texture::set_tex_rect(const std::array<float, 4>& texture_rect) {
    if (quad_.mat) {
        quad_.v[0].uvs = quad_.mat->get_canvas_uv(vector2f(texture_rect[0], texture_rect[1]), true);
        quad_.v[1].uvs = quad_.mat->get_canvas_uv(vector2f(texture_rect[2], texture_rect[1]), true);
        quad_.v[2].uvs = quad_.mat->get_canvas_uv(vector2f(texture_rect[2], texture_rect[3]), true);
        quad_.v[3].uvs = quad_.mat->get_canvas_uv(vector2f(texture_rect[0], texture_rect[3]), true);

        if (!is_texture_stretching_enabled_)
            update_dimensions_from_tex_coord_();
    } else {
        quad_.v[0].uvs = vector2f(texture_rect[0], texture_rect[1]);
        quad_.v[1].uvs = vector2f(texture_rect[2], texture_rect[1]);
        quad_.v[2].uvs = vector2f(texture_rect[2], texture_rect[3]);
        quad_.v[3].uvs = vector2f(texture_rect[0], texture_rect[3]);
    }

    notify_renderer_need_redraw();
}

void texture::set_tex_coord(const std::array<float, 8>& texture_coords) {
    if (quad_.mat) {
        quad_.v[0].uvs =
            quad_.mat->get_canvas_uv(vector2f(texture_coords[0], texture_coords[1]), true);
        quad_.v[1].uvs =
            quad_.mat->get_canvas_uv(vector2f(texture_coords[2], texture_coords[3]), true);
        quad_.v[2].uvs =
            quad_.mat->get_canvas_uv(vector2f(texture_coords[4], texture_coords[5]), true);
        quad_.v[3].uvs =
            quad_.mat->get_canvas_uv(vector2f(texture_coords[6], texture_coords[7]), true);

        if (!is_texture_stretching_enabled_)
            update_dimensions_from_tex_coord_();
    } else {
        quad_.v[0].uvs = vector2f(texture_coords[0], texture_coords[1]);
        quad_.v[1].uvs = vector2f(texture_coords[2], texture_coords[3]);
        quad_.v[2].uvs = vector2f(texture_coords[4], texture_coords[5]);
        quad_.v[3].uvs = vector2f(texture_coords[6], texture_coords[7]);
    }

    notify_renderer_need_redraw();
}

void texture::set_texture_stretching(bool texture_stretching) {
    if (is_texture_stretching_enabled_ != texture_stretching) {
        is_texture_stretching_enabled_ = texture_stretching;

        if (!is_texture_stretching_enabled_ && quad_.mat)
            update_dimensions_from_tex_coord_();
    }
}

void texture::update_dimensions_from_tex_coord_() {
    vector2f extent = quad_.v[2].uvs - quad_.v[0].uvs;
    set_dimensions(extent * vector2f(quad_.mat->get_canvas_dimensions()));
}

void texture::set_texture(const std::string& file_name) {
    std::string parsed_file = parse_file_name(file_name);
    content_                = parsed_file;

    if (parsed_file.empty())
        return;

    auto& renderer = get_manager().get_renderer();

    std::shared_ptr<gui::material> mat;
    if (utils::file_exists(parsed_file))
        mat = renderer.create_atlas_material("GUI", parsed_file, filter_);

    quad_.mat = mat;

    if (mat) {
        quad_.v[0].uvs = quad_.mat->get_canvas_uv(vector2f(0, 0), true);
        quad_.v[1].uvs = quad_.mat->get_canvas_uv(vector2f(1, 0), true);
        quad_.v[2].uvs = quad_.mat->get_canvas_uv(vector2f(1, 1), true);
        quad_.v[3].uvs = quad_.mat->get_canvas_uv(vector2f(0, 1), true);

        if (!is_apparent_width_defined())
            set_width(quad_.mat->get_rect().width());

        if (!is_apparent_height_defined())
            set_height(quad_.mat->get_rect().height());
    } else {
        gui::out << gui::error << "gui::" << type_.back() << " : "
                 << "Cannot load file \"" << parsed_file << "\" for \"" << name_
                 << "\".\nUsing white texture instead." << std::endl;
    }

    notify_renderer_need_redraw();
}

void texture::set_texture(std::shared_ptr<render_target> target) {
    content_ = std::string{};

    auto& renderer = get_manager().get_renderer();

    std::shared_ptr<gui::material> mat;
    if (target)
        mat = renderer.create_material(std::move(target));

    quad_.mat = mat;

    if (mat) {
        quad_.v[0].uvs = quad_.mat->get_canvas_uv(vector2f(0, 0), true);
        quad_.v[1].uvs = quad_.mat->get_canvas_uv(vector2f(1, 0), true);
        quad_.v[2].uvs = quad_.mat->get_canvas_uv(vector2f(1, 1), true);
        quad_.v[3].uvs = quad_.mat->get_canvas_uv(vector2f(0, 1), true);

        if (!is_apparent_width_defined())
            set_width(quad_.mat->get_rect().width());

        if (!is_apparent_height_defined())
            set_height(quad_.mat->get_rect().height());
    } else {
        gui::out << gui::error << "gui::" << type_.back() << " : "
                 << "Cannot create a texture from render target.\n"
                    "Using white texture instead."
                 << std::endl;
    }

    notify_renderer_need_redraw();
}

void texture::set_solid_color(const color& c) {
    content_ = c;

    quad_.mat      = nullptr;
    quad_.v[0].col = c;
    quad_.v[1].col = c;
    quad_.v[2].col = c;
    quad_.v[3].col = c;

    notify_renderer_need_redraw();
}

void texture::set_quad(const quad& q) {
    content_ = std::string{};

    quad_           = q;
    vector2f extent = quad_.v[2].pos - quad_.v[0].pos;
    set_dimensions(extent);

    notify_renderer_need_redraw();
}

void texture::set_vertex_color(const color& c, std::size_t index) {
    if (index == std::numeric_limits<std::size_t>::max()) {
        for (std::size_t i = 0; i < 4; ++i)
            quad_.v[i].col = c;

        notify_renderer_need_redraw();
        return;
    }

    if (index >= 4) {
        gui::out << gui::error << "gui::" << type_.back() << " : "
                 << "Vertex index out of bound (" << index << ")." << std::endl;
        return;
    }

    quad_.v[index].col = c;

    notify_renderer_need_redraw();
}

void texture::update_borders_() {
    base::update_borders_();

    quad_.v[0].pos = border_list_.top_left();
    quad_.v[1].pos = border_list_.top_right();
    quad_.v[2].pos = border_list_.bottom_right();
    quad_.v[3].pos = border_list_.bottom_left();
}

} // namespace lxgui::gui
