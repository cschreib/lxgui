set(TARGET_DIR ${PROJECT_SOURCE_DIR}/impl/gui/sdl)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-sdl
    ${SRCROOT}/gui_sdl.cpp
    ${SRCROOT}/gui_sdl_atlas.cpp
    ${SRCROOT}/gui_sdl_font.cpp
    ${SRCROOT}/gui_sdl_renderer.cpp
    ${SRCROOT}/gui_sdl_material.cpp
    ${SRCROOT}/gui_sdl_render_target.cpp
)

add_library(lxgui::gui::sdl ALIAS lxgui-sdl)

# need C++17
target_compile_features(lxgui-sdl PRIVATE cxx_std_17)
lxgui_set_warning_level(lxgui-sdl)
target_include_directories(lxgui-sdl PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# needed dependencies
target_link_libraries(lxgui-sdl PUBLIC lxgui::lxgui)
if (NOT LXGUI_COMPILER_EMSCRIPTEN)
    target_link_libraries(lxgui-sdl PUBLIC SDL2::SDL2)
    target_link_libraries(lxgui-sdl PRIVATE SDL2::image)
    target_link_libraries(lxgui-sdl PRIVATE SDL2::TTF)
else()
    target_compile_options(lxgui-sdl PUBLIC "SHELL:-s USE_SDL=2")
    target_compile_options(lxgui-sdl PUBLIC "SHELL:-s USE_SDL_IMAGE=2")
    target_compile_options(lxgui-sdl PUBLIC "SHELL:-s SDL2_IMAGE_FORMATS='[\"png\"]'")
    target_compile_options(lxgui-sdl PUBLIC "SHELL:-s USE_SDL_TTF=2")
    target_link_options(lxgui-sdl PUBLIC "SHELL:-s USE_SDL=2")
    target_link_options(lxgui-sdl PUBLIC "SHELL:-s USE_SDL_IMAGE=2")
    target_link_options(lxgui-sdl PUBLIC "SHELL:-s SDL2_IMAGE_FORMATS='[\"png\"]'")
    target_link_options(lxgui-sdl PUBLIC "SHELL:-s USE_SDL_TTF=2")
    target_link_options(lxgui-sdl PUBLIC "SHELL:-s MIN_WEBGL_VERSION=1")
endif()

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/impl/gui_sdl*.hpp)
install(FILES ${files} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lxgui/impl)

list(APPEND LXGUI_INSTALL_TARGETS lxgui-sdl)
set(LXGUI_INSTALL_TARGETS ${LXGUI_INSTALL_TARGETS} PARENT_SCOPE)
