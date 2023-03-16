#include "lxgui/impl/gui_gl_renderer.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_quad.hpp"
#include "lxgui/impl/gui_gl_atlas.hpp"
#include "lxgui/impl/gui_gl_font.hpp"
#include "lxgui/impl/gui_gl_material.hpp"
#include "lxgui/impl/gui_gl_render_target.hpp"
#include "lxgui/impl/gui_gl_vertex_cache.hpp"
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
thread_local std::weak_ptr<renderer::shader_cache> renderer::static_shader_cache;
#endif

renderer::renderer(const vector2ui& window_dimensions, bool init_glew [[maybe_unused]]) :
    window_dimensions_(window_dimensions) {
#if !defined(LXGUI_COMPILER_EMSCRIPTEN)
    if (init_glew)
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
    std::string full_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
#if defined(LXGUI_OPENGL3)
#    if defined(LXGUI_COMPILER_EMSCRIPTEN)
    return "WebGL (" + full_version + ")";
#    else
    return "OpenGL (" + full_version + ")";
#    endif
#else
    return "OpenGL fixed pipeline (" + full_version + ")";
#endif
}

#if defined(LXGUI_OPENGL3)
renderer::shader_cache::~shader_cache() {
    if (program != 0)
        glDeleteProgram(program);
}
#endif

void renderer::begin_(std::shared_ptr<gui::render_target> target) {
    matrix4f current_view_matrix;

    if (target) {
        current_target_ = std::static_pointer_cast<gl::render_target>(target);
        current_target_->begin();

        current_view_matrix = current_target_->get_view_matrix();
    } else {
        glViewport(0.0f, 0.0f, window_dimensions_.x, window_dimensions_.y);

        current_view_matrix = matrix4f::view(vector2f(window_dimensions_));
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Premultipled alpha
    glDisable(GL_CULL_FACE);

#if defined(LXGUI_OPENGL3)
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(shader_cache_->program);
    previous_texture_ = static_cast<std::uint32_t>(-1);
#else
    glDisable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);
#endif

    set_view_(current_view_matrix);
}

void renderer::end_() {
    if (current_target_) {
        current_target_->end();
        current_target_ = nullptr;
    }
}

void renderer::set_view_(const matrix4f& view_matrix) {
    current_view_matrix_ = view_matrix;

    matrix4f corrected_view = view_matrix;
    if (!current_target_) {
        // Rendering to main screen, flip Y
        for (std::size_t i = 0; i < 4; ++i)
            corrected_view(i, 1) *= -1.0f;
    }

#if defined(LXGUI_OPENGL3)
    glUniformMatrix4fv(shader_cache_->proj_location, 1, GL_FALSE, corrected_view.data);
#else
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(corrected_view.data);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#endif
}

matrix4f renderer::get_view() const {
    return current_view_matrix_;
}

