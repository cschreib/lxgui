set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/gui/sdl)
set(SRCROOT ${TARGET_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${SDL2_INCLUDE_DIRS})

add_library(lxgui-sdl
    ${SRCROOT}/gui_sdl_font.cpp
    ${SRCROOT}/gui_sdl_renderer.cpp
    ${SRCROOT}/gui_sdl_material.cpp
    ${SRCROOT}/gui_sdl_rendertarget.cpp
    ${SRCROOT}/gui_sdl.cpp
)

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/impl/gui_sd*.hpp)
install(FILES ${files} DESTINATION include/lxgui/impl)
install(TARGETS lxgui-sdl ARCHIVE DESTINATION lib)

