#include "lxgui/impl/gui_gl_renderer.hpp"
#include "lxgui/impl/gui_gl_material.hpp"
#include "lxgui/impl/gui_gl_rendertarget.hpp"
#include "lxgui/impl/gui_gl_font.hpp"

#include <lxgui/gui_sprite.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/gui_manager.hpp>
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

const char* gl_error_to_string(GLenum mError)
{
    switch (mError)
    {
        case GL_NO_ERROR: return "no error";
        case GL_INVALID_ENUM: return "invalid enum";
        case GL_INVALID_VALUE: return "invalid value";
        case GL_INVALID_OPERATION: return "invalid operation";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "invalid framebuffer operation";
        case GL_OUT_OF_MEMORY: return "out of memory";
        default: return "unknown error";
    }
}

void print_gl_errors(const char* sDoingWhat)
{
    while (GLenum mError = glGetError())
    {
        lxgui::gui::out << "error in " << sDoingWhat << ": " << gl_error_to_string(mError) << std::endl;
    }
}

namespace lxgui {
namespace gui {
namespace gl
{
renderer::renderer(bool bInitGLEW [[maybe_unused]])
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
renderer::~renderer() noexcept
{
    glDeleteVertexArrays(uiVertexArray_.size(), uiVertexArray_.data());
    glDeleteBuffers(uiVertexBuffers_.size(), uiVertexBuffers_.data());

    if (uiTextureProgram_) glDeleteProgram(uiTextureProgram_);
    if (uiColorProgram_) glDeleteProgram(uiColorProgram_);
}
#endif

void renderer::update_view_matrix_() const
{
    float fWidth = pParent_->get_target_width();
    float fHeight = pParent_->get_target_height();

    mViewMatrix_ = {
        2.0f/fWidth, 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f/fHeight, 0.0f, 0.0f,
        -1.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };

    bUpdateViewMatrix_ = false;
}

void renderer::begin(std::shared_ptr<gui::render_target> pTarget) const
{
    if (pTarget)
    {
        pCurrentTarget_ = std::static_pointer_cast<gl::render_target>(pTarget);
        pCurrentTarget_->begin();
        pCurrentViewMatrix_ = &pCurrentTarget_->get_view_matrix();
    }
    else
    {
        if (bUpdateViewMatrix_)
            update_view_matrix_();

        glViewport(0.0f, 0.0f, pParent_->get_target_width(), pParent_->get_target_height());
        pCurrentViewMatrix_ = &mViewMatrix_;
    }

    print_gl_errors("setting viewport");
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // Premultipled alpha
    glDisable(GL_CULL_FACE);
    print_gl_errors("disabling culling");

#if !defined(LXGUI_OPENGL3)
    glDisable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(pCurrentViewMatrix_->data);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#else
    glActiveTexture(GL_TEXTURE0);
    print_gl_errors("setting active texture");
#endif

#if defined(LXGUI_OPENGL3)
    glUseProgram(uiTextureProgram_);
    print_gl_errors("use program");
    glUniformMatrix4fv(iTextureProjLocation_, 1, GL_FALSE, pCurrentViewMatrix_->data);
    print_gl_errors("setting view matrix texture");
    glUseProgram(uiColorProgram_);
    print_gl_errors("use program");
    glUniformMatrix4fv(iColorProjLocation_, 1, GL_FALSE, pCurrentViewMatrix_->data);
    print_gl_errors("setting view matrix color");

    uiPreviousTexture_ = (uint)-1;
    uiPreviousProgram_ = (uint)-1;
#endif
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

void renderer::render_quad(const quad& mQuad) const
{
    static constexpr std::array<uint, 6> ids = {{0, 1, 2, 2, 3, 0}};

#if !defined(LXGUI_OPENGL3)
    glColor4ub(255, 255, 255, 255);

    std::shared_ptr<gl::material> pMat = std::static_pointer_cast<gl::material>(mQuad.mat);
    if (pMat->get_type() == material::type::TEXTURE)
    {
        pMat->bind();

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
            color c = mQuad.v[j].col*pMat->get_color();
            c.r *= c.a; c.g *= c.a; c.b *= c.a;  // Premultipled alpha
            glColor4f(c.r, c.g, c.b, c.a);
            glVertex2f(mQuad.v[j].pos.x, mQuad.v[j].pos.y);
        }
        glEnd();
    }
#else
    std::shared_ptr<gl::material> pMat = std::static_pointer_cast<gl::material>(mQuad.mat);

    const uint uiArrayID = pMat->get_type() == material::type::TEXTURE ? 1 : 0;

    glBindVertexArray(uiVertexArray_[uiArrayID]);
    print_gl_errors("bind vertex array");

    glBindBuffer(GL_ARRAY_BUFFER, uiVertexBuffers_[2 + uiArrayID]);
    print_gl_errors("bind vertex buffer");

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * mQuad.v.size(), mQuad.v.data(), GL_DYNAMIC_DRAW);
    print_gl_errors("buffer data");

