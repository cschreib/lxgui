set(TARGET_DIR ${PROJECT_SOURCE_DIR}/luapp)
set(SRCROOT ${TARGET_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${LUA_INCLUDE_DIR})

add_library(lxgui-luapp STATIC
    ${SRCROOT}/luapp_argument.cpp
    ${SRCROOT}/luapp_data.cpp
    ${SRCROOT}/luapp_function.cpp
    ${SRCROOT}/luapp_glues.cpp
    ${SRCROOT}/luapp_state.cpp
)

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/lu*.hpp)
install(FILES ${files} DESTINATION include/lxgui)
install(TARGETS lxgui-luapp ARCHIVE DESTINATION lib)

