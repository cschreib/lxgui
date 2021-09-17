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

add_library(lxgui::gui::gl ALIAS lxgui-gl)

# need C++17
target_compile_features(lxgui-gl PRIVATE cxx_std_17)
lxgui_set_warning_level(lxgui-gl)
target_include_directories(lxgui-gl PUBLIC ${PROJECT_SOURCE_DIR}/include)

# needed dependencies
target_link_libraries(lxgui-gl PUBLIC lxgui::lxgui)
target_link_libraries(lxgui-gl PRIVATE Freetype::Freetype)
if (NOT LXGUI_COMPILER_EMSCRIPTEN)
    target_link_libraries(lxgui-gl PUBLIC OpenGL::GL)
    target_link_libraries(lxgui-gl PRIVATE PNG::PNG)
    target_link_libraries(lxgui-gl PRIVATE GLEW::GLEW)
else()
    set(LXGUI_GL_EMSCRIPTEN_USE_FLAGS "${LXGUI_EMSCRIPTEN_FLAGS} -s USE_LIBPNG=1 -s MIN_WEBGL_VERSION=2")
    set_target_properties(lxgui-gl PROPERTIES COMPILE_FLAGS ${LXGUI_GL_EMSCRIPTEN_USE_FLAGS})
    set_target_properties(lxgui-gl PROPERTIES LINK_FLAGS ${LXGUI_GL_EMSCRIPTEN_USE_FLAGS})
endif()

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/impl/gui_gl_*.hpp)
install(FILES ${files} DESTINATION include/lxgui/impl)
install(TARGETS lxgui-gl ARCHIVE DESTINATION lib)
