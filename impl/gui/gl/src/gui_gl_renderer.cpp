#include "lxgui/impl/gui_gl_renderer.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/impl/gui_gl_atlas.hpp"
#include "lxgui/impl/gui_gl_font.hpp"
#include "lxgui/impl/gui_gl_material.hpp"
#include "lxgui/impl/gui_gl_rendertarget.hpp"
#include "lxgui/impl/gui_gl_vertexcache.hpp"
#include "lxgui/utils_string.hpp"

#if defined(LXGUI_PLATFORM_WINDOWS)
#    define NOMINMAX
#    include <windows.h>
#endif

#if !defined(LXGUI_COMPILER_EMSCRIPTEN)
#    include <GL/glew.h>
#    if defined(LXGUI_PLATFORM_OSX)
#        include <OpenGL/gl.h>
#    else
#        include <GL/gl.h>
#    endif
#else
#    if defined(LXGUI_PLATFORM_OSX)
#        include <OpenGLES/ES3/gl.h>
#    else
#        include <GLES3/gl3.h>
#    endif
#endif

namespace lxgui::gui::gl {

#if defined(LXGUI_OPENGL3)
thread_local std::weak_ptr<renderer::shader_cache> renderer::p_static_shader_cache;
#endif

renderer::renderer(const vector2ui& m_window_dimensions, bool b_init_glew [[maybe_unused]]) :
    m_window_dimensions_(m_window_dimensions) {
#if !defined(LXGUI_COMPILER_EMSCRIPTEN)
    if (b_init_glew)
        glewInit();
#endif

    render_target::check_availability();
    material::check_availability();

#if defined(LXGUI_OPENGL3)
    compile_programs_();
    setup_buffers_();
#endif
}

std::string renderer::get_name() const {
    std::string s_full_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
#if defined(LXGUI_OPENGL3)
#    if defined(LXGUI_COMPILER_EMSCRIPTEN)
    return "WebGL (" + sFullVersion + ")";
#    else
    return "OpenGL (" + s_full_version + ")";
#    endif
#else
    return "OpenGL fixed pipeline (" + sFullVersion + ")";
#endif
}

#if defined(LXGUI_OPENGL3)
renderer::shader_cache::~shader_cache() {
    if (ui_program != 0)
        glDeleteProgram(ui_program);
}
#endif

void renderer::begin_(std::shared_ptr<gui::render_target> p_target) {
    matrix4f m_current_view_matrix;

    if (p_target) {
        p_current_target_ = std::static_pointer_cast<gl::render_target>(p_target);
        p_current_target_->begin();

        m_current_view_matrix = p_current_target_->get_view_matrix();
    } else {
        glViewport(0.0f, 0.0f, m_window_dimensions_.x, m_window_dimensions_.y);

        m_current_view_matrix = matrix4f::view(vector2f(m_window_dimensions_));
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Premultipled alpha
    glDisable(GL_CULL_FACE);

#if defined(LXGUI_OPENGL3)
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(p_shader_cache_->ui_program);
    ui_previous_texture_ = static_cast<std::uint32_t>(-1);
#else
    glDisable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);
#endif

    set_view_(m_current_view_matrix);
}

void renderer::end_() {
    if (p_current_target_) {
        p_current_target_->end();
        p_current_target_ = nullptr;
    }
}

void renderer::set_view_(const matrix4f& m_view_matrix) {
    m_current_view_matrix_ = m_view_matrix;

    matrix4f m_corrected_view = m_view_matrix;
    if (!p_current_target_) {
        // Rendering to main screen, flip Y
        for (std::size_t i = 0; i < 4; ++i)
            m_corrected_view(i, 1) *= -1.0f;
    }

#if defined(LXGUI_OPENGL3)
    glUniformMatrix4fv(p_shader_cache_->proj_location, 1, GL_FALSE, m_corrected_view.data);
#else
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(mCorrectedView.data);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#endif
}

matrix4f renderer::get_view() const {
    return m_current_view_matrix_;
}

void renderer::render_quads_(
    const gui::material* p_material, const std::vector<std::array<vertex, 4>>& l_quad_list) {

#if !defined(LXGUI_OPENGL3)
    static constexpr std::array<std::size_t, 6> lIDs = {{0, 1, 2, 2, 3, 0}};
    glColor4ub(255, 255, 255, 255);

    const gl::material* pMat = static_cast<const gl::material*>(pMaterial);
    if (pMat) {
        pMat->bind();

        glEnable(GL_TEXTURE_2D);
        glBegin(GL_TRIANGLES);
        for (const auto& v : lQuadList) {
            for (std::size_t i = 0; i < 6; ++i) {
                std::size_t j = lIDs[i];
                float       a = v[j].col.a;
                glColor4f(v[j].col.r * a, v[j].col.g * a, v[j].col.b * a, a); // Premultipled alpha
                glTexCoord2f(v[j].uvs.x, v[j].uvs.y);
                glVertex2f(v[j].pos.x, v[j].pos.y);
            }
        }
        glEnd();
    } else {
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_TRIANGLES);
        for (const auto& v : lQuadList) {
            for (std::size_t i = 0; i < 6; ++i) {
                std::size_t j = lIDs[i];
                float       a = v[j].col.a;
                glColor4f(v[j].col.r * a, v[j].col.g * a, v[j].col.b * a, a); // Premultipled alpha
                glVertex2f(v[j].pos.x, v[j].pos.y);
            }
        }
        glEnd();
    }
#else
    // Note: we rotate through a fairly large number of vertex caches
    // rather than constantly reusing the same cache. This is because
    // update_data() calls glBufferSubData(), which will wait for the
    // previous draw call using this cache to finish before updating.
    // If we rotate, it is more likely that the draw call is done, and
    // that we don't have to wait.
    const auto& p_cache   = p_array_cache_[ui_array_cycle_cache_];
    ui_array_cycle_cache_ = (ui_array_cycle_cache_ + 1) % cache_cycle_size;

