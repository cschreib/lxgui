set(TARGET_DIR ${PROJECT_SOURCE_DIR}/luapp)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-luapp
    ${SRCROOT}/luapp_argument.cpp
    ${SRCROOT}/luapp_data.cpp
    ${SRCROOT}/luapp_function.cpp
    ${SRCROOT}/luapp_state.cpp
)

add_library(lxgui::luapp ALIAS lxgui-luapp)

# need C++17
target_compile_features(lxgui-luapp PRIVATE cxx_std_17)
lxgui_set_warning_level(lxgui-luapp)
target_include_directories(lxgui-luapp PUBLIC ${PROJECT_SOURCE_DIR}/include)

# needed dependencies
target_link_libraries(lxgui-luapp PUBLIC lxgui::utils)
target_link_libraries(lxgui-luapp PUBLIC Lua::Lua)
target_link_libraries(lxgui-luapp PUBLIC sol2::sol2)
target_link_libraries(lxgui-luapp PUBLIC fmt::fmt)

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/lu*.hpp)
install(FILES ${files} DESTINATION include/lxgui)
install(TARGETS lxgui-luapp ARCHIVE DESTINATION lib)

