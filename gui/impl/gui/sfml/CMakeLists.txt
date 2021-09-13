set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/gui/sfml)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-sfml
    ${SRCROOT}/gui_sfml.cpp
    ${SRCROOT}/gui_sfml_atlas.cpp
    ${SRCROOT}/gui_sfml_font.cpp
    ${SRCROOT}/gui_sfml_renderer.cpp
    ${SRCROOT}/gui_sfml_material.cpp
    ${SRCROOT}/gui_sfml_rendertarget.cpp
    ${SRCROOT}/gui_sfml_vertexcache.cpp
)

add_library(lxgui::gui::sfml ALIAS lxgui-sfml)

# need C++17
target_compile_features(lxgui-sfml PRIVATE cxx_std_17)
lxgui_set_warning_level(lxgui-sfml)
target_include_directories(lxgui-sfml PUBLIC ${PROJECT_SOURCE_DIR}/include)

# needed dependencies
target_link_libraries(lxgui-sfml PUBLIC lxgui::lxgui)
target_link_libraries(lxgui-sfml PUBLIC sfml::graphics)

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/impl/gui_sfml*.hpp)
install(FILES ${files} DESTINATION include/lxgui/impl)
install(TARGETS lxgui-sfml ARCHIVE DESTINATION lib)