    if (pMat->get_type() == material::type::TEXTURE)
    {
        if (uiPreviousProgram_ != uiTextureProgram_)
        {
            glUseProgram(uiTextureProgram_);
            print_gl_errors("use program");
            uiPreviousProgram_ = uiTextureProgram_;
        }
        if (uiPreviousTexture_ != pMat->get_handle_())
        {
            pMat->bind();
            uiPreviousTexture_ = pMat->get_handle_();
        }
    }
    else
    {
        if (uiPreviousProgram_ != uiColorProgram_)
        {
            glUseProgram(uiColorProgram_);
            print_gl_errors("use program");
            uiPreviousProgram_ = uiColorProgram_;
        }
        glUniform4fv(iColorColLocation_, 1, &pMat->get_color().r);
        print_gl_errors("setting color color");
    }

    glDrawElements(GL_TRIANGLES, ids.size(), GL_UNSIGNED_INT, 0);
    print_gl_errors("draw");
#endif
}

void renderer::render_quads(const quad& mQuad, const std::vector<std::array<vertex,4>>& lQuadList) const
{
    static constexpr std::array<uint, 6> ids = {{0, 1, 2, 2, 3, 0}};

#if !defined(LXGUI_OPENGL3)
    glColor4ub(255, 255, 255, 255);

    std::shared_ptr<gl::material> pMat = std::static_pointer_cast<gl::material>(mQuad.mat);
    if (pMat->get_type() == material::type::TEXTURE)
    {
        pMat->bind();

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
                color c = v[j].col*pMat->get_color();
                c.r *= c.a; c.g *= c.a; c.b *= c.a; // Premultipled alpha
                glColor4f(c.r, c.g, c.b, c.a);
                glVertex2f(v[j].pos.x, v[j].pos.y);
            }
        }
        glEnd();
    }
#else
    std::shared_ptr<gl::material> pMat = std::static_pointer_cast<gl::material>(mQuad.mat);

    const uint uiArrayID = pMat->get_type() == material::type::TEXTURE ? 1 : 0;

    glBindVertexArray(uiVertexArray_[2 + uiArrayID]);
    print_gl_errors("bind vertex array");

