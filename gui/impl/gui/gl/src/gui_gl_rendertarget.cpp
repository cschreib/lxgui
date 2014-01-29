#include "lxgui/impl/gui_gl_rendertarget.hpp"
#include "lxgui/impl/gui_gl_manager.hpp"

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glew.h>
#include <GL/gl.h>

#include <iostream>

namespace gui {
namespace gl
{
render_target::render_target(uint uiWidth, uint uiHeight) :
    uiFBOHandle_(0), bUpdateViewMatrix_(true)
{
    pTexture_ = utils::refptr<gl::material>(new gl::material(
        uiWidth, uiHeight, gl::material::REPEAT, gl::material::NONE, true
    ));

    glGenFramebuffers(1, &uiFBOHandle_);
    glBindFramebuffer(GL_FRAMEBUFFER, uiFBOHandle_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pTexture_->get_handle_(), 0);

    if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        throw gui::exception("render_target", "Failed creating render target.");
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
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
    glDisable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_CULL_FACE);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(reinterpret_cast<float*>(&mViewMatrix_));

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void render_target::update_view_matrix_() const
{
    float fWidth = pTexture_->get_real_width();
    float fHeight = pTexture_->get_real_height();

    float tmp[16] = {
        2.0f/fWidth, 0.0f, -1.0f, -1.0f,
        0.0f, 2.0f/fHeight, -1.0f, -1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    mViewMatrix_ = tmp;

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

        if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            throw gui::exception("render_target", "Failed resizing render target.");
        }

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

        return true;
    }

    return false;
}

utils::wptr<gl::material> render_target::get_material()
{
    return pTexture_;
}

void render_target::check_availability()
{
    if (!manager::is_gl_extension_supported("GL_EXT_framebuffer_object"))
    {
        throw gui::exception("render_target", "OpenGL extenion "
            "'GL_EXT_framebuffer_object' is not supported by your hardware.");
    }
}
}
}
