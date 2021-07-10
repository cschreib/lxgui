#include "lxgui/impl/gui_gl_renderer.hpp"
#include "lxgui/impl/gui_gl_atlas.hpp"
#include "lxgui/impl/gui_gl_material.hpp"
#include "lxgui/impl/gui_gl_rendertarget.hpp"
#include "lxgui/impl/gui_gl_font.hpp"
#include "lxgui/impl/gui_gl_vertexcache.hpp"

#include <lxgui/gui_sprite.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/gui_exception.hpp>
#include <lxgui/utils_string.hpp>

#ifdef WIN32
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

#if defined(LXGUI_OPENGL3)
thread_local std::weak_ptr<renderer::shader_cache> renderer::pStaticShaderCache_;
#endif

renderer::renderer(uint uiWindowWidth, uint uiWindowHeight, bool bInitGLEW [[maybe_unused]]) :
    uiWindowWidth_(uiWindowWidth), uiWindowHeight_(uiWindowHeight)
{
#if !defined(WASM)
    if (bInitGLEW)
        glewInit();
#endif

    render_target::check_availability();
    material::check_availability();

#if defined(LXGUI_OPENGL3)
    compile_programs_();
    setup_buffers_();
#endif
}

#if defined(LXGUI_OPENGL3)
renderer::shader_cache::~shader_cache()
{
    if (uiProgram_ != 0) glDeleteProgram(uiProgram_);
}
#endif

void renderer::begin(std::shared_ptr<gui::render_target> pTarget) const
{
    matrix4f mCurrentViewMatrix;

    if (pTarget)
    {
        pCurrentTarget_ = std::static_pointer_cast<gl::render_target>(pTarget);
        pCurrentTarget_->begin();

        mCurrentViewMatrix = pCurrentTarget_->get_view_matrix();
    }
    else
    {
        glViewport(0.0f, 0.0f, uiWindowWidth_, uiWindowHeight_);

        mCurrentViewMatrix = matrix4f::view(vector2f(uiWindowWidth_, uiWindowHeight_));
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Premultipled alpha
    glDisable(GL_CULL_FACE);

#if !defined(LXGUI_OPENGL3)
    glDisable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);
#else
    glActiveTexture(GL_TEXTURE0);
#endif

#if defined(LXGUI_OPENGL3)
    glUseProgram(pShaderCache_->uiProgram_);
    uiPreviousTexture_ = static_cast<uint>(-1);
#endif

    set_view(mCurrentViewMatrix);
}

void renderer::end() const
{
#if defined(LXGUI_OPENGL3)
#endif

    if (pCurrentTarget_)
    {
        pCurrentTarget_->end();
        pCurrentTarget_ = nullptr;
    }
}

void renderer::set_view(const matrix4f& mViewMatrix) const
{
    mCurrentViewMatrix_ = mViewMatrix;

    matrix4f mCorrectedView = mViewMatrix;
    if (!pCurrentTarget_)
    {
        // Rendering to main screen, flip Y
        for (uint i = 0; i < 4; ++i)
            mCorrectedView(i,1) *= -1.0f;
    }

#if !defined(LXGUI_OPENGL3)
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(mCorrectedView.data);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#else
    glUniformMatrix4fv(pShaderCache_->iProjLocation_, 1, GL_FALSE, mCorrectedView.data);
#endif
}

matrix4f renderer::get_view() const
{
    return mCurrentViewMatrix_;
}

void renderer::render_quad(const quad& mQuad) const
{
#if !defined(LXGUI_OPENGL3)
    static constexpr std::array<uint, 6> ids = {{0, 1, 2, 2, 3, 0}};

    glColor4ub(255, 255, 255, 255);

    const gl::material* pMat = static_cast<const gl::material*>(mQuad.mat.get());
    if (mMat)
    {
        mMat->bind();

        glEnable(GL_TEXTURE_2D);
        glBegin(GL_TRIANGLES);
        for (uint i = 0; i < 6; ++i)
        {
            uint j = ids[i];
            float a = mQuad.v[j].col.a;
            glColor4f(mQuad.v[j].col.r*a, mQuad.v[j].col.g*a, mQuad.v[j].col.b*a, a); // Premultipled alpha
            glTexCoord2f(mQuad.v[j].uvs.x, mQuad.v[j].uvs.y);
            glVertex2f(mQuad.v[j].pos.x, mQuad.v[j].pos.y);
        }
        glEnd();
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_TRIANGLES);
        for (uint i = 0; i < 6; ++i)
        {
            uint j = ids[i];
            glColor4f(mQuad.v[j].col.r*a, mQuad.v[j].col.g*a, mQuad.v[j].col.b*a, a); // Premultipled alpha
            glVertex2f(mQuad.v[j].pos.x, mQuad.v[j].pos.y);
        }
        glEnd();
    }
