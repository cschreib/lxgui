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

atlas_page::atlas_page(gui::renderer& m_renderer, material::filter m_filter) :
    gui::atlas_page(m_filter) {
    ui_size_ = m_renderer.get_texture_atlas_page_size();

    GLint i_previous_id;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &i_previous_id);

    glGenTextures(1, &ui_texture_handle_);

    GLsizei i_size = static_cast<GLsizei>(ui_size_);
    glBindTexture(GL_TEXTURE_2D, ui_texture_handle_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, i_size, i_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    switch (m_filter) {
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

    glBindTexture(GL_TEXTURE_2D, i_previous_id);

    // Clear texture data (makes WebGL happy)
    GLuint ui_fbo = 0;
    glGenFramebuffers(1, &ui_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ui_fbo);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ui_texture_handle_, 0);

    float l_clear_color[4] = {0, 0, 0, 0};
    glClearBufferfv(GL_COLOR, 0, l_clear_color);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &ui_fbo);
}

atlas_page::~atlas_page() {
    glDeleteTextures(1, &ui_texture_handle_);
}

std::shared_ptr<gui::material>
atlas_page::add_material_(const gui::material& m_mat, const bounds2f& m_location) {
    const gl::material& m_gl_mat = static_cast<const gl::material&>(m_mat);

    GLint i_previous_id;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &i_previous_id);

    GLuint ui_fbo = 0;
    glGenFramebuffers(1, &ui_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, ui_fbo);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gl_mat.get_handle(), 0);

    std::vector<ub32color> l_data(m_location.width() * m_location.height());
    glReadPixels(
        0, 0, m_location.width(), m_location.height(), GL_RGBA, GL_UNSIGNED_BYTE, l_data.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &ui_fbo);

    glBindTexture(GL_TEXTURE_2D, ui_texture_handle_);
    glTexSubImage2D(
        GL_TEXTURE_2D, 0, m_location.left, m_location.top, m_location.width(), m_location.height(),
        GL_RGBA, GL_UNSIGNED_BYTE, l_data.data());

    glBindTexture(GL_TEXTURE_2D, i_previous_id);

    return std::make_shared<gl::material>(
        ui_texture_handle_, vector2ui(ui_size_, ui_size_), m_location, m_filter_);
}

float atlas_page::get_width_() const {
    return ui_size_;
}

float atlas_page::get_height_() const {
    return ui_size_;
}

atlas::atlas(renderer& m_renderer, material::filter m_filter) : gui::atlas(m_renderer, m_filter) {}

std::unique_ptr<gui::atlas_page> atlas::create_page_() {
    return std::make_unique<gl::atlas_page>(m_renderer_, m_filter_);
}

} // namespace lxgui::gui::gl
