set(TARGET_DIR ${PROJECT_SOURCE_DIR}/impl/gui/sdl)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-sdl
    ${SRCROOT}/gui_sdl.cpp
    ${SRCROOT}/gui_sdl_atlas.cpp
    ${SRCROOT}/gui_sdl_font.cpp
    ${SRCROOT}/gui_sdl_renderer.cpp
    ${SRCROOT}/gui_sdl_material.cpp
    ${SRCROOT}/gui_sdl_rendertarget.cpp
)

add_library(lxgui::gui::sdl ALIAS lxgui-sdl)

# need C++17
target_compile_features(lxgui-sdl PRIVATE cxx_std_17)
lxgui_set_warning_level(lxgui-sdl)
target_include_directories(lxgui-sdl PUBLIC ${PROJECT_SOURCE_DIR}/include)

# needed dependencies
target_link_libraries(lxgui-sdl PUBLIC lxgui::lxgui)
if (NOT LXGUI_COMPILER_EMSCRIPTEN)
    target_link_libraries(lxgui-sdl PUBLIC SDL2::SDL2)
    target_link_libraries(lxgui-sdl PRIVATE SDL2::image)
    target_link_libraries(lxgui-sdl PRIVATE SDL2::TTF)
else()
    set(LXGUI_SDL_EMSCRIPTEN_USE_FLAGS "${LXGUI_EMSCRIPTEN_FLAGS} -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]' -s USE_SDL_TTF=2")
    set_target_properties(lxgui-sdl PROPERTIES COMPILE_FLAGS ${LXGUI_SDL_EMSCRIPTEN_USE_FLAGS})
    set_target_properties(lxgui-sdl PROPERTIES LINK_FLAGS ${LXGUI_SDL_EMSCRIPTEN_USE_FLAGS})
endif()

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/impl/gui_sd*.hpp)
install(FILES ${files} DESTINATION include/lxgui/impl)
install(TARGETS lxgui-sdl ARCHIVE DESTINATION lib)
