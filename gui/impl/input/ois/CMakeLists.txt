set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/input/ois)
set(SRCROOT ${TARGET_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LUA_INCLUDE_DIR})
include_directories(${OIS_INCLUDE_DIR})

add_library(lxgui-input-ois STATIC
    ${SRCROOT}/ois_input_impl
)

install(FILES ${PROJECT_SOURCE_DIR}/include/lxgui/impl/ois_input_impl.hpp DESTINATION include/lxgui/impl)
install(TARGETS lxgui-input-ois ARCHIVE DESTINATION lib)

