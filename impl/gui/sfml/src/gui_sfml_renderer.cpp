#include "lxgui/impl/gui_sfml_renderer.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_matrix4.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/impl/gui_sfml_atlas.hpp"
#include "lxgui/impl/gui_sfml_font.hpp"
#include "lxgui/impl/gui_sfml_material.hpp"
#include "lxgui/impl/gui_sfml_rendertarget.hpp"
#include "lxgui/impl/gui_sfml_vertexcache.hpp"
#include "lxgui/utils_string.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>

namespace lxgui::gui::sfml {

renderer::renderer(sf::RenderWindow& m_window) :
    m_window_(m_window), m_window_dimensions_(m_window.getSize().x, m_window.getSize().y) {}

std::string renderer::get_name() const {
    return "SFML";
}

void renderer::begin_(std::shared_ptr<gui::render_target> p_target) {
    if (p_current_target_ || p_current_sfml_target_)
        throw gui::exception("gui::sfml::renderer", "Missing call to end()");

    if (p_target) {
        p_current_target_ = std::static_pointer_cast<sfml::render_target>(p_target);
        p_current_target_->begin();
        p_current_sfml_target_ = p_current_target_->get_render_texture();
    } else {
        sf::FloatRect m_visible_area(0, 0, m_window_dimensions_.x, m_window_dimensions_.y);
        m_window_.setView(sf::View(m_visible_area));
        p_current_sfml_target_ = &m_window_;
    }
}

void renderer::end_() {
    if (p_current_target_)
        p_current_target_->end();

    p_current_target_     = nullptr;
    p_current_sfml_target_ = nullptr;
}

void renderer::set_view_(const matrix4f& m_view_matrix) {
    static const float rad_to_deg = 180.0f / std::acos(-1.0f);

    float f_scale_x =
        std::sqrt(m_view_matrix(0, 0) * m_view_matrix(0, 0) + m_view_matrix(1, 0) * m_view_matrix(1, 0));
    float f_scale_y =
        std::sqrt(m_view_matrix(0, 1) * m_view_matrix(0, 1) + m_view_matrix(1, 1) * m_view_matrix(1, 1));
    float f_angle =
        std::atan2(m_view_matrix(0, 1) / f_scale_y, m_view_matrix(0, 0) / f_scale_x) * rad_to_deg;

    sf::View m_view;
    m_view.setCenter(sf::Vector2f(-m_view_matrix(3, 0) / f_scale_x, -m_view_matrix(3, 1) / f_scale_y));
    m_view.rotate(f_angle);
    m_view.setSize(sf::Vector2f(2.0f / f_scale_x, 2.0 / f_scale_y));

    p_current_sfml_target_->setView(m_view);
}

matrix4f renderer::get_view() const {
    matrix4f m_current_view_matrix =
        matrix4f(p_current_sfml_target_->getView().getTransform().getMatrix());
    if (!p_current_target_) {
        // Rendering to main screen, flip Y
        for (std::size_t i = 0; i < 4; ++i)
            m_current_view_matrix(i, 1) *= -1.0f;
    }

    return m_current_view_matrix;
}

void renderer::render_quads_(
    const gui::material* p_material, const std::vector<std::array<vertex, 4>>& l_quad_list) {
    static const std::array<std::size_t, 6> l_i_ds          = {{0, 1, 2, 2, 3, 0}};
    static const std::size_t                ui_num_vertices = l_i_ds.size();

    const sfml::material* p_mat = static_cast<const sfml::material*>(p_material);

    vector2f m_tex_dims(1.0f, 1.0f);
    if (p_mat)
        m_tex_dims = vector2f(p_mat->get_canvas_dimensions());

    sf::VertexArray m_array(sf::PrimitiveType::Triangles, l_i_ds.size() * l_quad_list.size());
    for (std::size_t k = 0; k < l_quad_list.size(); ++k) {
        const std::array<vertex, 4>& m_vertices = l_quad_list[k];
        for (std::size_t i = 0; i < ui_num_vertices; ++i) {
            const std::size_t j         = l_i_ds[i];
            sf::Vertex&       m_sf_vertex = m_array[k * ui_num_vertices + i];
            const vertex&     m_vertex   = m_vertices[j];
            const float       a         = m_vertex.col.a;

            m_sf_vertex.position.x  = m_vertex.pos.x;
            m_sf_vertex.position.y  = m_vertex.pos.y;
            m_sf_vertex.texCoords.x = m_vertex.uvs.x * m_tex_dims.x;
            m_sf_vertex.texCoords.y = m_vertex.uvs.y * m_tex_dims.y;
            m_sf_vertex.color.r     = m_vertex.col.r * a * 255; // Premultipled alpha
            m_sf_vertex.color.g     = m_vertex.col.g * a * 255; // Premultipled alpha
            m_sf_vertex.color.b     = m_vertex.col.b * a * 255; // Premultipled alpha
            m_sf_vertex.color.a     = a * 255;
        }
    }

    sf::RenderStates m_state;
    m_state.blendMode =
        sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha); // Premultiplied alpha
    if (p_mat)
        m_state.texture = p_mat->get_texture();
    p_current_sfml_target_->draw(m_array, m_state);
}