#else
    // Note: we rotate through a fairly large number of vertex caches
    // rather than constantly reusing the same cache. This is because
    // update_data() calls glBufferSubData(), which will wait for the
    // previous draw call using this cache to finish before updating.
    // If we rotate, it is more likely that the draw call is done, and
    // that we don't have to wait.
    const auto& pCache = pQuadCache_[uiQuadCycleCache_];
    uiQuadCycleCache_ = (uiQuadCycleCache_ + 1) % CACHE_CYCLE_SIZE;

    // Update vertex data
    pCache->update_data(mQuad.v.data(), mQuad.v.size());

    // Render
    render_cache(mQuad.mat.get(), *pCache, matrix4f::IDENTITY);
#endif
}

void renderer::render_quads(const gui::material* pMaterial, const std::vector<std::array<vertex,4>>& lQuadList) const
{

#if !defined(LXGUI_OPENGL3)
    static constexpr std::array<uint, 6> ids = {{0, 1, 2, 2, 3, 0}};
    glColor4ub(255, 255, 255, 255);

    const gl::material* pMat = static_cast<const gl::material*>(mMaterial);
    if (mMat)
    {
        mMat->bind();

        glEnable(GL_TEXTURE_2D);
        glBegin(GL_TRIANGLES);
        for (const auto& v :lQuadList)
        {
            for (uint i = 0; i < 6; ++i)
            {
                uint j = ids[i];
                float a = v[j].col.a;
                glColor4f(v[j].col.r*a, v[j].col.g*a, v[j].col.b*a, a); // Premultipled alpha
                glTexCoord2f(v[j].uvs.x, v[j].uvs.y);
                glVertex2f(v[j].pos.x, v[j].pos.y);
            }
        }
        glEnd();
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_TRIANGLES);
        for (const auto& v : lQuadList)
        {
            for (uint i = 0; i < 6; ++i)
            {
                uint j = ids[i];
                glColor4f(v[j].col.r*a, v[j].col.g*a, v[j].col.b*a, a); // Premultipled alpha
                glVertex2f(v[j].pos.x, v[j].pos.y);
            }
        }
        glEnd();
    }
#else
    // Note: we rotate through a fairly large number of vertex caches
    // rather than constantly reusing the same cache. This is because
    // update_data() calls glBufferSubData(), which will wait for the
    // previous draw call using this cache to finish before updating.
    // If we rotate, it is more likely that the draw call is done, and
    // that we don't have to wait.
    const auto& pCache = pArrayCache_[uiArrayCycleCache_];
    uiArrayCycleCache_ = (uiArrayCycleCache_ + 1) % CACHE_CYCLE_SIZE;

    // Update vertex data
    pCache->update(lQuadList[0].data(), lQuadList.size()*4);

    // Render
    render_cache(pMaterial, *pCache, matrix4f::IDENTITY);
#endif
}

void renderer::render_cache(const gui::material* pMaterial, const gui::vertex_cache& mCache,
    const matrix4f& mModelTransform) const
{
#if !defined(LXGUI_OPENGL3)
    throw gui::exception("gl::renderer", "Legacy OpenGL does not support vertex caches.");
#else
    const gl::material* pMat = static_cast<const gl::material*>(pMaterial);
    const gl::vertex_cache& mGLCache = static_cast<const gl::vertex_cache&>(mCache);

    // Setup uniforms
    int iType = 0;
    if (pMat)
    {
        iType = 0;
        if (uiPreviousTexture_ != pMat->get_handle_())
        {
            pMat->bind();
            uiPreviousTexture_ = pMat->get_handle_();
        }
    }
    else
    {
        iType = 1;
    }

    glUniform1i(pShaderCache_->iTypeLocation_, iType);
    glUniformMatrix4fv(pShaderCache_->iModelLocation_, 1, GL_FALSE, mModelTransform.data);

    // Render
    mGLCache.render();
#endif
}

std::shared_ptr<gui::material> renderer::create_material_(const std::string& sFileName, material::filter mFilter) const
{
    if (!utils::ends_with(sFileName, ".png"))
        throw gui::exception("gui::gl::renderer", "Unsupported texture format '" + sFileName + "'.");

    return create_material_png(sFileName, mFilter);
}

std::shared_ptr<gui::atlas> renderer::create_atlas_(material::filter mFilter) const
{
#if defined(WASM)
    throw gui::exception("gui::gl::renderer", "WebGL has no native support for texture atlases.");
#else
    return std::make_shared<gl::atlas>(*this, mFilter);
#endif
}

