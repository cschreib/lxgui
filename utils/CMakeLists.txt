set(TARGET_DIR ${PROJECT_SOURCE_DIR}/utils)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-utils
    ${SRCROOT}/utils_exception.cpp
    ${SRCROOT}/utils_filesystem.cpp
    ${SRCROOT}/utils_string.cpp
)

# need C++17
target_compile_features(lxgui-utils PRIVATE cxx_std_17)
lxgui_set_warning_level(lxgui-utils)
target_include_directories(lxgui-utils PUBLIC ${PROJECT_SOURCE_DIR}/include)

# needed dependencies
target_compile_definitions(lxgui-utils PRIVATE -DUTF_CPP_CPLUSPLUS=201703L)
target_link_libraries(lxgui-utils PRIVATE utf8::cpp)
if(LXGUI_PLATFORM_LINUX)
    target_link_libraries(lxgui-utils PUBLIC m stdc++ stdc++fs)
endif()

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/utils*.hpp)
install(FILES ${files} DESTINATION include/lxgui)
install(TARGETS lxgui-utils ARCHIVE DESTINATION lib)