sf::Transform to_sfml(const matrix4f& m_matrix) {
    return sf::Transform(
        m_matrix(0, 0), m_matrix(1, 0), m_matrix(3, 0), m_matrix(0, 1), m_matrix(1, 1), m_matrix(3, 1),
        0.0, 0.0, 1.0);
}

void renderer::render_cache_(
    const gui::material*     p_material,
    const gui::vertex_cache& m_cache,
    const matrix4f&          m_model_transform) {
    throw gui::exception("gui::sfml::renderer", "SFML does not support vertex caches.");

#if 0
    const sfml::material* pMat = static_cast<const sfml::material*>(pMaterial);
    const sfml::vertex_cache& mSFCache = static_cast<const sfml::vertex_cache&>(mCache);

    // Note: the following will not work correctly, as vertex_cache has texture coordinates
    // normalised, but sf::RenderTarget::draw assumes coordinates in pixels.
    // https://github.com/SFML/SFML/issues/1773
    sf::RenderStates mState;
    mState.blendMode = sf::BlendMode(sf::BlendMode::One, sf::BlendMode::OneMinusSrcAlpha); // Premultiplied alpha
    if (pMat)
        mState.texture = pMat->get_texture();
    mState.transform = to_sfml(mModelTransform);
    pCurrentSFMLTarget_->draw(mSFCache.get_impl(), 0, mSFCache.get_num_vertex(), mState);
#endif
}

std::shared_ptr<gui::material>
renderer::create_material_(const std::string& s_file_name, material::filter m_filter) {
    return std::make_shared<sfml::material>(s_file_name, material::wrap::repeat, m_filter);
}

std::shared_ptr<gui::atlas> renderer::create_atlas_(material::filter m_filter) {
    return std::make_shared<sfml::atlas>(*this, m_filter);
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
    const vector2ui& m_dimensions, const ub32color* p_pixel_data, material::filter m_filter) {
    std::shared_ptr<sfml::material> p_tex =
        std::make_shared<sfml::material>(m_dimensions, false, material::wrap::repeat, m_filter);

    p_tex->update_texture(p_pixel_data);

    return std::move(p_tex);
}

std::shared_ptr<gui::material> renderer::create_material(
    std::shared_ptr<gui::render_target> p_render_target, const bounds2f& m_location) {
    auto p_tex = std::static_pointer_cast<sfml::render_target>(p_render_target)->get_material().lock();
    if (m_location == p_render_target->get_rect()) {
        return std::move(p_tex);
    } else {
        return std::make_shared<sfml::material>(
            p_tex->get_render_texture()->getTexture(), m_location, p_tex->get_filter());
    }
}

std::shared_ptr<gui::render_target>
renderer::create_render_target(const vector2ui& m_dimensions, material::filter m_filter) {
    return std::make_shared<sfml::render_target>(m_dimensions, m_filter);
}

std::shared_ptr<gui::font> renderer::create_font_(
    const std::string&                   s_font_file,
    std::size_t                          ui_size,
    std::size_t                          ui_outline,
    const std::vector<code_point_range>& l_code_points,
    char32_t                             ui_default_code_point) {
    return std::make_shared<sfml::font>(
        s_font_file, ui_size, ui_outline, l_code_points, ui_default_code_point);
}

bool renderer::is_vertex_cache_supported() const {
#if defined(SFML_HAS_NORMALISED_COORDINATES_VBO)
    // Requires https://github.com/SFML/SFML/pull/1807
    return sf::VertexBuffer::isAvailable();
#else
    return false;
#endif
}

std::shared_ptr<gui::vertex_cache> renderer::create_vertex_cache(gui::vertex_cache::type m_type) {
#if defined(SFML_HAS_NORMALISED_COORDINATES_VBO)
    // Requires https://github.com/SFML/SFML/pull/1807
    return std::make_shared<sfml::vertex_cache>(mType);
#else
    throw gui::exception("gui::sfml::renderer", "SFML does not support vertex caches.");
#endif
}

void renderer::notify_window_resized(const vector2ui& m_new_dimensions) {
    m_window_dimensions_ = m_new_dimensions;
}

} // namespace lxgui::gui::sfml