    // Update vertex data
    p_cache->update(l_quad_list[0].data(), l_quad_list.size() * 4);

    // Render
    render_cache_(p_material, *p_cache, matrix4f::identity);
#endif
}

void renderer::render_cache_(
    const gui::material*     p_material,
    const gui::vertex_cache& m_cache,
    const matrix4f&          m_model_transform) {
#if !defined(LXGUI_OPENGL3)
    throw gui::exception("gl::renderer", "Legacy OpenGL does not support vertex caches.");
#else
    const gl::material*     p_mat      = static_cast<const gl::material*>(p_material);
    const gl::vertex_cache& m_gl_cache = static_cast<const gl::vertex_cache&>(m_cache);

    // Setup uniforms
    int type = 0;
    if (p_mat) {
        type = 0;
        if (ui_previous_texture_ != p_mat->get_handle()) {
            p_mat->bind();
            ui_previous_texture_ = p_mat->get_handle();
        }
    } else {
        type = 1;
    }

    glUniform1i(p_shader_cache_->type_location, type);
    glUniformMatrix4fv(p_shader_cache_->model_location, 1, GL_FALSE, m_model_transform.data);

    // Render
    m_gl_cache.render();
#endif
}

std::shared_ptr<gui::material>
renderer::create_material_(const std::string& s_file_name, material::filter m_filter) {
    if (!utils::ends_with(s_file_name, ".png"))
        throw gui::exception(
            "gui::gl::renderer", "Unsupported texture format '" + s_file_name + "'.");

    return create_material_png_(s_file_name, m_filter);
}

std::shared_ptr<gui::atlas> renderer::create_atlas_(material::filter m_filter) {
    return std::make_shared<gl::atlas>(*this, m_filter);
}

std::size_t renderer::get_texture_max_size() const {
    return material::get_max_size();
}

std::shared_ptr<gui::material> renderer::create_material(
    const vector2ui& m_dimensions, const ub32color* p_pixel_data, material::filter m_filter) {
    std::shared_ptr<gl::material> p_tex =
        std::make_shared<gl::material>(m_dimensions, material::wrap::repeat, m_filter);

    p_tex->update_texture(p_pixel_data);

    return std::move(p_tex);
}

std::shared_ptr<gui::material> renderer::create_material(
    std::shared_ptr<gui::render_target> p_render_target, const bounds2f& m_location) {
    auto p_tex =
        std::static_pointer_cast<gl::render_target>(p_render_target)->get_material().lock();
    if (m_location == p_render_target->get_rect()) {
        return std::move(p_tex);
    } else {
        return std::make_shared<gl::material>(
            p_tex->get_handle(), p_tex->get_canvas_dimensions(), m_location, p_tex->get_filter());
    }
}

