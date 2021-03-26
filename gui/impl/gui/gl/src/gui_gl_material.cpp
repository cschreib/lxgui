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

#if defined(MSVC)
template<typename T>
T log2(T v)
{
    return log(v)/log(2.0);
}
#endif

namespace lxgui {
namespace gui {
namespace gl
{
bool material::ONLY_POWER_OF_TWO = true;
uint material::MAXIMUM_SIZE = 128;

material::material(uint uiWidth, uint uiHeight, wrap mWrap, filter mFilter, bool bGPUOnly) :
    mType_(type::TEXTURE)
{
    pTexData_ = std::unique_ptr<texture_data>(new texture_data());
    pTexData_->uiWidth_ = uiWidth;
    pTexData_->uiHeight_ = uiHeight;
    pTexData_->mWrap_ = mWrap;
    pTexData_->mFilter_ = mFilter;

    if (ONLY_POWER_OF_TWO)
    {
        pTexData_->uiRealWidth_ = pow(2.0f, ceil(log2((float)uiWidth)));
        pTexData_->uiRealHeight_ = pow(2.0f, ceil(log2((float)uiHeight)));
    }
    else
    {
        pTexData_->uiRealWidth_ = uiWidth;
        pTexData_->uiRealHeight_ = uiHeight;
    }

    if (pTexData_->uiRealWidth_ > MAXIMUM_SIZE || pTexData_->uiRealHeight_ > MAXIMUM_SIZE)
    {
        throw gui::exception("gui::gl::material", "Texture dimensions not supported by graphics card : ("+
            utils::to_string(pTexData_->uiRealWidth_)+" x "+
            utils::to_string(pTexData_->uiRealHeight_)+")."
        );
    }

    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);

    glGenTextures(1, &pTexData_->uiTextureHandle_);

    glBindTexture(GL_TEXTURE_2D, pTexData_->uiTextureHandle_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
        pTexData_->uiRealWidth_, pTexData_->uiRealHeight_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
    );

    switch (mWrap)
    {
    case wrap::CLAMP :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        break;
    case wrap::REPEAT :
    default :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;
    }
    switch (mFilter)
    {
    case filter::LINEAR :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        break;
    case filter::NONE :
    default :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        break;
    }

    glBindTexture(GL_TEXTURE_2D, iPreviousID);

    if (!bGPUOnly)
        pTexData_->pData_.resize(uiWidth*uiHeight);
}

material::material(const color& mColor) : mType_(type::COLOR)
{
    pColData_ = std::unique_ptr<color_data>(new color_data());
    pColData_->mColor_ = mColor;
}

material::~material()
{
    switch (mType_)
    {
    case type::TEXTURE :
        glDeleteTextures(1, &pTexData_->uiTextureHandle_); break;
    case type::COLOR :
        break;
    }
}

material::type material::get_type() const
{
    return mType_;
}

const color& material::get_color() const
{
    return pColData_->mColor_;
}

void material::set_wrap(wrap mWrap)
{
    if (!pTexData_) return;

    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);
    glBindTexture(GL_TEXTURE_2D, pTexData_->uiTextureHandle_);

    switch (mWrap)
    {
    case wrap::CLAMP :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        break;
    case wrap::REPEAT :
    default :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;
    }

    glBindTexture(GL_TEXTURE_2D, iPreviousID);
}

void material::set_filter(filter mFilter)
{
    if (!pTexData_) return;

    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);
    glBindTexture(GL_TEXTURE_2D, pTexData_->uiTextureHandle_);

    switch (mFilter)
    {
    case filter::LINEAR :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        break;
    case filter::NONE :
    default :
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        break;
    }

    glBindTexture(GL_TEXTURE_2D, iPreviousID);
}

void material::bind() const
{
    if (!pTexData_) return;

    glBindTexture(GL_TEXTURE_2D, pTexData_->uiTextureHandle_);
}

const std::vector<ub32color>& material::get_data() const
{
    return pTexData_->pData_;
}

std::vector<ub32color>& material::get_data()
{
    return pTexData_->pData_;
}

void material::set_pixel(uint x, uint y, const ub32color& mColor)
{
    pTexData_->pData_[x + y*pTexData_->uiWidth_] = mColor;
}

const ub32color& material::get_pixel(uint x, uint y) const
{
    return pTexData_->pData_[x + y*pTexData_->uiWidth_];
}

