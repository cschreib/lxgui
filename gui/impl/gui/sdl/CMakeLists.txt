set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/gui/sdl)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-sdl
    ${SRCROOT}/gui_sdl_font.cpp
    ${SRCROOT}/gui_sdl_renderer.cpp
    ${SRCROOT}/gui_sdl_material.cpp
    ${SRCROOT}/gui_sdl_rendertarget.cpp
    ${SRCROOT}/gui_sdl.cpp
)

target_include_directories(lxgui-sdl PUBLIC ${PROJECT_SOURCE_DIR}/include)
if (NOT EMSCRIPTEN)
    target_include_directories(lxgui-sdl PUBLIC ${SDL2_INCLUDE_DIRS})
else()
    set(SDL_EMSCRIPTEN_USE_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]' -s USE_SDL_TTF=2")
    set_target_properties(lxgui-sdl PROPERTIES COMPILE_FLAGS ${SDL_EMSCRIPTEN_USE_FLAGS})
    set_target_properties(lxgui-sdl PROPERTIES LINK_FLAGS ${SDL_EMSCRIPTEN_USE_FLAGS})
endif()

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/impl/gui_sd*.hpp)
install(FILES ${files} DESTINATION include/lxgui/impl)
install(TARGETS lxgui-sdl ARCHIVE DESTINATION lib)

