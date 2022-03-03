cmake_minimum_required(VERSION 3.8)

# Start new project
project(lxgui-test-opengl-sdl-emscripten LANGUAGES CXX VERSION 2.0)

# Add the directory containing Findlxgui.cmake to the model path
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../../cmake" ${CMAKE_MODULE_PATH})

# Find lxgui and dependencies
find_package(lxgui 2)

# Set output directory to ../bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin)

# Create new executable
add_executable(lxgui-test-opengl-sdl-emscripten
    ${PROJECT_SOURCE_DIR}/main.cpp # main entry point
    ${PROJECT_SOURCE_DIR}/../common/examples_common.cpp) # common functions for all examples

# We need C++17
target_compile_features(lxgui-test-opengl-sdl-emscripten PRIVATE cxx_std_17)

# Include dirs (shared headers for all examples)
target_include_directories(lxgui-test-opengl-sdl-emscripten PRIVATE ${PROJECT_SOURCE_DIR}/../common)

# lxgui libraries
target_link_libraries(lxgui-test-opengl-sdl-emscripten PRIVATE
    lxgui::gui::gl # OpenGL rendering implementation
    lxgui::input::sdl # SDL input implementation
    lxgui::lxgui) # core library

# Setup Emscripten-specific stuff
# Setup compilation options
target_link_options(lxgui-test-opengl-sdl-emscripten PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=1")
# Setup custom shell
target_link_options(lxgui-test-opengl-sdl-emscripten PRIVATE "SHELL:--shell-file ${PROJECT_SOURCE_DIR}/../common/emscripten_shell.html")
# Add "interface" folder to the preload-file list
target_link_options(lxgui-test-opengl-sdl-emscripten PRIVATE "SHELL:--preload-file ${PROJECT_SOURCE_DIR}/../../bin/interface@interface")
# Add "locale" folder to the preload-file list
target_link_options(lxgui-test-opengl-sdl-emscripten PRIVATE "SHELL:--preload-file ${PROJECT_SOURCE_DIR}/../../bin/locale@locale")

set(CMAKE_EXECUTABLE_SUFFIX .html)
