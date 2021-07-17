#include "lxgui/impl/gui_gl_atlas.hpp"
#include "lxgui/impl/gui_gl_material.hpp"
#include "lxgui/impl/gui_gl_renderer.hpp"

#include <lxgui/gui_out.hpp>
#include <lxgui/gui_exception.hpp>
#include <lxgui/utils_string.hpp>

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

#if !defined(GL_CLAMP_TO_EDGE)
    #define GL_CLAMP_TO_EDGE 0x812F
#endif

#include <cmath>

namespace lxgui {
namespace gui {
namespace gl
{

atlas_page::atlas_page(const gui::renderer& mRenderer, material::filter mFilter) : gui::atlas_page(mFilter)
{
    uiSize_ = mRenderer.get_texture_atlas_page_size();

    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);

    glGenTextures(1, &uiTextureHandle_);

    glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
        uiSize_, uiSize_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    switch (mFilter)
    {
    case material::filter::LINEAR :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        break;
    case material::filter::NONE :
    default :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        break;
    }

    glBindTexture(GL_TEXTURE_2D, iPreviousID);

    // Clear texture data (makes WebGL happy)
    uint uiFBO = 0;
    glGenFramebuffers(1, &uiFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, uiFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, uiTextureHandle_, 0);

    float lClearColor[4] = {0, 0, 0, 0};
    glClearBufferfv(GL_COLOR, 0, lClearColor);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &uiFBO);
}

atlas_page::~atlas_page()
{
    glDeleteTextures(1, &uiTextureHandle_);
}

std::shared_ptr<gui::material> atlas_page::add_material_(const gui::material& mMat,
    const quad2f& mLocation)
{
    const gl::material& mGLMat = static_cast<const gl::material&>(mMat);

    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);

    uint uiFBO = 0;
    glGenFramebuffers(1, &uiFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, uiFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mGLMat.get_handle_(), 0);

    std::vector<ub32color> lData(mLocation.width()*mLocation.height());
    glReadPixels(0, 0, mLocation.width(), mLocation.height(), GL_RGBA, GL_UNSIGNED_BYTE, lData.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &uiFBO);

    glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
        mLocation.left, mLocation.top, mLocation.width(), mLocation.height(),
        GL_RGBA, GL_UNSIGNED_BYTE, lData.data()
    );

    glBindTexture(GL_TEXTURE_2D, iPreviousID);

    return std::make_shared<gl::material>(uiTextureHandle_, uiSize_, uiSize_, mLocation, mFilter_);
}

float atlas_page::get_width() const
{
    return uiSize_;
}

float atlas_page::get_height() const
{
    return uiSize_;
}

atlas::atlas(const renderer& mRenderer, material::filter mFilter) : gui::atlas(mRenderer, mFilter) {}

std::unique_ptr<gui::atlas_page> atlas::create_page_() const
{
    return std::make_unique<gl::atlas_page>(mRenderer_, mFilter_);
}

}
}
}
