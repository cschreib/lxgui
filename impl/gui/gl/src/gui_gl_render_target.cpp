#include "lxgui/impl/gui_gl_render_target.hpp"

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

render_target::render_target(const vector2ui& dimensions, material::filter filt) {
    texture_ = std::make_shared<gl::material>(dimensions, material::wrap::repeat, filt);

    glGenFramebuffers(1, &fbo_handle_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle_);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_->get_handle(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        throw gui::exception("gui::gl::render_target", "Failed creating render target.");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

render_target::~render_target() {
    if (fbo_handle_ != 0)
        glDeleteFramebuffers(1, &fbo_handle_);
}

void render_target::begin() {
    vector2f view = vector2f(texture_->get_canvas_dimensions());
    view_matrix_  = matrix4f::view(view);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle_);

    glViewport(0.0f, 0.0f, view.x, view.y);
}

void render_target::end() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render_target::clear(const color& c) {
    glClearColor(c.r, c.g, c.b, c.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

bounds2f render_target::get_rect() const {
    return texture_->get_rect();
}

vector2ui render_target::get_canvas_dimensions() const {
    return texture_->get_canvas_dimensions();
}

bool render_target::set_dimensions(const vector2ui& dimensions) {
    if (texture_->set_dimensions(dimensions)) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle_);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_->get_handle(), 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            throw gui::exception("gui::gl::render_target", "Failed resizing render target.");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return true;
    }

    return false;
}

void render_target::save_to_file(std::string filename) const {
    const auto width  = texture_->get_rect().width();
    const auto height = texture_->get_rect().height();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_handle_);
    std::vector<color32> data(width * height);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    for (auto& c : data) {
        float a = c.a / 255.0f;
        // De-multiply alpha
        if (a > 0) {
            c.r /= a;
            c.g /= a;
            c.b /= a;
        }
    }

    save_rgba_to_png_(filename, data.data(), width, height);
}

std::weak_ptr<gl::material> render_target::get_material() {
    return texture_;
}

const matrix4f& render_target::get_view_matrix() const {
    return view_matrix_;
}

void render_target::check_availability() {
#if !defined(LXGUI_OPENGL3)
    if (!renderer::is_gl_extension_supported("GL_EXT_framebuffer_object")) {
        throw gui::exception(
            "gui::gl::render_target",
            "OpenGL extension 'GL_EXT_framebuffer_object' is not supported by your hardware.");
    }
#else
    // Always supported in OpenGL 3 / OpenGL ES 3
#endif
}

} // namespace lxgui::gui::gl
