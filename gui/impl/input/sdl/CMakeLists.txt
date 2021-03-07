set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/input/sdl)
set(SRCROOT ${TARGET_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LUA_INCLUDE_DIR})
include_directories(${SDL2_INCLUDE_DIRS})

add_library(lxgui-input-sdl STATIC
    ${SRCROOT}/input_sdl_source.cpp
)

install(FILES ${PROJECT_SOURCE_DIR}/include/lxgui/impl/input_sdl_source.hpp DESTINATION include/lxgui/impl)
install(TARGETS lxgui-input-sdl ARCHIVE DESTINATION lib)

