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

render_target::render_target(const vector2ui& mDimensions, material::filter mFilter) {
    pTexture_ = std::make_shared<gl::material>(mDimensions, material::wrap::REPEAT, mFilter);

    glGenFramebuffers(1, &uiFBOHandle_);
    glBindFramebuffer(GL_FRAMEBUFFER, uiFBOHandle_);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pTexture_->get_handle_(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        throw gui::exception("gui::gl::render_target", "Failed creating render target.");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

render_target::~render_target() {
    if (uiFBOHandle_ != 0)
        glDeleteFramebuffers(1, &uiFBOHandle_);
}

void render_target::begin() {
    vector2f mView = vector2f(pTexture_->get_canvas_dimensions());
    mViewMatrix_   = matrix4f::view(mView);

    glBindFramebuffer(GL_FRAMEBUFFER, uiFBOHandle_);

    glViewport(0.0f, 0.0f, mView.x, mView.y);
}

void render_target::end() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render_target::clear(const color& mColor) {
    glClearColor(mColor.r, mColor.g, mColor.b, mColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

bounds2f render_target::get_rect() const {
    return pTexture_->get_rect();
}

vector2ui render_target::get_canvas_dimensions() const {
    return pTexture_->get_canvas_dimensions();
}

bool render_target::set_dimensions(const vector2ui& mDimensions) {
    if (pTexture_->set_dimensions(mDimensions)) {
        glBindFramebuffer(GL_FRAMEBUFFER, uiFBOHandle_);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pTexture_->get_handle_(), 0);

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
    return pTexture_;
}

const matrix4f& render_target::get_view_matrix() const {
    return mViewMatrix_;
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