    glBindBuffer(GL_ARRAY_BUFFER, uiVertexBuffers_[2 + uiArrayID]);
    print_gl_errors("bind vertex buffer");

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * 4 * lQuadList.size(), lQuadList.data(), GL_DYNAMIC_DRAW);
    print_gl_errors("buffer data");

    uint uiNewSize = 6*lQuadList.size();
    if (uiNewSize > lRepeatedIds_.size())
    {
        uint uiOldSize = lRepeatedIds_.size();
        lRepeatedIds_.resize(uiNewSize);
        for (uint i = uiOldSize; i < uiNewSize; ++i)
        {
            lRepeatedIds_[i] = (i/6)*4 + ids[i%6];
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiVertexBuffers_[1]);
        print_gl_errors("bind index buffer");
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * lRepeatedIds_.size(), lRepeatedIds_.data(), GL_DYNAMIC_DRAW);
        print_gl_errors("index buffer data");
    }

    if (pMat->get_type() == material::type::TEXTURE)
    {
        if (uiPreviousProgram_ != uiTextureProgram_)
        {
            glUseProgram(uiTextureProgram_);
            print_gl_errors("use program");
            uiPreviousProgram_ = uiTextureProgram_;
        }
        if (uiPreviousTexture_ != pMat->get_handle_())
        {
            pMat->bind();
            uiPreviousTexture_ = pMat->get_handle_();
        }
    }
    else
    {
        if (uiPreviousProgram_ != uiColorProgram_)
        {
            glUseProgram(uiColorProgram_);
            print_gl_errors("use program");
            uiPreviousProgram_ = uiColorProgram_;
        }
        glUniform4fv(iColorColLocation_, 1, &pMat->get_color().r);
        print_gl_errors("setting color color");
    }

    glDrawElements(GL_TRIANGLES, uiNewSize, GL_UNSIGNED_INT, 0);
    print_gl_errors("draw");
#endif
}

std::shared_ptr<gui::material> renderer::create_material(const std::string& sFileName, material::filter mFilter) const
{
    std::string sBackedName = utils::to_string((int)mFilter) + '|' + sFileName;
    std::map<std::string, std::weak_ptr<gui::material>>::iterator iter = lTextureList_.find(sBackedName);
    if (iter != lTextureList_.end())
    {
        if (std::shared_ptr<gui::material> pLock = iter->second.lock())
            return pLock;
        else
            lTextureList_.erase(iter);
    }

    if (utils::ends_with(sFileName, ".png"))
        return create_material_png(sFileName, mFilter);
    else
    {
        gui::out << gui::warning << "gui::gl::renderer : Unsupported texture format '"
            << sFileName << "'." << std::endl;
        return nullptr;
    }
}

std::shared_ptr<gui::material> renderer::create_material(const color& mColor) const
{
    return std::make_shared<material>(mColor);
}

std::shared_ptr<gui::material> renderer::create_material(std::shared_ptr<gui::render_target> pRenderTarget) const
{
    return std::static_pointer_cast<gl::render_target>(pRenderTarget)->get_material().lock();
}

std::shared_ptr<gui::render_target> renderer::create_render_target(uint uiWidth, uint uiHeight) const
{
    return std::make_shared<render_target>(uiWidth, uiHeight);
}

