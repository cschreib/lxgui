#include "lxgui/impl/gui_gl_rendertarget.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/impl/gui_gl_renderer.hpp"

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

render_target::render_target(const vector2ui& m_dimensions, material::filter m_filter) {
    p_texture_ = std::make_shared<gl::material>(m_dimensions, material::wrap::repeat, m_filter);

    glGenFramebuffers(1, &ui_fbo_handle_);
    glBindFramebuffer(GL_FRAMEBUFFER, ui_fbo_handle_);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, p_texture_->get_handle(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        throw gui::exception("gui::gl::render_target", "Failed creating render target.");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

render_target::~render_target() {
    if (ui_fbo_handle_ != 0)
        glDeleteFramebuffers(1, &ui_fbo_handle_);
}

void render_target::begin() {
    vector2f m_view = vector2f(p_texture_->get_canvas_dimensions());
    m_view_matrix_   = matrix4f::view(m_view);

    glBindFramebuffer(GL_FRAMEBUFFER, ui_fbo_handle_);

    glViewport(0.0f, 0.0f, m_view.x, m_view.y);
}

void render_target::end() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render_target::clear(const color& m_color) {
    glClearColor(m_color.r, m_color.g, m_color.b, m_color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

bounds2f render_target::get_rect() const {
    return p_texture_->get_rect();
}

vector2ui render_target::get_canvas_dimensions() const {
    return p_texture_->get_canvas_dimensions();
}

bool render_target::set_dimensions(const vector2ui& m_dimensions) {
    if (p_texture_->set_dimensions(m_dimensions)) {
        glBindFramebuffer(GL_FRAMEBUFFER, ui_fbo_handle_);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, p_texture_->get_handle(), 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            throw gui::exception("gui::gl::render_target", "Failed resizing render target.");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return true;
    }

    return false;
}

std::weak_ptr<gl::material> render_target::get_material() {
    return p_texture_;
}

const matrix4f& render_target::get_view_matrix() const {
    return m_view_matrix_;
}

void render_target::check_availability() {
#if !defined(LXGUI_OPENGL3)
    if (!renderer::is_gl_extension_supported("GL_EXT_framebuffer_object")) {
        throw gui::exception(
            "gui::gl::render_target",
            "OpenGL extension "
            "'GL_EXT_framebuffer_object' is not supported by your hardware.");
    }
#else
    // Always supported in OpenGL 3 / OpenGL ES 3
#endif
}

} // namespace lxgui::gui::gl
