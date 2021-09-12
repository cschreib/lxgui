set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/gui/sdl)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-sdl
    ${SRCROOT}/gui_sdl.cpp
    ${SRCROOT}/gui_sdl_atlas.cpp
    ${SRCROOT}/gui_sdl_font.cpp
    ${SRCROOT}/gui_sdl_renderer.cpp
    ${SRCROOT}/gui_sdl_material.cpp
    ${SRCROOT}/gui_sdl_rendertarget.cpp
)

# need C++17
target_compile_features(lxgui-sdl PRIVATE cxx_std_17)

target_include_directories(lxgui-sdl PUBLIC ${PROJECT_SOURCE_DIR}/include)

# needed dependencies
target_link_libraries(lxgui-sdl PUBLIC lxgui)
if (NOT EMSCRIPTEN)
    target_link_libraries(lxgui-sdl PUBLIC SDL2::SDL2)
    target_link_libraries(lxgui-sdl PRIVATE SDL2::image)
    target_link_libraries(lxgui-sdl PRIVATE SDL2::TTF)
else()
    set(SDL_EMSCRIPTEN_USE_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]' -s USE_SDL_TTF=2")
    set_target_properties(lxgui-sdl PROPERTIES COMPILE_FLAGS ${SDL_EMSCRIPTEN_USE_FLAGS})
    set_target_properties(lxgui-sdl PROPERTIES LINK_FLAGS ${SDL_EMSCRIPTEN_USE_FLAGS})
endif()

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/impl/gui_sd*.hpp)
install(FILES ${files} DESTINATION include/lxgui/impl)
install(TARGETS lxgui-sdl ARCHIVE DESTINATION lib)
