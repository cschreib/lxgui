#include "lxgui/impl/gui_gl_renderer.hpp"
#include "lxgui/impl/gui_gl_material.hpp"

#include <lxgui/gui_out.hpp>
#include <lxgui/gui_exception.hpp>

#include <png.h>
#include <fstream>
#include <iostream>
#include <memory>

namespace lxgui {
namespace gui {
namespace gl
{
[[noreturn]] void raise_error(png_struct* /*png*/, char const* message)
{
    throw gui::exception("gui::gl::manager", message);
}

void read_data(png_structp pReadStruct, png_bytep pData, png_size_t uiLength)
{
    png_voidp p = png_get_io_ptr(pReadStruct);
    static_cast<std::ifstream*>(p)->read(reinterpret_cast<char*>(pData), uiLength);
}

std::shared_ptr<gui::material> renderer::create_material_png(const std::string& sFileName, material::filter mFilter) const
{
    std::ifstream mFile(sFileName, std::ios::binary);
    if (!mFile.is_open())
    {
        throw gui::exception("gui::gl::manager", "Cannot find file '" + sFileName + "'.");
    }

    const uint PNGSIGSIZE = 8;
    png_byte lSignature[PNGSIGSIZE];
    mFile.read(reinterpret_cast<char*>(lSignature), PNGSIGSIZE);
    if (!mFile.good() || png_sig_cmp(lSignature, 0, PNGSIGSIZE) != 0)
    {
        throw gui::exception("gui::gl::manager", sFileName + "' is not a valid PNG image : '"
            + std::string(lSignature, lSignature + PNGSIGSIZE) + "'.");
    }

    png_structp pReadStruct = nullptr;
    png_infop pInfoStruct = nullptr;

    try
    {
        pReadStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, raise_error, nullptr);
        if (!pReadStruct)
            throw gui::exception("gui::gl::manager", "'png_create_read_struct' failed.");

        pInfoStruct = png_create_info_struct(pReadStruct);
        if (!pInfoStruct)
            throw gui::exception("gui::gl::manager", "'png_create_info_struct' failed.");

        png_set_read_fn(pReadStruct, static_cast<png_voidp>(&mFile), read_data);

        png_set_sig_bytes(pReadStruct, PNGSIGSIZE);
        png_read_info(pReadStruct, pInfoStruct);

        png_uint_32 uiDepth = png_get_bit_depth(pReadStruct, pInfoStruct);

        if (uiDepth != 8)
            throw gui::exception("gui::gl::manager", "only 8 bit color chanels are supported for PNG images.");

        png_uint_32 uiChannels = png_get_channels(pReadStruct, pInfoStruct);

        if (uiChannels != 4 && uiChannels != 3)
            throw gui::exception("gui::gl::manager", "only RGB or RGBA is supported for PNG images.");

        png_uint_32 uiColorType = png_get_color_type(pReadStruct, pInfoStruct);

        if (uiColorType == PNG_COLOR_TYPE_RGB)
        {
            png_set_filler(pReadStruct, 0xff, PNG_FILLER_AFTER);
        }
        else if (uiColorType != PNG_COLOR_TYPE_RGBA)
            throw gui::exception("gui::gl::manager", "only RGB or RGBA is supported for PNG images.");

        png_uint_32 uiWidth  = png_get_image_width(pReadStruct, pInfoStruct);
        png_uint_32 uiHeight = png_get_image_height(pReadStruct, pInfoStruct);

        std::unique_ptr<png_bytep[]> pRows(new png_bytep[uiHeight]);
        std::shared_ptr<material>  pTex = std::make_shared<gui::gl::material>(
            uiWidth, uiHeight, material::wrap::REPEAT, mFilter
        );

        png_bytep* pTempRows = pRows.get();
        ub32color* pTempData = pTex->get_data().data();
        for (uint i = 0; i < uiHeight; ++i)
            pTempRows[i] = reinterpret_cast<png_bytep>(pTempData + i*uiWidth);

        png_read_image(pReadStruct, pTempRows);

        png_destroy_read_struct(&pReadStruct, &pInfoStruct, nullptr);

        pTex->premultiply_alpha();
        pTex->update_texture();
        pTex->clear_cache_data_();
        lTextureList_[sFileName] = pTex;

        return std::move(pTex);
    }
    catch (const gui::exception& e)
    {
        gui::out << gui::error << "gui::gl::manager : Error parsing " << sFileName << "." << std::endl;

        if (pReadStruct && pInfoStruct)
            png_destroy_read_struct(&pReadStruct, &pInfoStruct, nullptr);
        else if (pReadStruct)
            png_destroy_read_struct(&pReadStruct, nullptr, nullptr);

        throw;
    }
}
}
}
}