uint renderer::get_texture_max_size() const
{
    return material::get_max_size();
}

std::shared_ptr<gui::material> renderer::create_material(
    std::shared_ptr<gui::render_target> pRenderTarget, const quad2f& mLocation) const
{
    auto pTex = std::static_pointer_cast<gl::render_target>(pRenderTarget)->get_material().lock();
    if (mLocation == pRenderTarget->get_rect())
    {
        return pTex;
    }
    else
    {
        return std::make_shared<gl::material>(pTex->get_handle_(), mLocation, pTex->get_filter());
    }
}

std::shared_ptr<gui::render_target> renderer::create_render_target(uint uiWidth, uint uiHeight) const
{
    return std::make_shared<gl::render_target>(uiWidth, uiHeight);
}

std::shared_ptr<gui::font> renderer::create_font_(const std::string& sFontFile, uint uiSize) const
{
    return std::make_shared<gl::font>(sFontFile, uiSize);
}

bool renderer::is_texture_atlas_natively_supported() const
{
#if defined(WASM)
    return false;
#else
    return true;
#endif
}

bool renderer::is_vertex_cache_supported() const
{
#if !defined(LXGUI_OPENGL3)
    return false;
#else
    return true;
#endif
}

std::shared_ptr<gui::vertex_cache> renderer::create_vertex_cache(gui::vertex_cache::type mType) const
{
#if !defined(LXGUI_OPENGL3)
    throw gui::exception("gl::renderer", "Legacy OpenGL does not support vertex caches.");
#else
    return std::make_shared<gl::vertex_cache>(mType);
#endif
}

void renderer::notify_window_resized(uint uiNewWidth, uint uiNewHeight)
{
    uiWindowWidth_ = uiNewWidth;
    uiWindowHeight_ = uiNewHeight;
}

#if !defined(LXGUI_OPENGL3)
bool renderer::is_gl_extension_supported(const std::string& sExtension)
{
    // Extension names should not have spaces
    if (sExtension.find(' ') != std::string::npos || sExtension.empty())
        return false;

    GLint uiNumExtension = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &uiNumExtension);

    for (GLuint uiIndex = 0; uiIndex < (uint)uiNumExtension; ++uiIndex)
    {
        if (sExtension == reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, uiIndex)))
            return true;
    }

    return false;
}
#endif

#if defined(LXGUI_OPENGL3)
GLuint create_shader(GLenum mType, const char* sShaderSource)
{
    // Create the shader
    GLuint uiShader = glCreateShader(mType);
    if (uiShader == 0)
    {
        throw gui::exception("gl::renderer", "Could not create "+
            std::string(mType == GL_VERTEX_SHADER ? "vertex" : "fragment")+" shader.");
    }

    glShaderSource(uiShader, 1, &sShaderSource, nullptr);
    glCompileShader(uiShader);

    // Check sucess
    GLint iCompiled = 0;
    glGetShaderiv(uiShader, GL_COMPILE_STATUS, &iCompiled);
    if (iCompiled == 0)
    {
        GLint iInfoLength = 0;
        glGetProgramiv(uiShader, GL_INFO_LOG_LENGTH, &iInfoLength);

        std::vector<char> sErrorMessage(std::max(1, iInfoLength), '\0');
        if (iInfoLength > 1)
        {
            glGetProgramInfoLog(uiShader, iInfoLength, NULL, sErrorMessage.data());
        }

        glDeleteShader(uiShader);
        throw gui::exception("gl::renderer",
            "Could not compile shader: "+std::string(sErrorMessage.data()));
   }

   return uiShader;
}

GLuint create_program(const char* sVertexShaderSource, const char* sFragmentShaderSource)
{
    GLuint uiVertexShader = 0;
    GLuint uiFragmentShader = 0;
    GLuint uiProgramObject = 0;

    try
    {
        // Create shaders
        uiVertexShader = create_shader(GL_VERTEX_SHADER, sVertexShaderSource);
        uiFragmentShader = create_shader(GL_FRAGMENT_SHADER, sFragmentShaderSource);

        // Create program
        uiProgramObject = glCreateProgram();
        if (uiProgramObject == 0)
        {
            throw gui::exception("gl::renderer", "Could not create shader program.");
        }

        glAttachShader(uiProgramObject, uiVertexShader);
        glAttachShader(uiProgramObject, uiFragmentShader);
        glLinkProgram(uiProgramObject);

        // Check success
        GLint iLinked = 0;
        glGetProgramiv(uiProgramObject, GL_LINK_STATUS, &iLinked);
        if (iLinked == 0)
        {
            GLint iInfoLength = 0;
            glGetProgramiv(uiProgramObject, GL_INFO_LOG_LENGTH, &iInfoLength);

            std::vector<char> sErrorMessage(std::max(1, iInfoLength), '\0');
            if (iInfoLength > 1)
            {
                glGetProgramInfoLog(uiProgramObject, iInfoLength, NULL, sErrorMessage.data());
            }

            throw gui::exception("gl::renderer",
                "Could not link shader program: "+std::string(sErrorMessage.data()));
        }
    }
    catch (...)
    {
        if (uiVertexShader != 0) glDeleteShader(uiVertexShader);
        if (uiFragmentShader != 0) glDeleteShader(uiFragmentShader);
        if (uiProgramObject != 0) glDeleteProgram(uiProgramObject);
        throw;
    }

    glDeleteShader(uiVertexShader);
    glDeleteShader(uiFragmentShader);

    return uiProgramObject;
}

