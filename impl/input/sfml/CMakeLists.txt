set(TARGET_DIR ${PROJECT_SOURCE_DIR}/impl/input/sfml)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-input-sfml
    ${SRCROOT}/input_sfml_source.cpp
)

add_library(lxgui::input::sfml ALIAS lxgui-input-sfml)

# need C++17
target_compile_features(lxgui-input-sfml PRIVATE cxx_std_17)
lxgui_set_warning_level(lxgui-input-sfml)
target_include_directories(lxgui-input-sfml PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# needed dependencies
target_link_libraries(lxgui-input-sfml PUBLIC lxgui::lxgui)
target_link_libraries(lxgui-input-sfml PUBLIC sfml::graphics)

install(FILES ${PROJECT_SOURCE_DIR}/include/lxgui/impl/input_sfml_source.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lxgui/impl)

list(APPEND LXGUI_INSTALL_TARGETS lxgui-input-sfml)
set(LXGUI_INSTALL_TARGETS ${LXGUI_INSTALL_TARGETS} PARENT_SCOPE)
