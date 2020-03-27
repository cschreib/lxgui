set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/gui/gl)
set(SRCROOT ${TARGET_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LUA_INCLUDE_DIR})
include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories(${PNG_INCLUDE_DIR})
include_directories(${ZLIB_INCLUDE_DIR})
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIR})

add_library(lxgui-gl STATIC
    ${SRCROOT}/gui_gl_font.cpp
    ${SRCROOT}/gui_gl_renderer.cpp
    ${SRCROOT}/gui_gl_renderer_png.cpp
    ${SRCROOT}/gui_gl_material.cpp
    ${SRCROOT}/gui_gl_matrix4.cpp
    ${SRCROOT}/gui_gl_rendertarget.cpp
)

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/impl/gui_gl_*.hpp)
install(FILES ${files} DESTINATION include/lxgui/impl)
install(TARGETS lxgui-gl ARCHIVE DESTINATION lib)

