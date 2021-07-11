#include "lxgui/impl/gui_gl_vertexcache.hpp"

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

#include <array>

namespace lxgui {
namespace gui {
namespace gl
{

vertex_cache::vertex_cache(type mType) : gui::vertex_cache(mType)
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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiIndexBuffer_);

    glBindVertexArray(0);
}

vertex_cache::~vertex_cache()
{
    glDeleteVertexArrays(1, &uiVertexArray_);

    std::array<uint,2> lBuffers = { uiVertexBuffer_, uiIndexBuffer_ };
    glDeleteBuffers(lBuffers.size(), lBuffers.data());
}

void vertex_cache::update_data(const vertex* lVertexData, uint uiNumVertex, uint uiPosition)
{
    glBindBuffer(GL_ARRAY_BUFFER, uiVertexBuffer_);

    uint uiMaxIndex = uiNumVertex + uiPosition;

    if (uiMaxIndex > uiCurrentCapacityVertex_)
    {
        if (uiPosition == 0)
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*uiMaxIndex, lVertexData, GL_DYNAMIC_DRAW);
        }
        else
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*uiMaxIndex, nullptr, GL_DYNAMIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex)*uiPosition,
                sizeof(vertex)*uiNumVertex, lVertexData);
        }

        uiCurrentCapacityVertex_ = uiMaxIndex;
    }
    else
    {
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex)*uiPosition,
            sizeof(vertex)*uiNumVertex, lVertexData);
    }

    uiCurrentSizeVertex_ = std::max(uiCurrentSizeVertex_, uiMaxIndex);
}

void vertex_cache::update_indices(const uint* lVertexIndices, uint uiNumIndices)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uiIndexBuffer_);

    if (uiNumIndices > uiCurrentCapacityIndex_)
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*uiNumIndices,
            lVertexIndices, GL_DYNAMIC_DRAW);
        uiCurrentCapacityIndex_ = uiNumIndices;
    }
    else
    {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint)*uiNumIndices, lVertexIndices);
    }

    uiCurrentSizeIndex_ = uiNumIndices;
}

void vertex_cache::update_indices_if_grow(const uint* lVertexIndices, uint uiNumIndices)
{
    if (uiNumIndices > uiCurrentCapacityIndex_)
    {
        update_indices(lVertexIndices, uiNumIndices);
    }
    else
    {
        // Cheap resize, do not update indices
        uiCurrentSizeIndex_ = uiNumIndices;
    }
}

void vertex_cache::update(const vertex* lVertexData, uint uiNumVertex, uint uiPosition)
{
    if (mType_ == type::QUADS)
    {
        static constexpr std::array<uint, 6> lQuadIDs = {{0, 1, 2, 2, 3, 0}};
        static thread_local std::vector<uint> lRepeatedIds;

        if (uiNumVertex % 4 != 0)
        {
            throw gui::exception("gui::gl::vertex_cache",
                "Number of vertices in quad array must be a multiple of 4 "
                "(got "+utils::to_string(uiNumVertex)+").");
        }

        // Update the vertex data
        update_data(lVertexData, uiNumVertex, uiPosition);

        // Update the repeated quads IDs array if it needs to grow
        uint uiNumIndices = (uiNumVertex/4u)*6u + uiPosition;
        if (uiNumIndices > lRepeatedIds.size())
        {
            uint uiOldSize = lRepeatedIds.size();
            lRepeatedIds.resize(uiNumIndices);
            for (uint i = uiOldSize; i < uiNumIndices; ++i)
            {
                lRepeatedIds[i] = (i/6)*4 + lQuadIDs[i%6];
            }
        }

        // Update the index cache
        update_indices_if_grow(lRepeatedIds.data(), uiNumIndices);
    }
    else
    {
        static thread_local std::vector<uint> lRepeatedIds;

        if (uiNumVertex % 3 != 0)
        {
            throw gui::exception("gui::gl::vertex_cache",
                "Number of vertices in triangle array must be a multiple of 3 "
                "(got "+utils::to_string(uiNumVertex)+").");
        }

        // Update the vertex data
        update_data(lVertexData, uiNumVertex, uiPosition);

        // Update the repeated quads IDs array if it needs to grow
        uint uiNumIndices = uiNumVertex + uiPosition;
        if (uiNumIndices > lRepeatedIds.size())
        {
            uint uiOldSize = lRepeatedIds.size();
            lRepeatedIds.resize(uiNumIndices);
            for (uint i = uiOldSize; i < uiNumIndices; ++i)
            {
                lRepeatedIds[i] = i;
            }
        }

        // Update the index cache
        update_indices_if_grow(lRepeatedIds.data(), uiNumIndices);
    }
}

void vertex_cache::render() const
{
    glBindVertexArray(uiVertexArray_);
    glDrawElements(GL_TRIANGLES, uiCurrentSizeIndex_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

uint vertex_cache::get_num_vertex() const
{
    return uiCurrentSizeIndex_;
}

void vertex_cache::clear()
{
    uiCurrentSizeIndex_ = 0u;
    uiCurrentSizeVertex_ = 0u;
}

}
}
}
