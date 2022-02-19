#include "lxgui/impl/gui_gl_vertexcache.hpp"

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

vertex_cache::vertex_cache(type type) : gui::vertex_cache(type) {
    glGenVertexArrays(1, &ui_vertex_array_);

    std::array<std::uint32_t, 2> buffers;
    glGenBuffers(buffers.size(), buffers.data());
    ui_vertex_buffer_ = buffers[0];
    ui_index_buffer_  = buffers[1];

    glBindVertexArray(ui_vertex_array_);

    glBindBuffer(GL_ARRAY_BUFFER, ui_vertex_buffer_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex),
        reinterpret_cast<const void*>(sizeof(vector2f) * 2));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<const void*>(sizeof(vector2f)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_index_buffer_);

    glBindVertexArray(0);
}

vertex_cache::~vertex_cache() {
    glDeleteVertexArrays(1, &ui_vertex_array_);

    std::array<std::uint32_t, 2> buffers = {ui_vertex_buffer_, ui_index_buffer_};
    glDeleteBuffers(buffers.size(), buffers.data());
}

void vertex_cache::update_data(const vertex* vertex_data, std::size_t ui_num_vertex) {
    glBindBuffer(GL_ARRAY_BUFFER, ui_vertex_buffer_);

    if (ui_num_vertex > ui_current_capacity_vertex_) {
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * ui_num_vertex, vertex_data, GL_DYNAMIC_DRAW);
        ui_current_capacity_vertex_ = ui_num_vertex;
    } else {
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex) * ui_num_vertex, vertex_data);
    }

    ui_current_size_vertex_ = ui_num_vertex;
}

void vertex_cache::update_indices(const std::uint32_t* vertex_indices, std::size_t ui_num_indices) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ui_index_buffer_);

    if (ui_num_indices > ui_current_capacity_index_) {
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, sizeof(std::uint32_t) * ui_num_indices, vertex_indices,
            GL_DYNAMIC_DRAW);
        ui_current_capacity_index_ = ui_num_indices;
    } else {
        glBufferSubData(
            GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(std::uint32_t) * ui_num_indices, vertex_indices);
    }

    ui_current_size_index_ = ui_num_indices;
}

void vertex_cache::update_indices_if_grow(
    const std::uint32_t* vertex_indices, std::size_t ui_num_indices) {
    if (ui_num_indices > ui_current_capacity_index_) {
        update_indices(vertex_indices, ui_num_indices);
    } else {
        // Cheap resize, do not update indices
        ui_current_size_index_ = ui_num_indices;
    }
}

void vertex_cache::update(const vertex* vertex_data, std::size_t ui_num_vertex) {
    if (type_ == type::quads) {
        static constexpr std::array<std::uint32_t, 6>  quad_i_ds = {{0, 1, 2, 2, 3, 0}};
        static thread_local std::vector<std::uint32_t> repeated_ids;

        if (ui_num_vertex % 4 != 0) {
            throw gui::exception(
                "gui::gl::vertex_cache", "Number of vertices in quad array must be a multiple of 4 "
                                         "(got " +
                                             utils::to_string(ui_num_vertex) + ").");
        }

        // Update the vertex data
        update_data(vertex_data, ui_num_vertex);

        // Update the repeated quads IDs array if it needs to grow
        std::size_t ui_num_indices = (ui_num_vertex / 4u) * 6u;
        if (ui_num_indices > repeated_ids.size()) {
            std::size_t ui_old_size = repeated_ids.size();
            repeated_ids.resize(ui_num_indices);
            for (std::size_t i = ui_old_size; i < ui_num_indices; ++i) {
                repeated_ids[i] = (i / 6) * 4 + quad_i_ds[i % 6];
            }
        }

        // Update the index cache
        update_indices_if_grow(repeated_ids.data(), ui_num_indices);
    } else {
        static thread_local std::vector<std::uint32_t> repeated_ids;

        if (ui_num_vertex % 3 != 0) {
            throw gui::exception(
                "gui::gl::vertex_cache",
                "Number of vertices in triangle array must be a multiple of 3 "
                "(got " +
                    utils::to_string(ui_num_vertex) + ").");
        }

        // Update the vertex data
        update_data(vertex_data, ui_num_vertex);

        // Update the repeated quads IDs array if it needs to grow
        std::size_t ui_num_indices = ui_num_vertex;
        if (ui_num_indices > repeated_ids.size()) {
            std::size_t ui_old_size = repeated_ids.size();
            repeated_ids.resize(ui_num_indices);
            for (std::size_t i = ui_old_size; i < ui_num_indices; ++i) {
                repeated_ids[i] = i;
            }
        }

        // Update the index cache
        update_indices_if_grow(repeated_ids.data(), ui_num_indices);
    }
}

void vertex_cache::render() const {
    glBindVertexArray(ui_vertex_array_);
    glDrawElements(GL_TRIANGLES, ui_current_size_index_, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

} // namespace lxgui::gui::gl
