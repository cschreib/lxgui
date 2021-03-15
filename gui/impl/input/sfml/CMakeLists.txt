set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/input/sfml)
set(SRCROOT ${TARGET_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LUA_INCLUDE_DIR})
include_directories(${SFML_INCLUDE_DIR})

add_library(lxgui-input-sfml
    ${SRCROOT}/input_sfml_source
)

install(FILES ${PROJECT_SOURCE_DIR}/include/lxgui/impl/input_sfml_source.hpp DESTINATION include/lxgui/impl)
install(TARGETS lxgui-input-sfml ARCHIVE DESTINATION lib)