void renderer::compile_programs_()
{
    // Shaders are compiled once, and reused by other renderers
    thread_local bool bShaderCached = false;

    if (!bShaderCached)
    {
        char sVertexShader[] =
            "#version 300 es                                           \n"
            "layout(location = 0) in vec2 a_position;                  \n"
            "layout(location = 1) in vec4 a_color;                     \n"
            "layout(location = 2) in vec2 a_texCoord;                  \n"
            "uniform mat4 m_proj;                                      \n"
            "uniform mat4 m_model;                                     \n"
            "out vec4 v_color;                                         \n"
            "out vec2 v_texCoord;                                      \n"
            "void main()                                               \n"
            "{                                                         \n"
            "    gl_Position = m_proj*m_model*vec4(a_position.xy,0,1); \n"
            "    v_color = a_color;                                    \n"
            "    v_color.rgb *= v_color.a;                             \n"
            "    v_texCoord = a_texCoord;                              \n"
            "}                                                         \n";

        char sFragmentShader[] =
            "#version 300 es                                           \n"
            "precision mediump float;                                  \n"
            "in vec4 v_color;                                          \n"
            "in vec2 v_texCoord;                                       \n"
            "layout(location = 0) out vec4 o_color;                    \n"
            "uniform mediump int i_type;                               \n"
            "uniform sampler2D s_texture;                              \n"
            "void main()                                               \n"
            "{                                                         \n"
            "    if (i_type == 0)                                      \n"
            "        o_color = texture(s_texture, v_texCoord)*v_color; \n"
            "    else                                                  \n"
            "        o_color = v_color;                                \n"
            "}                                                         \n";

        pShaderCache_ = std::make_shared<shader_cache>();

        try
        {
            pShaderCache_->uiProgram_ = create_program(sVertexShader, sFragmentShader);
        }
        catch (...)
        {
            pShaderCache_ = nullptr;
            throw;
        }

        pShaderCache_->iSamplerLocation_ = glGetUniformLocation(pShaderCache_->uiProgram_, "s_texture");
        pShaderCache_->iProjLocation_ = glGetUniformLocation(pShaderCache_->uiProgram_, "m_proj");
        pShaderCache_->iModelLocation_ = glGetUniformLocation(pShaderCache_->uiProgram_, "m_model");
        pShaderCache_->iTypeLocation_ = glGetUniformLocation(pShaderCache_->uiProgram_, "i_type");

        pStaticShaderCache_ = pShaderCache_;
        bShaderCached = true;
    }
    else
    {
        pShaderCache_ = pStaticShaderCache_.lock();
    }
}

void renderer::setup_buffers_()
{
    static constexpr std::array<uint, 6> lQuadIDs = {{0, 1, 2, 2, 3, 0}};

    constexpr uint uiNumArrayIndices = 768u;
    std::vector<uint> lRepeatedIds(uiNumArrayIndices);
    for (uint i = 0; i < uiNumArrayIndices; ++i)
    {
        lRepeatedIds[i] = (i/6)*4 + lQuadIDs[i%6];
    }

    for (uint i = 0; i < CACHE_CYCLE_SIZE; ++i)
    {
        pQuadCache_[i] = std::static_pointer_cast<gl::vertex_cache>(create_vertex_cache(
            vertex_cache::type::QUADS));
        pQuadCache_[i]->update_indices(lQuadIDs.data(), lQuadIDs.size());

        pArrayCache_[i] = std::static_pointer_cast<gl::vertex_cache>(create_vertex_cache(
            vertex_cache::type::QUADS));
        pArrayCache_[i]->update_indices(lRepeatedIds.data(), lRepeatedIds.size());
    }
}
#endif

}
}
}
