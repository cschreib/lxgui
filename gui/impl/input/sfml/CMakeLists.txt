set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/input/sfml)
set(SRCROOT ${TARGET_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LUA_INCLUDE_DIR})
include_directories(${SFML_INCLUDE_DIR})

add_library(lxgui-input-sfml STATIC
    ${SRCROOT}/sfml_input_impl
)

install(FILES ${PROJECT_SOURCE_DIR}/include/lxgui/impl/sfml_input_impl.hpp DESTINATION include/lxgui/impl)
install(TARGETS lxgui-input-sfml ARCHIVE DESTINATION lib)

