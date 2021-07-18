set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/gui/gl)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-gl
    ${SRCROOT}/gui_gl_atlas.cpp
    ${SRCROOT}/gui_gl_font.cpp
    ${SRCROOT}/gui_gl_renderer.cpp
    ${SRCROOT}/gui_gl_renderer_png.cpp
    ${SRCROOT}/gui_gl_material.cpp
    ${SRCROOT}/gui_gl_rendertarget.cpp
    ${SRCROOT}/gui_gl_vertexcache.cpp
)

# Need C++17
target_compile_features(lxgui-gl PRIVATE cxx_std_17)

target_include_directories(lxgui-gl PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(lxgui-gl PUBLIC ${FREETYPE_INCLUDE_DIRS})
if (NOT EMSCRIPTEN)
    target_include_directories(lxgui-gl PUBLIC ${PNG_INCLUDE_DIR})
    target_include_directories(lxgui-gl PUBLIC ${ZLIB_INCLUDE_DIR})
    target_include_directories(lxgui-gl PUBLIC ${OPENGL_INCLUDE_DIR})
    target_include_directories(lxgui-gl PUBLIC ${GLEW_INCLUDE_DIR})
else()
    set(GL_EMSCRIPTEN_USE_FLAGS "-s USE_LIBPNG=1 -s MIN_WEBGL_VERSION=2")
    set_target_properties(lxgui-gl PROPERTIES COMPILE_FLAGS ${GL_EMSCRIPTEN_USE_FLAGS})
    set_target_properties(lxgui-gl PROPERTIES LINK_FLAGS ${GL_EMSCRIPTEN_USE_FLAGS})
endif()

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/impl/gui_gl_*.hpp)
install(FILES ${files} DESTINATION include/lxgui/impl)
install(TARGETS lxgui-gl ARCHIVE DESTINATION lib)
