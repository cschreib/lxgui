#include "lxgui/impl/gui_gl_manager.hpp"
#include "lxgui/impl/gui_gl_material.hpp"
#include <lxgui/gui_out.hpp>

#include <png.h>
#include <fstream>
#include <iostream>
#include <memory>

namespace gui {
namespace gl
{
void raise_error(png_struct* png, char const* message)
{
    throw utils::exception(message);
}

void read_data(png_structp pReadStruct, png_bytep pData, png_size_t uiLength)
{
    png_voidp p = png_get_io_ptr(pReadStruct);
    ((std::ifstream*)p)->read((char*)pData, uiLength);
}

utils::refptr<gui::material> manager::create_material_png(const std::string& sFileName, filter mFilter) const
{
    std::ifstream mFile(sFileName, std::ios::binary);
    if (!mFile.is_open())
    {
        gui::out << gui::warning << "gui::gl::manager : Cannot find file '" << sFileName << "'." << std::endl;
        return nullptr;
    }

    const uint PNGSIGSIZE = 8;
    png_byte lSignature[PNGSIGSIZE];
    mFile.read((char*)lSignature, PNGSIGSIZE);
    if (!mFile.good() || png_sig_cmp(lSignature, 0, PNGSIGSIZE) != 0)
    {
        gui::out << gui::warning << "gui::gl::manager : '" << sFileName <<
            "' is not a valid PNG image : '" << lSignature << "'." << std::endl;
        return nullptr;
    }

    png_structp pReadStruct = nullptr;
    png_infop pInfoStruct = nullptr;

    try
    {
        pReadStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, raise_error, nullptr);
        if (!pReadStruct)
        {
            gui::out << gui::error << "gui::gl::manager : 'png_create_read_struct' failed." << std::endl;
            return nullptr;
        }

        pInfoStruct = png_create_info_struct(pReadStruct);
        if (!pInfoStruct)
            throw utils::exception("'png_create_info_struct' failed.");

        png_set_read_fn(pReadStruct, (png_voidp)(&mFile), read_data);

        png_set_sig_bytes(pReadStruct, PNGSIGSIZE);
        png_read_info(pReadStruct, pInfoStruct);

        png_uint_32 uiDepth = png_get_bit_depth(pReadStruct, pInfoStruct);

        if (uiDepth != 8)
            throw utils::exception("only 8 bit color chanels are supported for PNG images.");

        png_uint_32 uiChannels = png_get_channels(pReadStruct, pInfoStruct);

        if (uiChannels != 4 && uiChannels != 3)
            throw utils::exception("only RGB or RGBA is supported for PNG images.");

        png_uint_32 uiColorType = png_get_color_type(pReadStruct, pInfoStruct);

        if (uiColorType == PNG_COLOR_TYPE_RGB)
        {
            png_set_filler(pReadStruct, 0xff, PNG_FILLER_AFTER);
        }
        else if (uiColorType != PNG_COLOR_TYPE_RGBA)
            throw utils::exception("only RGB or RGBA is supported for PNG images.");

        png_uint_32 uiWidth  = png_get_image_width(pReadStruct, pInfoStruct);
        png_uint_32 uiHeight = png_get_image_height(pReadStruct, pInfoStruct);

        std::unique_ptr<png_bytep[]> pRows(new png_bytep[uiHeight]);
        utils::refptr<material>  pTex(new gui::gl::material(
            uiWidth, uiHeight, gui::gl::material::REPEAT,
            (mFilter == FILTER_LINEAR ? gui::gl::material::LINEAR : gui::gl::material::NONE)
        ));

        png_bytep* pTempRows = pRows.get();
        ub32color* pTempData = pTex->get_data().data();
        for (uint i = 0; i < uiHeight; ++i)
            pTempRows[i] = (png_bytep)(pTempData + i*uiWidth);

        png_read_image(pReadStruct, pTempRows);

        png_destroy_read_struct(&pReadStruct, &pInfoStruct, nullptr);

        pTex->premultiply_alpha();
        pTex->update_texture();
        pTex->clear_cache_data_();
        lTextureList_[sFileName] = pTex;

        return pTex;
    }
    catch (const utils::exception& e)
    {
        gui::out << gui::error << "gui::gl::manager : Parsing " << sFileName << " :\n"
            << e.get_description() << std::endl;

        if (pReadStruct && pInfoStruct)
            png_destroy_read_struct(&pReadStruct, &pInfoStruct, nullptr);
        else if (pReadStruct)
            png_destroy_read_struct(&pReadStruct, nullptr, nullptr);

        return nullptr;
    }
}
}
}