void renderer::render_quads_(
    const gui::material* mat, const std::vector<std::array<vertex, 4>>& quad_list) {

#if !defined(LXGUI_OPENGL3)
    static constexpr std::array<std::size_t, 6> ids = {{0, 1, 2, 2, 3, 0}};
    glColor4ub(255, 255, 255, 255);

    const gl::material* gl_mat = static_cast<const gl::material*>(mat);
    if (gl_mat) {
        gl_mat->bind();

        glEnable(GL_TEXTURE_2D);
        glBegin(GL_TRIANGLES);
        for (const auto& v : quad_list) {
            for (std::size_t i = 0; i < 6; ++i) {
                std::size_t j = ids[i];
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
        for (const auto& v : quad_list) {
            for (std::size_t i = 0; i < 6; ++i) {
                std::size_t j = ids[i];
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
    const auto& cache  = array_cache_[array_cycle_cache_];
    array_cycle_cache_ = (array_cycle_cache_ + 1) % cache_cycle_size;

    // Update vertex data
    cache->update(quad_list[0].data(), quad_list.size() * 4);

    // Render
    render_cache_(mat, *cache, matrix4f::identity);
#endif
}

void renderer::render_cache_(
    const gui::material*     mat [[maybe_unused]],
    const gui::vertex_cache& cache [[maybe_unused]],
    const matrix4f&          model_transform [[maybe_unused]]) {
#if !defined(LXGUI_OPENGL3)
    throw gui::exception("gl::renderer", "Legacy OpenGL does not support vertex caches.");
#else
    const gl::material*     gl_mat   = static_cast<const gl::material*>(mat);
    const gl::vertex_cache& gl_cache = static_cast<const gl::vertex_cache&>(cache);

    // Setup uniforms
    int type = 0;
    if (gl_mat) {
        type = 0;
        if (previous_texture_ != gl_mat->get_handle()) {
            gl_mat->bind();
            previous_texture_ = gl_mat->get_handle();
        }
    } else {
        type = 1;
    }

    glUniform1i(shader_cache_->type_location, type);
    glUniformMatrix4fv(shader_cache_->model_location, 1, GL_FALSE, model_transform.data);

    // Render
    gl_cache.render();
#endif
}

std::shared_ptr<gui::material>
renderer::create_material_(const std::string& file_name, material::filter filt) {
    if (!utils::ends_with(file_name, ".png"))
        throw gui::exception(
            "gui::gl::renderer", "Unsupported texture format '" + file_name + "'.");

    return create_material_png_(file_name, filt);
}

std::shared_ptr<gui::atlas> renderer::create_atlas_(material::filter filt) {
    return std::make_shared<gl::atlas>(*this, filt);
}

std::size_t renderer::get_texture_max_size() const {
    return material::get_max_size();
}

std::shared_ptr<gui::material> renderer::create_material(
    const vector2ui& dimensions, const color32* pixel_data, material::filter filt) {
    std::shared_ptr<gl::material> tex =
        std::make_shared<gl::material>(dimensions, material::wrap::repeat, filt);

    tex->update_texture(pixel_data);

    return std::move(tex);
}

std::shared_ptr<gui::material>
renderer::create_material(std::shared_ptr<gui::render_target> target, const bounds2f& location) {
    auto tex = std::static_pointer_cast<gl::render_target>(target)->get_material().lock();
    if (location == target->get_rect()) {
        return std::move(tex);
    } else {
        return std::make_shared<gl::material>(
            tex->get_handle(), tex->get_canvas_dimensions(), location, tex->get_filter());
    }
}

std::shared_ptr<gui::render_target>
renderer::create_render_target(const vector2ui& dimensions, material::filter filt) {
    return std::make_shared<gl::render_target>(dimensions, filt);
}

std::shared_ptr<gui::font> renderer::create_font_(
    const std::string&                   font_file,
    std::size_t                          size,
    std::size_t                          outline,
    const std::vector<code_point_range>& code_points,
    char32_t                             default_code_point) {
    return std::make_shared<gl::font>(font_file, size, outline, code_points, default_code_point);
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

std::shared_ptr<gui::vertex_cache> renderer::create_vertex_cache(gui::vertex_cache::type type
                                                                 [[maybe_unused]]) {
#if !defined(LXGUI_OPENGL3)
    throw gui::exception("gl::renderer", "Legacy OpenGL does not support vertex caches.");
#else
    return std::make_shared<gl::vertex_cache>(type);
#endif
}

void renderer::notify_window_resized(const vector2ui& new_dimensions) {
    window_dimensions_ = new_dimensions;
}

#if !defined(LXGUI_OPENGL3)
bool renderer::is_gl_extension_supported(const std::string& extension) {
    // Extension names should not have spaces
    if (extension.find(' ') != std::string::npos || extension.empty())
        return false;

    GLint num_extensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);

    for (GLuint index = 0; index < static_cast<GLuint>(num_extensions); ++index) {
        if (extension == reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, index)))
            return true;
    }

    return false;
}
#endif

#if defined(LXGUI_OPENGL3)
GLuint create_shader(GLenum type, const char* shader_source) {
    // Create the shader
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        throw gui::exception(
            "gl::renderer", "Could not create " +
                                std::string(type == GL_VERTEX_SHADER ? "vertex" : "fragment") +
                                " shader.");
    }

    glShaderSource(shader, 1, &shader_source, nullptr);
    glCompileShader(shader);

    // Check sucess
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == 0) {
        GLint info_length = 0;
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &info_length);

        std::vector<char> error_message(std::max(1, info_length), '\0');
        if (info_length > 1) {
            glGetProgramInfoLog(shader, info_length, NULL, error_message.data());
        }

        glDeleteShader(shader);
        throw gui::exception(
            "gl::renderer", "Could not compile shader: " + std::string(error_message.data()));
    }

    return shader;
}