std::shared_ptr<gui::render_target>
renderer::create_render_target(const vector2ui& m_dimensions, material::filter m_filter) {
    return std::make_shared<gl::render_target>(m_dimensions, m_filter);
}

std::shared_ptr<gui::font> renderer::create_font_(
    const std::string&                   s_font_file,
    std::size_t                          ui_size,
    std::size_t                          ui_outline,
    const std::vector<code_point_range>& l_code_points,
    char32_t                             ui_default_code_point) {
    return std::make_shared<gl::font>(
        s_font_file, ui_size, ui_outline, l_code_points, ui_default_code_point);
}

bool renderer::is_texture_atlas_supported() const {
    return true;
}

bool renderer::is_texture_vertex_color_supported() const {
    return true;
}

bool renderer::is_vertex_cache_supported() const {
#if !defined(LXGUI_OPENGL3)
    return false;
#else
    return true;
#endif
}

std::shared_ptr<gui::vertex_cache> renderer::create_vertex_cache(gui::vertex_cache::type m_type) {
#if !defined(LXGUI_OPENGL3)
    throw gui::exception("gl::renderer", "Legacy OpenGL does not support vertex caches.");
#else
    return std::make_shared<gl::vertex_cache>(m_type);
#endif
}

void renderer::notify_window_resized(const vector2ui& m_new_dimensions) {
    m_window_dimensions_ = m_new_dimensions;
}

#if !defined(LXGUI_OPENGL3)
bool renderer::is_gl_extension_supported(const std::string& sExtension) {
    // Extension names should not have spaces
    if (sExtension.find(' ') != std::string::npos || sExtension.empty())
        return false;

    GLint uiNumExtension = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &uiNumExtension);

    for (GLuint uiIndex = 0; uiIndex < static_cast<GLUint>(uiNumExtension); ++uiIndex) {
        if (sExtension == reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, uiIndex)))
            return true;
    }

    return false;
}
#endif

#if defined(LXGUI_OPENGL3)
GLuint create_shader(GLenum m_type, const char* s_shader_source) {
    // Create the shader
    GLuint ui_shader = glCreateShader(m_type);
    if (ui_shader == 0) {
        throw gui::exception(
            "gl::renderer", "Could not create " +
                                std::string(m_type == GL_VERTEX_SHADER ? "vertex" : "fragment") +
                                " shader.");
    }

    glShaderSource(ui_shader, 1, &s_shader_source, nullptr);
    glCompileShader(ui_shader);

    // Check sucess
    GLint compiled = 0;
    glGetShaderiv(ui_shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == 0) {
        GLint info_length = 0;
        glGetProgramiv(ui_shader, GL_INFO_LOG_LENGTH, &info_length);

        std::vector<char> s_error_message(std::max(1, info_length), '\0');
        if (info_length > 1) {
            glGetProgramInfoLog(ui_shader, info_length, NULL, s_error_message.data());
        }

        glDeleteShader(ui_shader);
        throw gui::exception(
            "gl::renderer", "Could not compile shader: " + std::string(s_error_message.data()));
    }

    return ui_shader;
}

GLuint create_program(const char* s_vertex_shader_source, const char* s_fragment_shader_source) {
    GLuint ui_vertex_shader   = 0;
    GLuint ui_fragment_shader = 0;
    GLuint ui_program_object  = 0;

    try {
        // Create shaders
        ui_vertex_shader   = create_shader(GL_VERTEX_SHADER, s_vertex_shader_source);
        ui_fragment_shader = create_shader(GL_FRAGMENT_SHADER, s_fragment_shader_source);

        // Create program
        ui_program_object = glCreateProgram();
        if (ui_program_object == 0) {
            throw gui::exception("gl::renderer", "Could not create shader program.");
        }

        glAttachShader(ui_program_object, ui_vertex_shader);
        glAttachShader(ui_program_object, ui_fragment_shader);
        glLinkProgram(ui_program_object);

        // Check success
        GLint linked = 0;
        glGetProgramiv(ui_program_object, GL_LINK_STATUS, &linked);
        if (linked == 0) {
            GLint info_length = 0;
            glGetProgramiv(ui_program_object, GL_INFO_LOG_LENGTH, &info_length);

            std::vector<char> s_error_message(std::max(1, info_length), '\0');
            if (info_length > 1) {
                glGetProgramInfoLog(ui_program_object, info_length, NULL, s_error_message.data());
            }

            throw gui::exception(
                "gl::renderer",
                "Could not link shader program: " + std::string(s_error_message.data()));
        }
    } catch (...) {
        if (ui_vertex_shader != 0)
            glDeleteShader(ui_vertex_shader);
        if (ui_fragment_shader != 0)
            glDeleteShader(ui_fragment_shader);
        if (ui_program_object != 0)
            glDeleteProgram(ui_program_object);
        throw;
    }

    glDeleteShader(ui_vertex_shader);
    glDeleteShader(ui_fragment_shader);

    return ui_program_object;
}

