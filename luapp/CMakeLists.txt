set(TARGET_DIR ${PROJECT_SOURCE_DIR}/luapp)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-luapp STATIC
    ${SRCROOT}/luapp_argument.cpp
    ${SRCROOT}/luapp_data.cpp
    ${SRCROOT}/luapp_function.cpp
    ${SRCROOT}/luapp_state.cpp
)

# Need C++17
target_compile_features(lxgui-luapp PRIVATE cxx_std_17)

target_include_directories(lxgui-luapp PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(lxgui-luapp PUBLIC ${LUA_INCLUDE_DIR})
target_include_directories(lxgui-luapp PUBLIC ${PROJECT_SOURCE_DIR}/sol2/include)

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/lu*.hpp)
install(FILES ${files} DESTINATION include/lxgui)
install(TARGETS lxgui-luapp ARCHIVE DESTINATION lib)

