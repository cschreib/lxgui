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
render_target::render_target(uint uiWidth, uint uiHeight)
{
    pTexture_ = std::make_shared<gl::material>(
        uiWidth, uiHeight, material::wrap::REPEAT, material::filter::NONE, true
    );

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
    if (bUpdateViewMatrix_)
        update_view_matrix_();

    glBindFramebuffer(GL_FRAMEBUFFER, uiFBOHandle_);

    glViewport(0.0f, 0.0f, pTexture_->get_real_width(), pTexture_->get_real_height());
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Premultiplied alpha
    glDisable(GL_CULL_FACE);

#if !defined(WASM)
    glDisable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(reinterpret_cast<float*>(&mViewMatrix_));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#endif
}

void render_target::update_view_matrix_() const
{
    float fWidth = pTexture_->get_real_width();
    float fHeight = pTexture_->get_real_height();

    mViewMatrix_ = {
        2.0f/fWidth, 0.0f, -1.0f, -1.0f,
        0.0f, 2.0f/fHeight, -1.0f, -1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    bUpdateViewMatrix_ = false;
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

uint render_target::get_width() const
{
    return pTexture_->get_width();
}

uint render_target::get_height() const
{
    return pTexture_->get_height();
}

uint render_target::get_real_width() const
{
    return pTexture_->get_real_width();
}

uint render_target::get_real_height() const
{
    return pTexture_->get_real_height();
}

bool render_target::set_dimensions(uint uiWidth, uint uiHeight)
{
    bUpdateViewMatrix_ = true;

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

void render_target::check_availability()
{
#if !defined(WASM)
    if (!renderer::is_gl_extension_supported("GL_EXT_framebuffer_object"))
    {
        throw gui::exception("gui::gl::render_target", "OpenGL extension "
            "'GL_EXT_framebuffer_object' is not supported by your hardware.");
    }
#else
    // Always supported in OpenGLES3
#endif
}
}
}
}
