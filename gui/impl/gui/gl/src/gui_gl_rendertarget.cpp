#include "lxgui/impl/gui_gl_rendertarget.hpp"
#include "lxgui/impl/gui_gl_renderer.hpp"

#include <lxgui/gui_exception.hpp>

#if defined(WIN32)
    #define NOMINMAX
    #include <windows.h>
#endif

#if !defined(WASM)
    #include <GL/glew.h>
    #if defined(MACOSX)
        #include <OpenGL/gl.h>
    #else
        #include <GL/gl.h>
    #endif
#else
    #if defined(MACOSX)
        #include <OpenGLES/ES3/gl.h>
    #else
        #include <GLES3/gl3.h>
    #endif
#endif

namespace lxgui {
namespace gui {
namespace gl
{
render_target::render_target(uint uiWidth, uint uiHeight, material::filter mFilter)
{
    pTexture_ = std::make_shared<gl::material>(uiWidth, uiHeight, material::wrap::REPEAT, mFilter);

    glGenFramebuffers(1, &uiFBOHandle_);
    glBindFramebuffer(GL_FRAMEBUFFER, uiFBOHandle_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pTexture_->get_handle_(), 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        throw gui::exception("gui::gl::render_target", "Failed creating render target.");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

render_target::~render_target()
{
    if (uiFBOHandle_ != 0)
        glDeleteFramebuffers(1, &uiFBOHandle_);
}

void render_target::begin()
{
    float fWidth = pTexture_->get_canvas_width();
    float fHeight = pTexture_->get_canvas_height();

    mViewMatrix_ = matrix4f::view(vector2f(fWidth, fHeight));

    glBindFramebuffer(GL_FRAMEBUFFER, uiFBOHandle_);

    glViewport(0.0f, 0.0f, fWidth, fHeight);
}

void render_target::end()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render_target::clear(const color& mColor)
{
    glClearColor(mColor.r, mColor.g, mColor.b, mColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

quad2f render_target::get_rect() const
{
    return pTexture_->get_rect();
}

uint render_target::get_canvas_width() const
{
    return pTexture_->get_canvas_width();
}

uint render_target::get_canvas_height() const
{
    return pTexture_->get_canvas_height();
}

bool render_target::set_dimensions(uint uiWidth, uint uiHeight)
{
    if (pTexture_->set_dimensions(uiWidth, uiHeight))
    {
        glBindFramebuffer(GL_FRAMEBUFFER, uiFBOHandle_);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pTexture_->get_handle_(), 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            throw gui::exception("gui::gl::render_target", "Failed resizing render target.");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return true;
    }

    return false;
}

std::weak_ptr<gl::material> render_target::get_material()
{
    return pTexture_;
}

const matrix4f& render_target::get_view_matrix() const
{
    return mViewMatrix_;
}

void render_target::check_availability()
{
#if !defined(LXGUI_OPENGL3)
    if (!renderer::is_gl_extension_supported("GL_EXT_framebuffer_object"))
    {
        throw gui::exception("gui::gl::render_target", "OpenGL extension "
            "'GL_EXT_framebuffer_object' is not supported by your hardware.");
    }
#else
    // Always supported in OpenGL 3 / OpenGL ES 3
#endif
}
}
}
}
