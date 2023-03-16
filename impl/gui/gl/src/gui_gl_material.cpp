#include "lxgui/impl/gui_gl_material.hpp"

#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_gl_renderer.hpp"
#include "lxgui/utils_string.hpp"

#if defined(LXGUI_PLATFORM_WINDOWS)
#    define NOMINMAX
#    include <windows.h>
#endif

#if !defined(LXGUI_COMPILER_EMSCRIPTEN)
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

#if !defined(GL_CLAMP_TO_EDGE)
#    define GL_CLAMP_TO_EDGE 0x812F
#endif

#include <cmath>

namespace lxgui::gui::gl {

bool        material::only_power_of_two = true;
std::size_t material::maximum_size      = 128;

std::size_t next_pot(std::size_t size) {
    return std::pow(2.0f, std::ceil(std::log2(static_cast<float>(size))));
}

material::material(const vector2ui& dimensions, wrap wrp, filter filt) :
    gui::material(false), wrap_(wrp), filter_(filt), is_owner_(true) {
    if (only_power_of_two)
        canvas_dimensions_ = vector2ui(next_pot(dimensions.x), next_pot(dimensions.y));
    else
        canvas_dimensions_ = dimensions;

    if (canvas_dimensions_.x > maximum_size || canvas_dimensions_.y > maximum_size) {
        throw gui::exception(
            "gui::gl::material", "Texture dimensions not supported by graphics card: (" +
                                     utils::to_string(canvas_dimensions_.x) + " x " +
                                     utils::to_string(canvas_dimensions_.y) + ").");
    }

    GLint previous_id;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_id);

    glGenTextures(1, &texture_handle_);

    glBindTexture(GL_TEXTURE_2D, texture_handle_);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA8, canvas_dimensions_.x, canvas_dimensions_.y, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, nullptr);

    switch (wrp) {
    case wrap::clamp:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        break;
    case wrap::repeat:
    default:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;
    }

    switch (filt) {
    case filter::linear:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        break;
    case filter::none:
    default:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        break;
    }

    glBindTexture(GL_TEXTURE_2D, previous_id);

    rect_ = bounds2f(0, dimensions.x, 0, dimensions.y);
}

material::material(
    std::uint32_t    texture_handle,
    const vector2ui& canvas_dimensions,
    const bounds2f   rect,
    filter           filt) :
    gui::material(true),
    canvas_dimensions_(canvas_dimensions),
    filter_(filt),
    texture_handle_(texture_handle),
    rect_(rect),
    is_owner_(false) {}

material::~material() {
    if (is_owner_)
        glDeleteTextures(1, &texture_handle_);
}

void material::set_wrap(wrap wrp) {
    if (!is_owner_) {
        throw gui::exception(
            "gui::gl::material", "A material in an atlas cannot change its wrapping mode.");
    }

    GLint previous_id;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_id);
    glBindTexture(GL_TEXTURE_2D, texture_handle_);

    switch (wrp) {
    case wrap::clamp:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        break;
    case wrap::repeat:
    default:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        break;
    }

    glBindTexture(GL_TEXTURE_2D, previous_id);
}

void material::set_filter(filter filt) {
    if (!is_owner_) {
        throw gui::exception(
            "gui::gl::material", "A material in an atlas cannot change its filtering.");
    }

    GLint previous_id;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_id);
    glBindTexture(GL_TEXTURE_2D, texture_handle_);

    switch (filt) {
    case filter::linear:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        break;
    case filter::none:
    default:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        break;
    }

    glBindTexture(GL_TEXTURE_2D, previous_id);
}

material::filter material::get_filter() const {
    return filter_;
}

void material::bind() const {
    glBindTexture(GL_TEXTURE_2D, texture_handle_);
}

void material::premultiply_alpha(std::vector<color32>& data) {
    for (auto& c : data) {
        float a = c.a / 255.0f;
        c.r *= a;
        c.g *= a;
        c.b *= a;
    }
}

bounds2f material::get_rect() const {
    return rect_;
}

vector2ui material::get_canvas_dimensions() const {
    return canvas_dimensions_;
}

bool material::uses_same_texture(const gui::material& other) const {
    return texture_handle_ == static_cast<const gl::material&>(other).texture_handle_;
}

bool material::set_dimensions(const vector2ui& dimensions) {
    if (!is_owner_) {
        throw gui::exception("gui::gl::material", "A material in an atlas cannot be resized.");
    }

    bool canvas_updated = false;

    if (dimensions.x > canvas_dimensions_.x || dimensions.y > canvas_dimensions_.y) {
        vector2ui canvas_dimensions = dimensions;
        if (only_power_of_two) {
            canvas_dimensions.x = next_pot(canvas_dimensions.x);
            canvas_dimensions.y = next_pot(canvas_dimensions.y);
        }

        if (canvas_dimensions.x > maximum_size || canvas_dimensions.y > maximum_size)
            return false;

        GLint previous_id;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_id);

        glBindTexture(GL_TEXTURE_2D, texture_handle_);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA8, canvas_dimensions.x, canvas_dimensions.y, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, nullptr);

        switch (wrap_) {
        case wrap::clamp:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            break;
        case wrap::repeat:
        default:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            break;
        }
        switch (filter_) {
        case filter::linear:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            break;
        case filter::none:
        default:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            break;
        }

        glBindTexture(GL_TEXTURE_2D, previous_id);

        canvas_dimensions_ = canvas_dimensions;
        canvas_updated     = true;
    }

    rect_ = bounds2f(0, dimensions.x, 0, dimensions.y);
    return canvas_updated;
}

void material::update_texture(const color32* data) {
    GLint previous_id;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_id);

    glBindTexture(GL_TEXTURE_2D, texture_handle_);
    glTexSubImage2D(
        GL_TEXTURE_2D, 0, rect_.left, rect_.top, rect_.width(), rect_.height(), GL_RGBA,
        GL_UNSIGNED_BYTE, data);

    glBindTexture(GL_TEXTURE_2D, previous_id);
}

std::uint32_t material::get_handle() const {
    return texture_handle_;
}

void material::check_availability() {
#if !defined(LXGUI_OPENGL3)
    only_power_of_two = !renderer::is_gl_extension_supported("GL_ARB_texture_non_power_of_two");
#else
    // Non-power-of-two textures are always supported in OpenGL 3 / OpenGL ES 3
    only_power_of_two = false;
#endif

    int max = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
    maximum_size = max;
}

std::size_t material::get_max_size() {
    return maximum_size;
}

} // namespace lxgui::gui::gl