void renderer::compile_programs_() {
    // Shaders are compiled once, and reused by other renderers
    thread_local bool b_shader_cached = false;

    if (!b_shader_cached) {
        char s_vertex_shader[] = "#version 300 es                                           \n"
                                 "layout(location = 0) in vec2 a_position;                  \n"
                                 "layout(location = 1) in vec4 a_color;                     \n"
                                 "layout(location = 2) in vec2 a_texCoord;                  \n"
                                 "uniform mat4 m_proj;                                      \n"
                                 "uniform mat4 m_model;                                     \n"
                                 "out vec4 v_color;                                         \n"
                                 "out vec2 v_texCoord;                                      \n"
                                 "void main()                                               \n"
                                 "{                                                         \n"
                                 "    gl_Position = m_proj*m_model*vec4(a_position.xy,0,1); \n"
                                 "    v_color = a_color;                                    \n"
                                 "    v_color.rgb *= v_color.a;                             \n"
                                 "    v_texCoord = a_texCoord;                              \n"
                                 "}                                                         \n";

        char s_fragment_shader[] = "#version 300 es                                           \n"
                                   "precision mediump float;                                  \n"
                                   "in vec4 v_color;                                          \n"
                                   "in vec2 v_texCoord;                                       \n"
                                   "layout(location = 0) out vec4 o_color;                    \n"
                                   "uniform mediump int i_type;                               \n"
                                   "uniform sampler2D s_texture;                              \n"
                                   "void main()                                               \n"
                                   "{                                                         \n"
                                   "    if (i_type == 0)                                      \n"
                                   "        o_color = texture(s_texture, v_texCoord)*v_color; \n"
                                   "    else                                                  \n"
                                   "        o_color = v_color;                                \n"
                                   "}                                                         \n";

        p_shader_cache_ = std::make_shared<shader_cache>();

        try {
            p_shader_cache_->ui_program = create_program(s_vertex_shader, s_fragment_shader);
        } catch (...) {
            p_shader_cache_ = nullptr;
            throw;
        }

        p_shader_cache_->sampler_location =
            glGetUniformLocation(p_shader_cache_->ui_program, "s_texture");
        p_shader_cache_->proj_location =
            glGetUniformLocation(p_shader_cache_->ui_program, "m_proj");
        p_shader_cache_->model_location =
            glGetUniformLocation(p_shader_cache_->ui_program, "m_model");
        p_shader_cache_->type_location =
            glGetUniformLocation(p_shader_cache_->ui_program, "i_type");

        p_static_shader_cache = p_shader_cache_;
        b_shader_cached       = true;
    } else {
        p_shader_cache_ = p_static_shader_cache.lock();
    }
}

void renderer::setup_buffers_() {
    static constexpr std::array<std::uint32_t, 6> l_quad_i_ds = {{0, 1, 2, 2, 3, 0}};

    constexpr std::uint32_t    ui_num_array_indices = 768u;
    std::vector<std::uint32_t> l_repeated_ids(ui_num_array_indices);
    for (std::uint32_t i = 0; i < ui_num_array_indices; ++i) {
        l_repeated_ids[i] = (i / 6) * 4 + l_quad_i_ds[i % 6];
    }

    for (std::size_t i = 0; i < cache_cycle_size; ++i) {
        p_quad_cache_[i] = std::static_pointer_cast<gl::vertex_cache>(
            create_vertex_cache(vertex_cache::type::quads));
        p_quad_cache_[i]->update_indices(l_quad_i_ds.data(), l_quad_i_ds.size());

        p_array_cache_[i] = std::static_pointer_cast<gl::vertex_cache>(
            create_vertex_cache(vertex_cache::type::quads));
        p_array_cache_[i]->update_indices(l_repeated_ids.data(), l_repeated_ids.size());
    }
}
#endif

} // namespace lxgui::gui::gl
