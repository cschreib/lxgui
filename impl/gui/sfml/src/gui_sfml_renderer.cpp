#include "lxgui/impl/gui_sfml_renderer.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/impl/gui_sfml_atlas.hpp"
#include "lxgui/impl/gui_sfml_font.hpp"
#include "lxgui/impl/gui_sfml_material.hpp"
#include "lxgui/impl/gui_sfml_render_target.hpp"
#include "lxgui/impl/gui_sfml_vertex_cache.hpp"
#include "lxgui/utils_string.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>

namespace lxgui::gui::sfml {

renderer::renderer(sf::RenderWindow& window) :
    window_(window), window_dimensions_(window.getSize().x, window.getSize().y) {}

std::string renderer::get_name() const {
    return "SFML";
}

void renderer::begin_(std::shared_ptr<gui::render_target> target) {
    if (current_target_ || current_sfml_target_)
        throw gui::exception("gui::sfml::renderer", "Missing call to end()");

    if (target) {
        current_target_ = std::static_pointer_cast<sfml::render_target>(target);
        current_target_->begin();
        current_sfml_target_ = current_target_->get_render_texture();
    } else {
        sf::FloatRect visible_area(
            sf::Vector2f(0, 0), sf::Vector2f(window_dimensions_.x, window_dimensions_.y));
        window_.setView(sf::View(visible_area));
        current_sfml_target_ = &window_;
    }
}

void renderer::end_() {
    if (current_target_)
        current_target_->end();

    current_target_      = nullptr;
    current_sfml_target_ = nullptr;
}

void renderer::set_view_(const matrix4f& view_matrix) {
    static const float rad_to_deg = 180.0f / std::acos(-1.0f);

    float scale_x =
        std::sqrt(view_matrix(0, 0) * view_matrix(0, 0) + view_matrix(1, 0) * view_matrix(1, 0));
    float scale_y =
        std::sqrt(view_matrix(0, 1) * view_matrix(0, 1) + view_matrix(1, 1) * view_matrix(1, 1));
    float angle = std::atan2(view_matrix(0, 1) / scale_y, view_matrix(0, 0) / scale_x) * rad_to_deg;

    sf::View view;
    view.setCenter(sf::Vector2f(-view_matrix(3, 0) / scale_x, -view_matrix(3, 1) / scale_y));
    view.rotate(sf::radians(angle));
    view.setSize(sf::Vector2f(2.0f / scale_x, 2.0 / scale_y));

    current_sfml_target_->setView(view);
}

matrix4f renderer::get_view() const {
    matrix4f current_view_matrix =
        matrix4f(current_sfml_target_->getView().getTransform().getMatrix());

    if (!current_target_) {
        // Rendering to main screen, flip Y
        for (std::size_t i = 0; i < 4; ++i)
            current_view_matrix(i, 1) *= -1.0f;
    }

    return current_view_matrix;
}

void renderer::render_quads_(
    const gui::material* mat, const std::vector<std::array<vertex, 4>>& quad_list) {
    static const std::array<std::size_t, 6> ids          = {{0, 1, 2, 2, 3, 0}};
    static const std::size_t                num_vertices = ids.size();

    const sfml::material* sf_mat = static_cast<const sfml::material*>(mat);

#if !defined(SFML_HAS_NORMALISED_COORDINATES)
    vector2f tex_dims(1.0f, 1.0f);
    if (sf_mat)
        tex_dims = vector2f(sf_mat->get_canvas_dimensions());
#endif

    sf::VertexArray array(sf::PrimitiveType::Triangles, ids.size() * quad_list.size());
    for (std::size_t k = 0; k < quad_list.size(); ++k) {
        const std::array<vertex, 4>& vertices = quad_list[k];
        for (std::size_t i = 0; i < num_vertices; ++i) {
            const std::size_t j = ids[i];

            sf::Vertex&   sf_vertex = array[k * num_vertices + i];
            const vertex& vertex    = vertices[j];
            const float   a         = vertex.col.a;

            sf_vertex.position.x = vertex.pos.x;
            sf_vertex.position.y = vertex.pos.y;
#if defined(SFML_HAS_NORMALISED_COORDINATES)
            sf_vertex.texCoords.x = vertex.uvs.x;
            sf_vertex.texCoords.y = vertex.uvs.y;
#else
            sf_vertex.texCoords.x = vertex.uvs.x * tex_dims.x;
            sf_vertex.texCoords.y = vertex.uvs.y * tex_dims.y;
#endif
            // Premultipled alpha
            sf_vertex.color.r = vertex.col.r * a * 255;
            sf_vertex.color.g = vertex.col.g * a * 255;
            sf_vertex.color.b = vertex.col.b * a * 255;
            sf_vertex.color.a = a * 255;
        }
    }

    sf::RenderStates state;
    // Premultiplied alpha
    state.blendMode = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha);
#if defined(SFML_HAS_NORMALISED_COORDINATES)
    // UV coordinates and not pixel coordinates
    state.coordinateType = sf::CoordinateType::Normalized;
#endif
    // Texture
    if (sf_mat)
        state.texture = sf_mat->get_texture();

