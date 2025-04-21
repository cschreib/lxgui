#include "lxgui/gui_exception.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/impl/gui_gl_material.hpp"
#include "lxgui/impl/gui_gl_renderer.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <png.h>

namespace {
[[noreturn]] void raise_error(png_struct* /*png*/, char const* message) {
    throw lxgui::gui::exception("gui::gl::manager", message);
}

void read_data(png_structp read_struct, png_bytep data, png_size_t length) {
    png_voidp p = png_get_io_ptr(read_struct);
    static_cast<std::ifstream*>(p)->read(reinterpret_cast<char*>(data), length);
}
} // namespace

namespace lxgui::gui::gl {

std::shared_ptr<gui::material>
renderer::create_material_png_(const std::string& file_name, material::filter filt) const {
    std::ifstream file(file_name, std::ios::binary);
    if (!file.is_open()) {
        throw gui::exception("gui::gl::manager", "Cannot find file '" + file_name + "'.");
    }

    const std::size_t pngsigsize = 8;
    png_byte          signature[pngsigsize];
    file.read(reinterpret_cast<char*>(signature), pngsigsize);
    if (!file.good() || png_sig_cmp(signature, 0, pngsigsize) != 0) {
        throw gui::exception(
            "gui::gl::manager", file_name + "' is not a valid PNG image: '" +
                                    std::string(std::begin(signature), std::end(signature)) + "'.");
    }

    png_structp read_struct = nullptr;
    png_infop   info_struct = nullptr;

    try {
        read_struct = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, raise_error, nullptr);
        if (!read_struct)
            throw gui::exception("gui::gl::manager", "'png_create_read_struct' failed.");

        info_struct = png_create_info_struct(read_struct);
        if (!info_struct)
            throw gui::exception("gui::gl::manager", "'png_create_info_struct' failed.");

        png_set_read_fn(read_struct, static_cast<png_voidp>(&file), read_data);

        png_set_sig_bytes(read_struct, pngsigsize);
        png_read_info(read_struct, info_struct);

        png_uint_32 depth = png_get_bit_depth(read_struct, info_struct);

        if (depth != 8)
            throw gui::exception(
                "gui::gl::manager", "only 8 bit color chanels are supported for PNG images.");

        png_uint_32 channels = png_get_channels(read_struct, info_struct);

        if (channels != 4 && channels != 3)
            throw gui::exception(
                "gui::gl::manager", "only RGB or RGBA is supported for PNG images.");

        png_uint_32 color_type = png_get_color_type(read_struct, info_struct);

        if (color_type == PNG_COLOR_TYPE_RGB) {
            png_set_filler(read_struct, 0xff, PNG_FILLER_AFTER);
        } else if (color_type != PNG_COLOR_TYPE_RGBA)
            throw gui::exception(
                "gui::gl::manager", "only RGB or RGBA is supported for PNG images.");

        std::size_t width  = png_get_image_width(read_struct, info_struct);
        std::size_t height = png_get_image_height(read_struct, info_struct);

        std::vector<color32>   data(width * height);
        std::vector<png_bytep> rows(height);

        for (std::size_t i = 0; i < height; ++i)
            rows[i] = reinterpret_cast<png_bytep>(&data[i * width]);

        png_read_image(read_struct, rows.data());

        png_destroy_read_struct(&read_struct, &info_struct, nullptr);

        material::premultiply_alpha(data);

        std::shared_ptr<material> tex = std::make_shared<gui::gl::material>(
            vector2ui(width, height), material::wrap::repeat, filt);

        tex->update_texture(data.data());

        return std::move(tex);
    } catch (const gui::exception& e) {
        gui::out << gui::error << "gui::gl::manager: Error parsing " << file_name << "."
                 << std::endl;
        gui::out << gui::error << e.what() << "" << std::endl;

        if (read_struct && info_struct)
            png_destroy_read_struct(&read_struct, &info_struct, nullptr);
        else if (read_struct)
            png_destroy_read_struct(&read_struct, nullptr, nullptr);

        throw;
    }
}

} // namespace lxgui::gui::gl
