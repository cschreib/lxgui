#include "lxgui/impl/gui_gl_material.hpp"
#include "lxgui/impl/gui_gl_renderer.hpp"

#include <lxgui/gui_out.hpp>
#include <lxgui/gui_exception.hpp>
#include <lxgui/utils_string.hpp>

#if defined(LXGUI_PLATFORM_WINDOWS)
    #define NOMINMAX
    #include <windows.h>
#endif

#if !defined(LXGUI_COMPILER_EMSCRIPTEN)
    #if defined(LXGUI_PLATFORM_OSX)
        #include <OpenGL/gl.h>
    #else
        #include <GL/gl.h>
    #endif
#else
    #if defined(LXGUI_PLATFORM_OSX)
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

material::material(const vector2ui& mDimensions, wrap mWrap, filter mFilter) :
    gui::material(false), mWrap_(mWrap), mFilter_(mFilter), bIsOwner_(true)
{
    if (ONLY_POWER_OF_TWO)
        mCanvasDimensions_ = vector2ui(next_pot(mDimensions.x), next_pot(mDimensions.y));
    else
        mCanvasDimensions_ = mDimensions;

    if (mCanvasDimensions_.x > MAXIMUM_SIZE || mCanvasDimensions_.y > MAXIMUM_SIZE)
    {
        throw gui::exception("gui::gl::material", "Texture dimensions not supported by graphics card : ("+
            utils::to_string(mCanvasDimensions_.x)+" x "+utils::to_string(mCanvasDimensions_.y)+").");
    }

    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);

    glGenTextures(1, &uiTextureHandle_);

    glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
        mCanvasDimensions_.x, mCanvasDimensions_.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
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

    mRect_ = bounds2f(0, mDimensions.x, 0, mDimensions.y);
}

material::material(uint uiTextureHandle, const vector2ui& mCanvasDimensions,
    const bounds2f mRect, filter mFilter) :
    gui::material(true), mCanvasDimensions_(mCanvasDimensions), mFilter_(mFilter),
    uiTextureHandle_(uiTextureHandle), mRect_(mRect), bIsOwner_(false)
{
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

void material::premultiply_alpha(std::vector<ub32color>& lData)
{
    for (auto& c : lData)
    {
        float a = c.a/255.0f;
        c.r *= a;
        c.g *= a;
        c.b *= a;
    }
}

bounds2f material::get_rect() const
{
    return mRect_;
}

vector2ui material::get_canvas_dimensions() const
{
    return mCanvasDimensions_;
}

bool material::uses_same_texture(const gui::material& mOther) const
{
    return uiTextureHandle_ == static_cast<const gl::material&>(mOther).uiTextureHandle_;
}

bool material::set_dimensions(const vector2ui& mDimensions)
{
    if (!bIsOwner_)
    {
        throw gui::exception("gui::gl::material", "A material in an atlas cannot be resized.");
    }

    bool bCanvasUpdated = false;

    if (mDimensions.x > mCanvasDimensions_.x || mDimensions.y > mCanvasDimensions_.y)
    {
        vector2ui mCanvasDimensions = mDimensions;
        if (ONLY_POWER_OF_TWO)
        {
            mCanvasDimensions.x = next_pot(mCanvasDimensions.x);
            mCanvasDimensions.y = next_pot(mCanvasDimensions.y);
        }

        if (mCanvasDimensions.x > MAXIMUM_SIZE || mCanvasDimensions.y > MAXIMUM_SIZE)
            return false;

        GLint iPreviousID;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);

        glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
            mCanvasDimensions.x, mCanvasDimensions.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
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

        mCanvasDimensions_ = mCanvasDimensions;
        bCanvasUpdated = true;
    }

    mRect_ = bounds2f(0, mDimensions.x, 0, mDimensions.y);
    return bCanvasUpdated;
}

void material::update_texture(const ub32color* pData)
{
    GLint iPreviousID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iPreviousID);

    glBindTexture(GL_TEXTURE_2D, uiTextureHandle_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, mRect_.left, mRect_.top, mRect_.width(), mRect_.height(),
        GL_RGBA, GL_UNSIGNED_BYTE, pData
    );

    glBindTexture(GL_TEXTURE_2D, iPreviousID);
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
