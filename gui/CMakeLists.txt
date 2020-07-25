set(TARGET_DIR ${PROJECT_SOURCE_DIR}/gui)
set(SRCROOT ${TARGET_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/sol2/include)
include_directories(${LUA_INCLUDE_DIR})

add_library(lxgui STATIC
    ${SRCROOT}/gui_anchor.cpp
    ${SRCROOT}/gui_backdrop.cpp
    ${SRCROOT}/gui_button.cpp
    ${SRCROOT}/gui_button_glues.cpp
    ${SRCROOT}/gui_button_parser.cpp
    ${SRCROOT}/gui_checkbutton.cpp
    ${SRCROOT}/gui_checkbutton_glues.cpp
    ${SRCROOT}/gui_checkbutton_parser.cpp
    ${SRCROOT}/gui_color.cpp
    ${SRCROOT}/gui_editbox.cpp
    ${SRCROOT}/gui_editbox_glues.cpp
    ${SRCROOT}/gui_editbox_parser.cpp
    ${SRCROOT}/gui_event.cpp
    ${SRCROOT}/gui_eventmanager.cpp
    ${SRCROOT}/gui_eventreceiver.cpp
    ${SRCROOT}/gui_focusframe.cpp
    ${SRCROOT}/gui_focusframe_glues.cpp
    ${SRCROOT}/gui_focusframe_parser.cpp
    ${SRCROOT}/gui_fontstring.cpp
    ${SRCROOT}/gui_fontstring_glues.cpp
    ${SRCROOT}/gui_fontstring_parser.cpp
    ${SRCROOT}/gui_frame.cpp
    ${SRCROOT}/gui_frame_glues.cpp
    ${SRCROOT}/gui_frame_parser.cpp
    ${SRCROOT}/gui_gradient.cpp
    ${SRCROOT}/gui_layeredregion.cpp
    ${SRCROOT}/gui_layeredregion_glues.cpp
    ${SRCROOT}/gui_layeredregion_parser.cpp
    ${SRCROOT}/gui_lunar.cpp
    ${SRCROOT}/gui_manager.cpp
    ${SRCROOT}/gui_manager_glues.cpp
    ${SRCROOT}/gui_manager_parser.cpp
    ${SRCROOT}/gui_out.cpp
    ${SRCROOT}/gui_region.cpp
    ${SRCROOT}/gui_region_parser.cpp
    ${SRCROOT}/gui_renderer_impl.cpp
    ${SRCROOT}/gui_scrollframe.cpp
    ${SRCROOT}/gui_scrollframe_glues.cpp
    ${SRCROOT}/gui_scrollframe_parser.cpp
    ${SRCROOT}/gui_slider.cpp
    ${SRCROOT}/gui_slider_glues.cpp
    ${SRCROOT}/gui_slider_parser.cpp
    ${SRCROOT}/gui_sprite.cpp
    ${SRCROOT}/gui_statusbar.cpp
    ${SRCROOT}/gui_statusbar_glues.cpp
    ${SRCROOT}/gui_statusbar_parser.cpp
    ${SRCROOT}/gui_text.cpp
    ${SRCROOT}/gui_texture.cpp
    ${SRCROOT}/gui_texture_glues.cpp
    ${SRCROOT}/gui_texture_parser.cpp
    ${SRCROOT}/gui_uiobject.cpp
    ${SRCROOT}/gui_uiobject_glues.cpp
    ${SRCROOT}/gui_uiobject_parser.cpp
    ${SRCROOT}/input.cpp
)

configure_file("${PROJECT_SOURCE_DIR}/include/lxgui/lxgui.hpp.config"
               "${CMAKE_SOURCE_DIR}/include/lxgui/lxgui.hpp")

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/gui*.hpp)
install(FILES ${files} DESTINATION include/lxgui)
file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/input*.hpp)
install(FILES ${files} DESTINATION include/lxgui)
file(GLOB_RECURSE files ${PROJECT_SOURCE_DIR}/sol2/include/sol/*)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/sol2/include/sol DESTINATION include)
install(FILES ${CMAKE_SOURCE_DIR}/include/lxgui/lxgui.hpp DESTINATION include/lxgui)
install(TARGETS lxgui ARCHIVE DESTINATION lib)

