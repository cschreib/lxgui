set(TARGET_DIR ${PROJECT_SOURCE_DIR}/impl/gui/gl)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-gl
    ${SRCROOT}/gui_gl_atlas.cpp
    ${SRCROOT}/gui_gl_font.cpp
    ${SRCROOT}/gui_gl_renderer.cpp
    ${SRCROOT}/gui_gl_renderer_png.cpp
    ${SRCROOT}/gui_gl_material.cpp
    ${SRCROOT}/gui_gl_render_target.cpp
    ${SRCROOT}/gui_gl_vertex_cache.cpp
)

add_library(lxgui::gui::gl ALIAS lxgui-gl)

# need C++17
target_compile_features(lxgui-gl PRIVATE cxx_std_17)
lxgui_set_warning_level(lxgui-gl)
target_include_directories(lxgui-gl PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# needed dependencies
target_link_libraries(lxgui-gl PUBLIC lxgui::lxgui)
target_link_libraries(lxgui-gl PRIVATE Freetype::Freetype)
if (NOT LXGUI_COMPILER_EMSCRIPTEN)
    target_link_libraries(lxgui-gl PUBLIC OpenGL::GL)
    target_link_libraries(lxgui-gl PRIVATE PNG::PNG)
    target_link_libraries(lxgui-gl PRIVATE GLEW::GLEW)
else()
    target_compile_options(lxgui-gl PUBLIC "SHELL:-s USE_LIBPNG=1")
    target_link_options(lxgui-gl PUBLIC "SHELL:-s USE_LIBPNG=1")
    target_link_options(lxgui-gl PUBLIC "SHELL:-s MIN_WEBGL_VERSION=2")
endif()

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/impl/gui_gl_*.hpp)
install(FILES ${files} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lxgui/impl)

list(APPEND LXGUI_INSTALL_TARGETS lxgui-gl)
set(LXGUI_INSTALL_TARGETS ${LXGUI_INSTALL_TARGETS} PARENT_SCOPE)
