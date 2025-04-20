set(TARGET_DIR ${PROJECT_SOURCE_DIR}/impl/input/sdl)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-input-sdl
    ${SRCROOT}/input_sdl_source.cpp
)

add_library(lxgui::input::sdl ALIAS lxgui-input-sdl)
set_target_properties(lxgui-input-sdl PROPERTIES EXPORT_NAME input::sdl)

# need C++17
target_compile_features(lxgui-input-sdl PRIVATE cxx_std_17)
lxgui_set_warning_level(lxgui-input-sdl)
target_include_directories(lxgui-input-sdl PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# needed dependencies
target_link_libraries(lxgui-input-sdl PUBLIC lxgui::lxgui)
if (NOT LXGUI_COMPILER_EMSCRIPTEN)
    target_link_libraries(lxgui-input-sdl PUBLIC SDL2::SDL2)
    target_link_libraries(lxgui-input-sdl PRIVATE SDL2_image::SDL2_image)
else()
    target_compile_options(lxgui-input-sdl PUBLIC "SHELL:-s USE_SDL=2")
    target_compile_options(lxgui-input-sdl PUBLIC "SHELL:-s USE_SDL_IMAGE=2")
    target_compile_options(lxgui-input-sdl PUBLIC "SHELL:-s SDL2_IMAGE_FORMATS='[\"png\"]'")
    target_link_options(lxgui-input-sdl PUBLIC "SHELL:-s USE_SDL=2")
    target_link_options(lxgui-input-sdl PUBLIC "SHELL:-s USE_SDL_IMAGE=2")
    target_link_options(lxgui-input-sdl PUBLIC "SHELL:-s SDL2_IMAGE_FORMATS='[\"png\"]'")
endif()

install(FILES ${PROJECT_SOURCE_DIR}/include/lxgui/impl/input_sdl_source.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lxgui/impl)

list(APPEND LXGUI_INSTALL_TARGETS lxgui-input-sdl)
set(LXGUI_INSTALL_TARGETS ${LXGUI_INSTALL_TARGETS} PARENT_SCOPE)
