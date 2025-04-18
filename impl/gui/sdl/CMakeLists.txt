set(TARGET_DIR ${PROJECT_SOURCE_DIR}/impl/gui/sdl)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-gui-sdl
    ${SRCROOT}/gui_sdl.cpp
    ${SRCROOT}/gui_sdl_atlas.cpp
    ${SRCROOT}/gui_sdl_font.cpp
    ${SRCROOT}/gui_sdl_renderer.cpp
    ${SRCROOT}/gui_sdl_material.cpp
    ${SRCROOT}/gui_sdl_render_target.cpp
)

add_library(lxgui::gui::sdl ALIAS lxgui-gui-sdl)
set_target_properties(lxgui-gui-sdl PROPERTIES EXPORT_NAME gui::sdl)

# need C++17
target_compile_features(lxgui-gui-sdl PRIVATE cxx_std_17)
lxgui_set_warning_level(lxgui-gui-sdl)
target_include_directories(lxgui-gui-sdl PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# needed dependencies
target_link_libraries(lxgui-gui-sdl PUBLIC lxgui::lxgui)
if (NOT LXGUI_COMPILER_EMSCRIPTEN)
    target_link_libraries(lxgui-gui-sdl PUBLIC SDL2::SDL2)
    target_link_libraries(lxgui-gui-sdl PRIVATE SDL2::image)
    target_link_libraries(lxgui-gui-sdl PRIVATE SDL2::TTF)
else()
    target_compile_options(lxgui-gui-sdl PUBLIC "SHELL:-s USE_SDL=2")
    target_compile_options(lxgui-gui-sdl PUBLIC "SHELL:-s USE_SDL_IMAGE=2")
    target_compile_options(lxgui-gui-sdl PUBLIC "SHELL:-s SDL2_IMAGE_FORMATS='[\"png\"]'")
    target_compile_options(lxgui-gui-sdl PUBLIC "SHELL:-s USE_SDL_TTF=2")
    target_link_options(lxgui-gui-sdl PUBLIC "SHELL:-s USE_SDL=2")
    target_link_options(lxgui-gui-sdl PUBLIC "SHELL:-s USE_SDL_IMAGE=2")
    target_link_options(lxgui-gui-sdl PUBLIC "SHELL:-s SDL2_IMAGE_FORMATS='[\"png\"]'")
    target_link_options(lxgui-gui-sdl PUBLIC "SHELL:-s USE_SDL_TTF=2")
    target_link_options(lxgui-gui-sdl PUBLIC "SHELL:-s MIN_WEBGL_VERSION=1")
endif()

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/impl/gui_sdl*.hpp)
install(FILES ${files} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lxgui/impl)

list(APPEND LXGUI_INSTALL_TARGETS lxgui-gui-sdl)
set(LXGUI_INSTALL_TARGETS ${LXGUI_INSTALL_TARGETS} PARENT_SCOPE)
