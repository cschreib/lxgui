#include "lxgui/gui_animated_texture.hpp"

#include "lxgui/gui_layered_region.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_material.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/utils_file_system.hpp"

#include <sstream>

namespace lxgui::gui {

animated_texture::animated_texture(
    utils::control_block& block, manager& mgr, const region_core_attributes& attr) :
    layered_region(block, mgr, attr), renderer_(mgr.get_renderer()) {

    initialize_(*this, attr);
}

std::string animated_texture::serialize(const std::string& tab) const {
    std::ostringstream str;
    str << base::serialize(tab);
    str << tab << "  # File       : " << file_ << "\n";
    str << tab << "  # Speed      : " << speed_ << "\n";
    str << tab << "  # State      : " << state_ << "\n";
    str << tab << "  # Paused     : " << is_paused_ << "\n";
    return str.str();
}

void animated_texture::render() const {
    base::render();

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

void animated_texture::update(float delta) {
    base::update(delta);

    if (is_paused_ || !quad_.mat)
        return;

    int num_frames = quad_.mat->get_rect().width() / quad_.mat->get_rect().height();

    int old_frame =
        std::clamp(static_cast<int>(std::round(state_ * num_frames - 0.5)), 0, num_frames - 1);

    state_ += delta * speed_ / num_frames;
    state_ = std::fmod(state_, 1.0f);

    int new_frame =
        std::clamp(static_cast<int>(std::round(state_ * num_frames - 0.5)), 0, num_frames - 1);

    if (old_frame != new_frame) {
        update_tex_coords_();
        notify_renderer_need_redraw();
    }
}

void animated_texture::copy_from(const region& obj) {
    base::copy_from(obj);

    const animated_texture* tex_obj = down_cast<animated_texture>(&obj);
    if (!tex_obj)
        return;

    this->set_texture(tex_obj->get_texture_file());
    this->set_speed(tex_obj->get_speed());
    this->set_state(tex_obj->get_state());
    this->set_paused(tex_obj->is_paused());
}

float animated_texture::get_speed() const {
    return speed_;
}

float animated_texture::get_state() const {
    return state_;
}

float animated_texture::is_paused() const {
    return is_paused_;
}

const std::string& animated_texture::get_texture_file() const {
    return file_;
}

color animated_texture::get_vertex_color(std::size_t index) const {
    if (index >= 4) {
        gui::out << gui::error << "gui::" << type_.back() << ": "
                 << "Vertex index out of bound (" << index << ")." << std::endl;
        return color::white;
    }

    return quad_.v[index].col;
}

void animated_texture::set_speed(float speed) {
    speed_ = speed;
}

void animated_texture::set_state(float state) {
    if (state == state_)
        return;

    state_ = state;
    update_tex_coords_();
}

void animated_texture::set_paused(bool is_paused) {
    is_paused_ = is_paused;
}

void animated_texture::set_texture(const std::string& file_name) {
    file_ = parse_file_name(file_name);

    if (file_.empty())
        return;

    auto& renderer = get_manager().get_renderer();

    std::shared_ptr<gui::material> mat;
    if (utils::file_exists(file_))
        mat = renderer.create_atlas_material("GUI", file_, material::filter::none);

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
        gui::out << gui::error << "gui::" << type_.back() << ": "
                 << "Cannot load file \"" << file_ << "\" for \"" << name_
                 << "\". Using white animated_texture instead." << std::endl;
    }

    update_tex_coords_();
    notify_renderer_need_redraw();
}

void animated_texture::set_vertex_color(const color& c, std::size_t index) {
    if (index == std::numeric_limits<std::size_t>::max()) {
        for (std::size_t i = 0; i < 4; ++i)
            quad_.v[i].col = c;

        notify_renderer_need_redraw();
        return;
    }

    if (index >= 4) {
        gui::out << gui::error << "gui::" << type_.back() << ": "
                 << "Vertex index out of bound (" << index << ")." << std::endl;
        return;
    }

    quad_.v[index].col = c;

    notify_renderer_need_redraw();
}

void animated_texture::update_tex_coords_() {
    // TODO
}

void animated_texture::update_borders_() {
    base::update_borders_();

    quad_.v[0].pos = border_list_.top_left();
    quad_.v[1].pos = border_list_.top_right();
    quad_.v[2].pos = border_list_.bottom_right();
    quad_.v[3].pos = border_list_.bottom_left();
}

} // namespace lxgui::gui
