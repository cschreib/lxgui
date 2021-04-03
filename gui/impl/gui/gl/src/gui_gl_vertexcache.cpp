#include "lxgui/impl/gui_gl_vertexcache.hpp"

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

vertex_cache::vertex_cache(std::shared_ptr<gui::material> pMaterial) : gui::vertex_cache(std::move(pMaterial))
{
    glGenVertexArrays(1, &uiVertexArray_);

    std::array<uint,2> lBuffers;
    glGenBuffers(lBuffers.size(), lBuffers.data());
    uiVertexBuffer_ = lBuffers[0];
    uiIndexBuffer_ = lBuffers[1];

    glBindVertexArray(uiVertexArray_);

    glBindBuffer(GL_ARRAY_BUFFER, uiVertexBuffer_);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<const void*>(sizeof(vector2f)*2));
    glEnableVertexAttribArray(1);
    if (pMaterial_->get_type() == material::type::TEXTURE)
    {
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<const void*>(sizeof(vector2f)));
        glEnableVertexAttribArray(2);
    }
}

vertex_cache::~vertex_cache()
{
    glDeleteVertexArrays(1, &uiVertexArray_);

    std::array<uint,2> lBuffers = { uiVertexBuffer_, uiIndexBuffer_ };
    glDeleteBuffers(lBuffers.size(), lBuffers.data());
}

void vertex_cache::update(const std::vector<vertex>& lVertexData, const std::vector<uint>& lVertexIndices)
{
    update_data(lVertexData);
    update_indices(lVertexIndices);
}

void vertex_cache::update_data(const std::vector<vertex>& lVertexData)
{
    glBindBuffer(GL_ARRAY_BUFFER, uiVertexBuffer_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * lVertexData.size(), lVertexData.data(), GL_DYNAMIC_DRAW);
}

void vertex_cache::update_indices(const std::vector<uint>& lVertexIndices)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiIndexBuffer_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * lVertexIndices.size(), lVertexIndices.data(), GL_DYNAMIC_DRAW);
}

}
}
}