std::shared_ptr<gui::font> renderer::create_font(const std::string& sFontFile, uint uiSize) const
{
    std::string sFontName = sFontFile + "|" + utils::to_string(uiSize);
    std::map<std::string, std::weak_ptr<gui::font>>::iterator iter = lFontList_.find(sFontName);
    if (iter != lFontList_.end())
    {
        if (std::shared_ptr<gui::font> pLock = iter->second.lock())
            return pLock;
        else
            lFontList_.erase(iter);
    }

    std::shared_ptr<gui::font> pFont = std::make_shared<gl::font>(sFontFile, uiSize);
    lFontList_[sFontName] = pFont;
    return pFont;
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

            throw gui::exception("gl::rendere",
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
    char sTextureVertexShader[] =
        "#version 300 es                                         \n"
        "layout(location = 0) in vec2 a_position;                \n"
        "layout(location = 1) in vec4 a_color;                   \n"
        "layout(location = 2) in vec2 a_texCoord;                \n"
        "uniform mat4 m_proj;                                    \n"
        "out vec4 v_color;                                       \n"
        "out vec2 v_texCoord;                                    \n"
        "void main()                                             \n"
        "{                                                       \n"
        "    gl_Position = m_proj*vec4(a_position.xy,0,1);       \n"
        "    v_color = a_color;                                  \n"
        "    v_color.rgb *= v_color.a;                           \n"
        "    v_texCoord = a_texCoord;                            \n"
        "}                                                       \n";

    char sTextureFragmentShader[] =
        "#version 300 es                                         \n"
        "precision mediump float;                                \n"
        "in vec4 v_color;                                        \n"
        "in vec2 v_texCoord;                                     \n"
        "layout(location = 0) out vec4 o_color;                  \n"
        "uniform sampler2D s_texture;                            \n"
        "void main()                                             \n"
        "{                                                       \n"
        "    o_color = texture(s_texture, v_texCoord)*v_color;   \n"
        "}                                                       \n";

    char sColorVertexShader[] =
        "#version 300 es                                         \n"
        "layout(location = 0) in vec2 a_position;                \n"
        "layout(location = 1) in vec4 a_color;                   \n"
        "uniform mat4 m_proj;                                    \n"
        "uniform vec4 c_color;                                   \n"
        "out vec4 v_color;                                       \n"
        "void main()                                             \n"
        "{                                                       \n"
        "    gl_Position = m_proj*vec4(a_position.xy,0,1);       \n"
        "    v_color = a_color*c_color;                          \n"
        "    v_color.rgb *= v_color.a;                           \n"
        "}                                                       \n";

    char sColorFragmentShader[] =
        "#version 300 es                                         \n"
        "precision mediump float;                                \n"
        "in vec4 v_color;                                        \n"
        "layout(location = 0) out vec4 o_color;                  \n"
        "void main()                                             \n"
        "{                                                       \n"
        "    o_color = v_color;                                  \n"
        "}                                                       \n";

    try
    {
        uiTextureProgram_ = create_program(sTextureVertexShader, sTextureFragmentShader);
        uiColorProgram_ = create_program(sColorVertexShader, sColorFragmentShader);
    }
    catch (...)
    {
        if (uiTextureProgram_) glDeleteProgram(uiTextureProgram_);
        if (uiColorProgram_) glDeleteProgram(uiColorProgram_);
    }

    iTextureSamplerLocation_ = glGetUniformLocation(uiTextureProgram_, "s_texture");
    iTextureProjLocation_ = glGetUniformLocation(uiTextureProgram_, "m_proj");
    iColorProjLocation_ = glGetUniformLocation(uiColorProgram_, "m_proj");
    iColorColLocation_ = glGetUniformLocation(uiColorProgram_, "c_color");
}

void renderer::setup_buffers_()
{
    glGenBuffers(uiVertexBuffers_.size(), uiVertexBuffers_.data());
    glGenVertexArrays(uiVertexArray_.size(), uiVertexArray_.data());

    auto setup_vbo = [](uint uiID, bool bWithTexCoord)
    {
        glBindBuffer(GL_ARRAY_BUFFER, uiID);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<const void*>(sizeof(vector2f)*2));
        glEnableVertexAttribArray(1);
        if (bWithTexCoord)
        {
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<const void*>(sizeof(vector2f)));
            glEnableVertexAttribArray(2);
        }
    };

    auto setup_ibo = [](uint uiID)
    {
        static constexpr std::array<uint, 6> ids = {{0, 1, 2, 2, 3, 0}};
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiID);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * ids.size(), ids.data(), GL_DYNAMIC_DRAW);
    };

    // Color only array & single quad
    glBindVertexArray(uiVertexArray_[0]);

    setup_vbo(uiVertexBuffers_[2], false);
    setup_ibo(uiVertexBuffers_[0]);

    // Texture array & single quad
    glBindVertexArray(uiVertexArray_[1]);

    setup_vbo(uiVertexBuffers_[3], true);
    setup_ibo(uiVertexBuffers_[0]);

    // Color only array & multi quads
    glBindVertexArray(uiVertexArray_[2]);

    setup_vbo(uiVertexBuffers_[2], false);

    // Texture array & multi quads
    glBindVertexArray(uiVertexArray_[3]);

    setup_vbo(uiVertexBuffers_[3], true);
}
#endif

}
}
}
