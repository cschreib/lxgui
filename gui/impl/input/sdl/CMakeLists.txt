set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/input/sdl)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-input-sdl
    ${SRCROOT}/input_sdl_source.cpp
)

target_include_directories(lxgui-input-sdl PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(lxgui-input-sdl PUBLIC ${LUA_INCLUDE_DIR})
if (NOT EMSCRIPTEN)
    target_include_directories(lxgui-input-sdl PUBLIC ${SDL2_INCLUDE_DIRS})
else()
    set(SDL_EMSCRIPTEN_USE_FLAGS "-s USE_SDL=2")
    set_target_properties(lxgui-input-sdl PROPERTIES COMPILE_FLAGS ${SDL_EMSCRIPTEN_USE_FLAGS})
    set_target_properties(lxgui-input-sdl PROPERTIES LINK_FLAGS ${SDL_EMSCRIPTEN_USE_FLAGS})
endif()

install(FILES ${PROJECT_SOURCE_DIR}/include/lxgui/impl/input_sdl_source.hpp DESTINATION include/lxgui/impl)
install(TARGETS lxgui-input-sdl ARCHIVE DESTINATION lib)

