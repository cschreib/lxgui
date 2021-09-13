set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui/impl/input/sfml)
set(SRCROOT ${TARGET_DIR}/src)

add_library(lxgui-input-sfml
    ${SRCROOT}/input_sfml_source.cpp
)

add_library(lxgui::input::sfml ALIAS lxgui-input-sfml)

# need C++17
target_compile_features(lxgui-input-sfml PRIVATE cxx_std_17)
lxgui_set_warning_level(lxgui-input-sfml)
target_include_directories(lxgui-input-sfml PUBLIC ${PROJECT_SOURCE_DIR}/include)

# needed dependencies
target_link_libraries(lxgui-input-sfml PUBLIC lxgui::lxgui)
target_link_libraries(lxgui-input-sfml PUBLIC sfml::graphics)

install(FILES ${PROJECT_SOURCE_DIR}/include/lxgui/impl/input_sfml_source.hpp DESTINATION include/lxgui/impl)
install(TARGETS lxgui-input-sfml ARCHIVE DESTINATION lib)
