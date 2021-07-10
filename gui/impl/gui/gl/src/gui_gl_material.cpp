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

namespace lxgui {
namespace gui {
namespace gl
{
bool material::ONLY_POWER_OF_TWO = true;
uint material::MAXIMUM_SIZE = 128;

uint next_pot(uint uiSize)
{
    return std::pow(2.0f, std::ceil(std::log2(static_cast<float>(uiSize))));
}

material::material(uint uiWidth, uint uiHeight, wrap mWrap, filter mFilter, bool bGPUOnly) :
    uiWidth_(uiWidth), uiHeight_(uiHeight), mWrap_(mWrap), mFilter_(mFilter), bIsOwner_(true)
{
    if (ONLY_POWER_OF_TWO)
    {
        uiRealWidth_ = next_pot(uiWidth);
        uiRealHeight_ = next_pot(uiHeight);
    }
    else
    {
        uiRealWidth_ = uiWidth;
        uiRealHeight_ = uiHeight;
    }

    if (uiRealWidth_ > MAXIMUM_SIZE || uiRealHeight_ > MAXIMUM_SIZE)
    {
        throw gui::exception("gui::gl::material", "Texture dimensions not supported by graphics card : ("+
            utils::to_string(uiRealWidth_)+" x "+
            utils::to_string(uiRealHeight_)+")."
        );
    }

    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);

    glGenTextures(1, &uiTextureHandle_);

    glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
        uiRealWidth_, uiRealHeight_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
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
        pData_.resize(uiWidth*uiHeight);

    mRect_ = quad2f(0, uiWidth_, 0, uiHeight_);
}

material::material(uint uiTextureHandle, uint uiWidth, uint uiHeight,
    const quad2f mRect, filter mFilter) : mFilter_(mFilter),
    uiTextureHandle_(uiTextureHandle), mRect_(mRect), bIsOwner_(false)
{
    uiWidth_ = mRect_.width();
    uiHeight_ = mRect_.height();
    uiRealWidth_ = uiWidth;
    uiRealHeight_ = uiHeight;
}

material::~material()
{
    if (bIsOwner_)
        glDeleteTextures(1, &uiTextureHandle_);
}

void material::set_wrap(wrap mWrap)
{
    if (!bIsOwner_)
    {
        throw gui::exception("gui::gl::material",
            "A material in an atlas cannot change its wrapping mode.");
    }

    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);
    glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);

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
    if (!bIsOwner_)
    {
        throw gui::exception("gui::gl::material",
            "A material in an atlas cannot change its filtering.");
    }

    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);
    glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);

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

material::filter material::get_filter() const
{
    return mFilter_;
}

void material::bind() const
{
    glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);
}

const std::vector<ub32color>& material::get_data() const
{
    return pData_;
}

std::vector<ub32color>& material::get_data()
{
    return pData_;
}

void material::set_pixel(uint x, uint y, const ub32color& mColor)
{
    pData_[x + y*uiWidth_] = mColor;
}

const ub32color& material::get_pixel(uint x, uint y) const
{
    return pData_[x + y*uiWidth_];
}

ub32color& material::get_pixel(uint x, uint y)
{
    return pData_[x + y*uiWidth_];
}

void material::premultiply_alpha()
{
    uint imax = uiWidth_*uiHeight_;
    for (uint i = 0; i < imax; ++i)
    {
        ub32color& c = pData_[i];
        float a = c.a/255.0f;
        c.r *= a;
        c.g *= a;
        c.b *= a;
    }
}

quad2f material::get_rect() const
{
    return mRect_;
}

float material::get_canvas_width() const
{
    return uiRealWidth_;
}

float material::get_canvas_height() const
{
    return uiRealHeight_;
}

bool material::set_dimensions(uint uiWidth, uint uiHeight)
{
    if (!bIsOwner_)
    {
        throw gui::exception("gui::gl::material", "A material in an atlas cannot be resized.");
    }

    uint uiRealWidth = uiWidth;
    uint uiRealHeight = uiHeight;
    if (ONLY_POWER_OF_TWO)
    {
        uiRealWidth  = next_pot(uiWidth);
        uiRealHeight = next_pot(uiHeight);
    }

    if (uiRealWidth > MAXIMUM_SIZE || uiRealHeight > MAXIMUM_SIZE)
        return false;

    uiWidth_  = uiWidth;
    uiHeight_ = uiHeight;
    mRect_    = quad2f(0, uiWidth_, 0, uiHeight_);

    if (uiWidth > uiRealWidth_ || uiHeight > uiRealHeight_)
    {
        uiRealWidth_  = uiRealWidth;
        uiRealHeight_ = uiRealHeight;

        GLint iPreviousID;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);

        glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
            uiRealWidth_, uiRealHeight_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
        );

        switch (mWrap_)
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
        switch (mFilter_)
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
        return false;
    }
}

void material::update_texture()
{
    if (!bIsOwner_)
    {
        throw gui::exception("gui::gl::material", "A material in an atlas cannot update its data.");
    }

    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);

    glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, uiWidth_, uiHeight_,
        GL_RGBA, GL_UNSIGNED_BYTE, pData_.data()
    );

    glBindTexture(GL_TEXTURE_2D, iPreviousID);
}

void material::clear_cache_data_()
{
    pData_.clear();
}

uint material::get_handle_() const
{
    return uiTextureHandle_;
}

void material::check_availability()
{
#if !defined(LXGUI_OPENGL3)
    ONLY_POWER_OF_TWO = !renderer::is_gl_extension_supported("GL_ARB_texture_non_power_of_two");
#else
    // Non-power-of-two textures are always supported in OpenGL 3 / OpenGL ES 3
    ONLY_POWER_OF_TWO = false;
#endif

    int iMax = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &iMax);
    MAXIMUM_SIZE = iMax;
}

uint material::get_max_size()
{
    return MAXIMUM_SIZE;
}

}
}
}
