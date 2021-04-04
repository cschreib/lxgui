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

vertex_cache::vertex_cache(uint uiSizeHint) :
    uiCurrentCapacityVertex_(uiSizeHint), uiCurrentCapacityIndex_(uiSizeHint)
{
    glGenVertexArrays(1, &uiVertexArray_);

    std::array<uint,2> lBuffers;
    glGenBuffers(lBuffers.size(), lBuffers.data());
    uiVertexBuffer_ = lBuffers[0];
    uiIndexBuffer_ = lBuffers[1];

    glBindVertexArray(uiVertexArray_);

    glBindBuffer(GL_ARRAY_BUFFER, uiVertexBuffer_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<const void*>(sizeof(vector2f)*2));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<const void*>(sizeof(vector2f)));

    if (uiSizeHint != 0u)
    {
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * uiSizeHint, nullptr, GL_DYNAMIC_DRAW);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiIndexBuffer_);

    if (uiSizeHint != 0u)
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * uiSizeHint, nullptr, GL_DYNAMIC_DRAW);
    }

    glBindVertexArray(0);
}

vertex_cache::~vertex_cache()
{
    glDeleteVertexArrays(1, &uiVertexArray_);

    std::array<uint,2> lBuffers = { uiVertexBuffer_, uiIndexBuffer_ };
    glDeleteBuffers(lBuffers.size(), lBuffers.data());
}

void vertex_cache::update_data(const vertex* lVertexData, uint uiNumVertex)
{
    glBindBuffer(GL_ARRAY_BUFFER, uiVertexBuffer_);

    if (uiNumVertex > uiCurrentCapacityVertex_)
    {
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * uiNumVertex, lVertexData, GL_DYNAMIC_DRAW);
        uiCurrentCapacityVertex_ = uiNumVertex;
    }
    else
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex) * uiNumVertex, lVertexData);
    }

    uiCurrentSizeVertex_ = uiNumVertex;
}

void vertex_cache::update_indices(const uint* lVertexIndices, uint uiNumIndices)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiIndexBuffer_);

    if (uiNumIndices > uiCurrentCapacityIndex_)
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * uiNumIndices, lVertexIndices, GL_DYNAMIC_DRAW);
        uiCurrentCapacityIndex_ = uiNumIndices;
    }
    else
    {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint) * uiNumIndices, lVertexIndices);
    }

    uiCurrentSizeIndex_ = uiNumIndices;
}

uint vertex_cache::get_num_indices() const
{
    return uiCurrentSizeIndex_;
}

void vertex_cache::render(uint uiNumIndices)
{
    if (uiNumIndices == (uint)-1)
        uiNumIndices = uiCurrentSizeIndex_;

    if (uiNumIndices > uiCurrentSizeIndex_)
        throw gui::exception("gl::vertex_cache", "Too many indices requested in render().");

    glBindVertexArray(uiVertexArray_);
    glDrawElements(GL_TRIANGLES, uiNumIndices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

}
}
}
