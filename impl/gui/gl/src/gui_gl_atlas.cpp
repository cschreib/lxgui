#include "lxgui/impl/gui_gl_atlas.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_gl_material.hpp"
#include "lxgui/impl/gui_gl_renderer.hpp"
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

#if !defined(GL_CLAMP_TO_EDGE)
#    define GL_CLAMP_TO_EDGE 0x812F
#endif

#include <cmath>

namespace lxgui::gui::gl {

atlas_page::atlas_page(gui::renderer& rdr, material::filter filt) : gui::atlas_page(filt) {
    size_ = rdr.get_texture_atlas_page_size();

    GLint previous_id;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_id);

    glGenTextures(1, &texture_handle_);

    GLsizei size_int = static_cast<GLsizei>(size_);
    glBindTexture(GL_TEXTURE_2D, texture_handle_);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA8, size_int, size_int, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    switch (filt) {
    case material::filter::linear:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        break;
    case material::filter::none:
    default:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        break;
    }

    glBindTexture(GL_TEXTURE_2D, previous_id);

    // Clear texture data (makes WebGL happy)
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_handle_, 0);

    float clear_color[4] = {0, 0, 0, 0};
    glClearBufferfv(GL_COLOR, 0, clear_color);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
}

atlas_page::~atlas_page() {
    glDeleteTextures(1, &texture_handle_);
}

std::shared_ptr<gui::material>
atlas_page::add_material_(const gui::material& mat, const bounds2f& location) {
    const gl::material& gl_mat = static_cast<const gl::material&>(mat);

    GLint previous_id;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_id);

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl_mat.get_handle(), 0);

    std::vector<ub32color> pixel_data(location.width() * location.height());
    glReadPixels(
        0, 0, location.width(), location.height(), GL_RGBA, GL_UNSIGNED_BYTE, pixel_data.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);

    glBindTexture(GL_TEXTURE_2D, texture_handle_);
    glTexSubImage2D(
        GL_TEXTURE_2D, 0, location.left, location.top, location.width(), location.height(), GL_RGBA,
        GL_UNSIGNED_BYTE, pixel_data.data());

    glBindTexture(GL_TEXTURE_2D, previous_id);

    return std::make_shared<gl::material>(
        texture_handle_, vector2ui(size_, size_), location, filter_);
}

float atlas_page::get_width_() const {
    return size_;
}

float atlas_page::get_height_() const {
    return size_;
}

atlas::atlas(renderer& rdr, material::filter filt) : gui::atlas(rdr, filt) {}

std::unique_ptr<gui::atlas_page> atlas::create_page_() {
    return std::make_unique<gl::atlas_page>(renderer_, filter_);
}

} // namespace lxgui::gui::gl