    current_sfml_target_->draw(array, state);
}

sf::Transform to_sfml(const matrix4f& matrix) {
    return sf::Transform(
        matrix(0, 0), matrix(1, 0), matrix(3, 0), matrix(0, 1), matrix(1, 1), matrix(3, 1), 0.0,
        0.0, 1.0);
}

void renderer::render_cache_(
    const gui::material*     mat [[maybe_unused]],
    const gui::vertex_cache& cache [[maybe_unused]],
    const matrix4f&          model_transform [[maybe_unused]]) {

#if defined(SFML_HAS_NORMALISED_COORDINATES)
    const sfml::material*     sf_mat   = static_cast<const sfml::material*>(mat);
    const sfml::vertex_cache& sf_cache = static_cast<const sfml::vertex_cache&>(cache);

    sf::RenderStates state;
    // Premultiplied alpha
    state.blendMode = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha);
    // UV coordinates and not pixel coordinates
    state.coordinateType = sf::CoordinateType::Normalized;
    // Texture
    if (sf_mat)
        state.texture = sf_mat->get_texture();
    // Transform
    state.transform = to_sfml(model_transform);

    // NB: Don't use draw(sf_cache.get_impl(), state); the SFML vertex cache never shrinks.
    current_sfml_target_->draw(sf_cache.get_impl(), 0, cache.get_vertex_count(), state);
#else
    throw gui::exception("gui::sfml::renderer", "SFML does not support vertex caches.");
#endif
}

std::shared_ptr<gui::material>
renderer::create_material_(const std::string& file_name, material::filter filt) {
    return std::make_shared<sfml::material>(file_name, material::wrap::repeat, filt);
}

std::shared_ptr<gui::atlas> renderer::create_atlas_(material::filter filt) {
    return std::make_shared<sfml::atlas>(*this, filt);
}

std::size_t renderer::get_texture_max_size() const {
    return sf::Texture::getMaximumSize();
}

bool renderer::is_texture_atlas_supported() const {
    return true;
}

bool renderer::is_texture_vertex_color_supported() const {
    return true;
}

std::shared_ptr<gui::material> renderer::create_material(
    const vector2ui& dimensions, const color32* pixel_data, material::filter filt) {
    std::shared_ptr<sfml::material> tex =
        std::make_shared<sfml::material>(dimensions, false, material::wrap::repeat, filt);

    tex->update_texture(pixel_data);

    return std::move(tex);
}

std::shared_ptr<gui::material>
renderer::create_material(std::shared_ptr<gui::render_target> target, const bounds2f& location) {
    auto tex = std::static_pointer_cast<sfml::render_target>(target)->get_material().lock();
    if (location == target->get_rect()) {
        return std::move(tex);
    } else {
        return std::make_shared<sfml::material>(
            tex->get_render_texture()->getTexture(), location, tex->get_filter());
    }
}

std::shared_ptr<gui::render_target>
renderer::create_render_target(const vector2ui& dimensions, material::filter filt) {
    return std::make_shared<sfml::render_target>(dimensions, filt);
}

std::shared_ptr<gui::font> renderer::create_font_(
    const std::string&                   font_file,
    std::size_t                          size,
    std::size_t                          outline,
    const std::vector<code_point_range>& code_points,
    char32_t                             default_code_point) {
    return std::make_shared<sfml::font>(font_file, size, outline, code_points, default_code_point);
}

bool renderer::is_vertex_cache_supported() const {
#if defined(SFML_HAS_NORMALISED_COORDINATES)
    return sf::VertexBuffer::isAvailable();
#else
    return false;
#endif
}

std::shared_ptr<gui::vertex_cache> renderer::create_vertex_cache(gui::vertex_cache::type type
                                                                 [[maybe_unused]]) {
#if defined(SFML_HAS_NORMALISED_COORDINATES)
    return std::make_shared<sfml::vertex_cache>(type);
#else
    throw gui::exception("gui::sfml::renderer", "SFML does not support vertex caches.");
#endif
}

void renderer::notify_window_resized(const vector2ui& new_dimensions) {
    window_dimensions_ = new_dimensions;
}

} // namespace lxgui::gui::sfml
