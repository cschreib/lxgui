set(SRCROOT ${PROJECT_SOURCE_DIR}/test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(lxgui-test
    ${SRCROOT}/main.cpp
)

# need C++17
target_compile_features(lxgui-test PRIVATE cxx_std_17)
lxgui_set_warning_level(lxgui-test)
target_include_directories(lxgui-test PRIVATE ${PROJECT_SOURCE_DIR}/include)

if(LXGUI_COMPILER_EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX .html)
    target_link_options(lxgui-test PRIVATE "SHELL:--shell-file ${SRCROOT}/emscripten_shell.html")
    target_link_options(lxgui-test PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=1")
    target_link_options(lxgui-test PRIVATE "SHELL:--preload-file ${PROJECT_SOURCE_DIR}/bin/interface@interface")
    target_link_options(lxgui-test PRIVATE "SHELL:--preload-file ${PROJECT_SOURCE_DIR}/bin/locale@locale")
endif()

if(LXGUI_TEST_IMPLEMENTATION STREQUAL "OPENGL_SFML")
    target_compile_definitions(lxgui-test PRIVATE GLSFML_GUI)
elseif(LXGUI_TEST_IMPLEMENTATION STREQUAL "OPENGL_SDL")
    target_compile_definitions(lxgui-test PRIVATE GLSDL_GUI)
elseif(LXGUI_TEST_IMPLEMENTATION STREQUAL "SDL")
    target_compile_definitions(lxgui-test PRIVATE SDL_GUI)
elseif(LXGUI_TEST_IMPLEMENTATION STREQUAL "SFML")
    target_compile_definitions(lxgui-test PRIVATE SFML_GUI)
endif()

if(LXGUI_TEST_IMPLEMENTATION STREQUAL "OPENGL_SFML")
    target_link_libraries(lxgui-test PRIVATE lxgui::gui::gl)
    target_link_libraries(lxgui-test PRIVATE lxgui::input::sfml)
elseif(LXGUI_TEST_IMPLEMENTATION STREQUAL "OPENGL_SDL")
    target_link_libraries(lxgui-test PRIVATE lxgui::gui::gl)
    target_link_libraries(lxgui-test PRIVATE lxgui::input::sdl)
elseif(LXGUI_TEST_IMPLEMENTATION STREQUAL "SDL")
    target_link_libraries(lxgui-test PRIVATE lxgui::gui::sdl)
    target_link_libraries(lxgui-test PRIVATE lxgui::input::sdl)
elseif(LXGUI_TEST_IMPLEMENTATION STREQUAL "SFML")
    target_link_libraries(lxgui-test PRIVATE lxgui::gui::sfml)
    target_link_libraries(lxgui-test PRIVATE lxgui::input::sfml)
endif()

target_link_libraries(lxgui-test PRIVATE lxgui::lxgui)

if(LXGUI_PLATFORM_WINDOWS AND LXGUI_COMPILER_MSVC)
    target_link_libraries(lxgui-test PRIVATE user32)
endif()
