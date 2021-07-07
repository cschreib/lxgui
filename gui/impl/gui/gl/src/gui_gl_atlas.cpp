#include "lxgui/impl/gui_gl_atlas.hpp"

#include <lxgui/gui_out.hpp>
#include <lxgui/gui_exception.hpp>
#include <lxgui/utils_string.hpp>

#if defined(WIN32)
    #define NOMINMAX
    #include <windows.h>
#endif

#if !defined(WASM)
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

#if !defined(GL_CLAMP_TO_EDGE)
    #define GL_CLAMP_TO_EDGE 0x812F
#endif

#include <cmath>

namespace lxgui {
namespace gui {
namespace gl
{

atlas_page::atlas_page(material::filter mFilter) : gui::atlas_page(mFilter) {}

std::shared_ptr<gui::material> atlas_page::add_material_(const gui::material& mMat,
    const quad2f& mLocation)
{
    // TODO
    return nullptr;
}

float atlas_page::get_width() const
{
    // TODO
    return 0.0f;
}

float atlas_page::get_height() const
{
    // TODO
    return 0.0;
}

atlas::atlas(material::filter mFilter) : gui::atlas(mFilter) {}

std::unique_ptr<gui::atlas_page> atlas::create_page_() const
{
    return std::make_unique<gl::atlas_page>(mFilter_);
}

}
}
}
