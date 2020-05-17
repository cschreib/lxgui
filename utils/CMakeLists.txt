set(TARGET_DIR ${PROJECT_SOURCE_DIR}/utils)
set(SRCROOT ${TARGET_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(lxgui-utils STATIC
    ${SRCROOT}/utils_exception.cpp
    ${SRCROOT}/utils_filesystem.cpp
    ${SRCROOT}/utils_string.cpp
    ${SRCROOT}/utils_any.cpp
)

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/utils*.hpp)
install(FILES ${files} DESTINATION include/lxgui)
install(TARGETS lxgui-utils ARCHIVE DESTINATION lib)

