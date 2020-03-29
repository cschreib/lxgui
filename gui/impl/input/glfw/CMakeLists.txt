set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/input/glfw)
set(SRCROOT ${TARGET_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LUA_INCLUDE_DIR})
include_directories(${GLFW_INCLUDE_DIR})

add_library(lxgui-input-glfw STATIC
    ${SRCROOT}/input_glfw_source
)

install(FILES ${PROJECT_SOURCE_DIR}/include/lxgui/impl/input_glfw_source.hpp DESTINATION include/lxgui/impl)
install(TARGETS lxgui-input-glfw ARCHIVE DESTINATION lib)

