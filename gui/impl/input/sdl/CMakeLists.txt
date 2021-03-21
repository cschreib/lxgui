set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/input/sdl)
set(SRCROOT ${TARGET_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LUA_INCLUDE_DIR})
if (NOT EMSCRIPTEN)
    include_directories(${SDL2_INCLUDE_DIRS})
endif()

add_library(lxgui-input-sdl
    ${SRCROOT}/input_sdl_source.cpp
)

install(FILES ${PROJECT_SOURCE_DIR}/include/lxgui/impl/input_sdl_source.hpp DESTINATION include/lxgui/impl)
install(TARGETS lxgui-input-sdl ARCHIVE DESTINATION lib)