ub32color& material::get_pixel(uint x, uint y)
{
    return pTexData_->pData_[x + y*pTexData_->uiWidth_];
}

void material::premultiply_alpha()
{
    uint imax = pTexData_->uiWidth_*pTexData_->uiHeight_;
    for (uint i = 0; i < imax; ++i)
    {
        ub32color& c = pTexData_->pData_[i];
        float a = c.a/255.0f;
        c.r *= a;
        c.g *= a;
        c.b *= a;
    }
}

float material::get_width() const
{
    switch (mType_)
    {
    case type::TEXTURE :
        return pTexData_->uiWidth_;
    case type::COLOR :
        return 1.0f;
    }

    return 1.0f;
}

float material::get_height() const
{
    switch (mType_)
    {
    case type::TEXTURE :
        return pTexData_->uiHeight_;
    case type::COLOR :
        return 1.0f;
    }

    return 1.0f;
}

float material::get_real_width() const
{
    switch (mType_)
    {
    case type::TEXTURE :
        return pTexData_->uiRealWidth_;
    case type::COLOR :
        return 1.0f;
    }

    return 1.0f;
}

float material::get_real_height() const
{
    switch (mType_)
    {
    case type::TEXTURE :
        return pTexData_->uiRealHeight_;
    case type::COLOR :
        return 1.0f;
    }

    return 1.0f;
}

bool material::set_dimensions(uint uiWidth, uint uiHeight)
{
    if (!pTexData_) return false;

    uint uiRealWidth = uiWidth;
    uint uiRealHeight = uiHeight;
    if (ONLY_POWER_OF_TWO)
    {
        uiRealWidth  = pow(2.0f, ceil(log2((float)uiWidth)));
        uiRealHeight = pow(2.0f, ceil(log2((float)uiHeight)));
    }

    if (uiRealWidth > MAXIMUM_SIZE || uiRealHeight > MAXIMUM_SIZE)
        return false;

    if (uiWidth > pTexData_->uiRealWidth_ || uiHeight > pTexData_->uiRealHeight_)
    {
        pTexData_->uiWidth_      = uiWidth;
        pTexData_->uiHeight_     = uiHeight;
        pTexData_->uiRealWidth_  = uiRealWidth;
        pTexData_->uiRealHeight_ = uiRealHeight;

        GLint iPreviousID;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);

        glBindTexture(GL_TEXTURE_2D, pTexData_->uiTextureHandle_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
            pTexData_->uiRealWidth_, pTexData_->uiRealHeight_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
        );

        switch (pTexData_->mWrap_)
        {
        case wrap::CLAMP :
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            break;
        case wrap::REPEAT :
        default :
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            break;
        }
        switch (pTexData_->mFilter_)
        {
        case filter::LINEAR :
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            break;
        case filter::NONE :
        default :
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            break;
        }

        glBindTexture(GL_TEXTURE_2D, iPreviousID);
        return true;
    }
    else
    {
        pTexData_->uiWidth_  = uiWidth;
        pTexData_->uiHeight_ = uiHeight;
        return false;
    }
}

void material::update_texture()
{
    if (!pTexData_) return;

    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);

    glBindTexture(GL_TEXTURE_2D, pTexData_->uiTextureHandle_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pTexData_->uiWidth_, pTexData_->uiHeight_,
        GL_RGBA, GL_UNSIGNED_BYTE, pTexData_->pData_.data()
    );

    glBindTexture(GL_TEXTURE_2D, iPreviousID);
}

void material::clear_cache_data_()
{
    if (!pTexData_) return;

    pTexData_->pData_.clear();
}

uint material::get_handle_()
{
    return pTexData_->uiTextureHandle_;
}

void material::check_availability()
{
#if !defined(LXGUI_OPENGL3)
    ONLY_POWER_OF_TWO = !renderer::is_gl_extension_supported("GL_ARB_texture_non_power_of_two");
    gui::out << "Note : non power of two textures are " << (ONLY_POWER_OF_TWO ? "not " : "") << "supported." << std::endl;
#else
    // Non-power-of-two textures are always supported in OpenGL 3 / OpenGL ES 3
    ONLY_POWER_OF_TWO = false;
#endif

    int iMax = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &iMax);
    MAXIMUM_SIZE = iMax;
    gui::out << "Note : maximum texture size is " << MAXIMUM_SIZE << "." << std::endl;
}
}
}
}
