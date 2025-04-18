set(TARGET_DIR ${PROJECT_SOURCE_DIR}/impl/gui/sfml)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-gui-sfml
    ${SRCROOT}/gui_sfml.cpp
    ${SRCROOT}/gui_sfml_atlas.cpp
    ${SRCROOT}/gui_sfml_font.cpp
    ${SRCROOT}/gui_sfml_renderer.cpp
    ${SRCROOT}/gui_sfml_material.cpp
    ${SRCROOT}/gui_sfml_render_target.cpp
    ${SRCROOT}/gui_sfml_vertex_cache.cpp
)

add_library(lxgui::gui::sfml ALIAS lxgui-gui-sfml)
set_target_properties(lxgui-gui-sfml PROPERTIES EXPORT_NAME gui::sfml)

# need C++17
target_compile_features(lxgui-gui-sfml PRIVATE cxx_std_17)
lxgui_set_warning_level(lxgui-gui-sfml)
target_include_directories(lxgui-gui-sfml PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# needed dependencies
target_link_libraries(lxgui-gui-sfml PUBLIC lxgui::lxgui)
target_link_libraries(lxgui-gui-sfml PUBLIC sfml-graphics)

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/impl/gui_sfml*.hpp)
install(FILES ${files} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lxgui/impl)

list(APPEND LXGUI_INSTALL_TARGETS lxgui-gui-sfml)
set(LXGUI_INSTALL_TARGETS ${LXGUI_INSTALL_TARGETS} PARENT_SCOPE)