GLuint create_program(const char* vertex_shader_source, const char* fragment_shader_source) {
    GLuint vertex_shader   = 0;
    GLuint fragment_shader = 0;
    GLuint program_object  = 0;

    try {
        // Create shaders
        vertex_shader   = create_shader(GL_VERTEX_SHADER, vertex_shader_source);
        fragment_shader = create_shader(GL_FRAGMENT_SHADER, fragment_shader_source);

        // Create program
        program_object = glCreateProgram();
        if (program_object == 0) {
            throw gui::exception("gl::renderer", "Could not create shader program.");
        }

        glAttachShader(program_object, vertex_shader);
        glAttachShader(program_object, fragment_shader);
        glLinkProgram(program_object);

        // Check success
        GLint linked = 0;
        glGetProgramiv(program_object, GL_LINK_STATUS, &linked);
        if (linked == 0) {
            GLint info_length = 0;
            glGetProgramiv(program_object, GL_INFO_LOG_LENGTH, &info_length);

            std::vector<char> error_message(std::max(1, info_length), '\0');
            if (info_length > 1) {
                glGetProgramInfoLog(program_object, info_length, NULL, error_message.data());
            }

            throw gui::exception(
                "gl::renderer",
                "Could not link shader program: " + std::string(error_message.data()));
        }
    } catch (...) {
        if (vertex_shader != 0)
            glDeleteShader(vertex_shader);
        if (fragment_shader != 0)
            glDeleteShader(fragment_shader);
        if (program_object != 0)
            glDeleteProgram(program_object);
        throw;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program_object;
}

void renderer::compile_programs_() {
    // Shaders are compiled once, and reused by other renderers
    thread_local bool shader_cached = false;

    if (!shader_cached) {
        char vertex_shader[] = "#version 300 es                                           \n"
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

        char fragment_shader[] = "#version 300 es                                           \n"
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

        shader_cache_ = std::make_shared<shader_cache>();

        try {
            shader_cache_->program = create_program(vertex_shader, fragment_shader);
        } catch (...) {
            shader_cache_ = nullptr;
            throw;
        }

        shader_cache_->sampler_location = glGetUniformLocation(shader_cache_->program, "s_texture");
        shader_cache_->proj_location    = glGetUniformLocation(shader_cache_->program, "m_proj");
        shader_cache_->model_location   = glGetUniformLocation(shader_cache_->program, "m_model");
        shader_cache_->type_location    = glGetUniformLocation(shader_cache_->program, "i_type");

        static_shader_cache = shader_cache_;
        shader_cached       = true;
    } else {
        shader_cache_ = static_shader_cache.lock();
    }
}

void renderer::setup_buffers_() {
    static constexpr std::array<std::uint32_t, 6> quad_i_ds = {{0, 1, 2, 2, 3, 0}};

    constexpr std::uint32_t    num_array_indices = 768u;
    std::vector<std::uint32_t> repeated_ids(num_array_indices);
    for (std::uint32_t i = 0; i < num_array_indices; ++i) {
        repeated_ids[i] = (i / 6) * 4 + quad_i_ds[i % 6];
    }

    for (std::size_t i = 0; i < cache_cycle_size; ++i) {
        quad_cache_[i] = std::static_pointer_cast<gl::vertex_cache>(
            create_vertex_cache(vertex_cache::type::quads));
        quad_cache_[i]->update_indices(quad_i_ds.data(), quad_i_ds.size());

        array_cache_[i] = std::static_pointer_cast<gl::vertex_cache>(
            create_vertex_cache(vertex_cache::type::quads));
        array_cache_[i]->update_indices(repeated_ids.data(), repeated_ids.size());
    }
}
#endif

} // namespace lxgui::gui::gl
