#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_gl_render_target.hpp"

#include <fstream>
#include <png.h>

namespace {
[[noreturn]] void raise_error(png_struct* /*png*/, char const* message) {
    throw lxgui::gui::exception("gui::gl::manager", message);
}

void write_data(png_structp write_struct, png_bytep data, png_size_t length) {
    png_voidp p = png_get_io_ptr(write_struct);
    static_cast<std::ofstream*>(p)->write(reinterpret_cast<char*>(data), length);
}

void flush_data(png_structp write_struct) {
    png_voidp p = png_get_io_ptr(write_struct);
    static_cast<std::ofstream*>(p)->flush();
}
} // namespace

namespace lxgui::gui::gl {

void render_target::save_rgba_to_png_(
    const std::string& file_name, const color32* data, std::size_t width, std::size_t height) {

    std::ofstream file(file_name, std::ios::binary);
    if (!file.is_open()) {
        throw gui::exception("gui::gl::manager", "Cannot write file '" + file_name + "'.");
    }

    png_structp write_struct = nullptr;
    png_infop   info_struct  = nullptr;

    try {
        write_struct = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!write_struct)
            throw gui::exception("gui::gl::manager", "'png_create_write_struct' failed.");

        info_struct = png_create_info_struct(write_struct);
        if (!info_struct)
            throw gui::exception("gui::gl::manager", "'png_create_info_struct' failed.");

        png_set_write_fn(write_struct, static_cast<png_voidp>(&file), write_data, flush_data);

        png_set_IHDR(
            write_struct, info_struct, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA,
            PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        png_write_info(write_struct, info_struct);

        for (std::size_t y = 0; y < height; ++y) {
            png_write_row(write_struct, reinterpret_cast<const png_byte*>(data + y * width));
        }

        png_write_end(write_struct, NULL);
    } catch (const gui::exception& e) {
        gui::out << gui::error << "gui::gl::manager: Error writing " << file_name << "."
                 << std::endl;
        gui::out << gui::error << e.what() << "" << std::endl;

        if (write_struct && info_struct)
            png_destroy_write_struct(&write_struct, &info_struct);
        else if (write_struct)
            png_destroy_write_struct(&write_struct, nullptr);

        throw;
    }
}

} // namespace lxgui::gui::gl
