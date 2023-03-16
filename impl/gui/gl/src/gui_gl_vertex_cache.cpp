#include "lxgui/impl/gui_gl_vertex_cache.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/utils_string.hpp"

#if defined(LXGUI_PLATFORM_WINDOWS)
#    define NOMINMAX
#    include <windows.h>
#endif

#if !defined(LXGUI_COMPILER_EMSCRIPTEN)
#    include <GL/glew.h>
#    if defined(LXGUI_PLATFORM_OSX)
#        include <OpenGL/gl.h>
#    else
#        include <GL/gl.h>
#    endif
#else
#    if defined(LXGUI_PLATFORM_OSX)
#        include <OpenGLES/ES3/gl.h>
#    else
#        include <GLES3/gl3.h>
#    endif
#endif

#include <array>

namespace lxgui::gui::gl {

vertex_cache::vertex_cache(type t) : gui::vertex_cache(t) {
    glGenVertexArrays(1, &vertex_array_);

    std::array<std::uint32_t, 2> buffers;
    glGenBuffers(buffers.size(), buffers.data());
    vertex_buffer_ = buffers[0];
    index_buffer_  = buffers[1];

    glBindVertexArray(vertex_array_);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex),
        reinterpret_cast<const void*>(sizeof(vector2f) * 2));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<const void*>(sizeof(vector2f)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);

    glBindVertexArray(0);
}

vertex_cache::~vertex_cache() {
    glDeleteVertexArrays(1, &vertex_array_);

    std::array<std::uint32_t, 2> buffers = {vertex_buffer_, index_buffer_};
    glDeleteBuffers(buffers.size(), buffers.data());
}

void vertex_cache::update_data(const vertex* vertex_data, std::size_t num_vertex) {
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);

    if (num_vertex > current_capacity_vertex_) {
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * num_vertex, vertex_data, GL_DYNAMIC_DRAW);
        current_capacity_vertex_ = num_vertex;
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex) * num_vertex, vertex_data);
    }

    current_size_vertex_ = num_vertex;
}

void vertex_cache::update_indices(const std::uint32_t* vertex_indices, std::size_t num_indices) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);

    if (num_indices > current_capacity_index_) {
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint32_t) * num_indices, vertex_indices,
            GL_DYNAMIC_DRAW);
        current_capacity_index_ = num_indices;
    } else {
        glBufferSubData(
            GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(std::uint32_t) * num_indices, vertex_indices);
    }

    current_size_index_ = num_indices;
    num_vertex_         = current_size_index_;
}

void vertex_cache::update_indices_if_grow(
    const std::uint32_t* vertex_indices, std::size_t num_indices) {
    if (num_indices > current_capacity_index_) {
        update_indices(vertex_indices, num_indices);
    } else {
        // Cheap resize, do not update indices
        current_size_index_ = num_indices;
        num_vertex_         = current_size_index_;
    }
}

void vertex_cache::update(const vertex* vertex_data, std::size_t num_vertex) {
    if (type_ == type::quads) {
        static constexpr std::array<std::uint32_t, 6>  quad_ids = {{0, 1, 2, 2, 3, 0}};
        static thread_local std::vector<std::uint32_t> repeated_ids;

        if (num_vertex % 4 != 0) {
            throw gui::exception(
                "gui::gl::vertex_cache",
                "Number of vertices in quad array must be a multiple of 4 (got " +
                    utils::to_string(num_vertex) + ").");
        }

        // Update the vertex data
        update_data(vertex_data, num_vertex);

        // Update the repeated quads IDs array if it needs to grow
        std::size_t num_indices = (num_vertex / 4u) * 6u;
        if (num_indices > repeated_ids.size()) {
            std::size_t old_size = repeated_ids.size();
            repeated_ids.resize(num_indices);
            for (std::size_t i = old_size; i < num_indices; ++i) {
                repeated_ids[i] = (i / 6) * 4 + quad_ids[i % 6];
            }
        }

        // Update the index cache
        update_indices_if_grow(repeated_ids.data(), num_indices);
    } else {
        static thread_local std::vector<std::uint32_t> repeated_ids;

        if (num_vertex % 3 != 0) {
            throw gui::exception(
                "gui::gl::vertex_cache",
                "Number of vertices in triangle array must be a multiple of 3 (got " +
                    utils::to_string(num_vertex) + ").");
        }

        // Update the vertex data
        update_data(vertex_data, num_vertex);

        // Update the repeated quads IDs array if it needs to grow
        std::size_t num_indices = num_vertex;
        if (num_indices > repeated_ids.size()) {
            std::size_t old_size = repeated_ids.size();
            repeated_ids.resize(num_indices);
            for (std::size_t i = old_size; i < num_indices; ++i) {
                repeated_ids[i] = i;
            }
        }

        // Update the index cache
        update_indices_if_grow(repeated_ids.data(), num_indices);
    }
}

void vertex_cache::render() const {
    glBindVertexArray(vertex_array_);
    glDrawElements(GL_TRIANGLES, current_size_index_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

} // namespace lxgui::gui::gl
