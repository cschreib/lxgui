set(TARGET_DIR ${PROJECT_SOURCE_DIR}/utils)
set(SRCROOT ${TARGET_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/utfcpp/source)

add_library(lxgui-utils STATIC
    ${SRCROOT}/utils_exception.cpp
    ${SRCROOT}/utils_filesystem.cpp
    ${SRCROOT}/utils_string.cpp
)

# Need C++17
target_compile_features(lxgui-utils PRIVATE cxx_std_17)

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/utils*.hpp)
install(FILES ${files} DESTINATION include/lxgui)
install(TARGETS lxgui-utils ARCHIVE DESTINATION lib)

