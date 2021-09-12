set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/input/sdl)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-input-sdl
    ${SRCROOT}/input_sdl_source.cpp
)

# need C++17
target_compile_features(lxgui-input-sdl PRIVATE cxx_std_17)

target_include_directories(lxgui-input-sdl PUBLIC ${PROJECT_SOURCE_DIR}/include)

# needed dependencies
target_link_libraries(lxgui-input-sdl PUBLIC lxgui)
if (NOT EMSCRIPTEN)
    target_link_libraries(lxgui-input-sdl PUBLIC SDL2::SDL2)
    target_link_libraries(lxgui-input-sdl PRIVATE SDL2::image)
else()
    set(SDL_EMSCRIPTEN_USE_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]'")
    set_target_properties(lxgui-input-sdl PROPERTIES COMPILE_FLAGS ${SDL_EMSCRIPTEN_USE_FLAGS})
    set_target_properties(lxgui-input-sdl PROPERTIES LINK_FLAGS ${SDL_EMSCRIPTEN_USE_FLAGS})
endif()

install(FILES ${PROJECT_SOURCE_DIR}/include/lxgui/impl/input_sdl_source.hpp DESTINATION include/lxgui/impl)
install(TARGETS lxgui-input-sdl ARCHIVE DESTINATION lib)
