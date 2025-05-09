cmake_minimum_required(VERSION 3.14..3.31)

# Start new project
project(lxgui-test-sdl-emscripten LANGUAGES CXX VERSION 2.0)

if(NOT TARGET lxgui::lxgui)
    # Find lxgui and dependencies
    find_package(lxgui 2)
endif()

# Set output directory to ../bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin)

# Create new executable
add_executable(lxgui-test-sdl-emscripten
    ${PROJECT_SOURCE_DIR}/main.cpp # main entry point
    ${PROJECT_SOURCE_DIR}/../common/examples_common.cpp) # common functions for all examples

# We need C++17
target_compile_features(lxgui-test-sdl-emscripten PRIVATE cxx_std_17)

# Include dirs (shared headers for all examples)
target_include_directories(lxgui-test-sdl-emscripten PRIVATE ${PROJECT_SOURCE_DIR}/../common)

# lxgui libraries
target_link_libraries(lxgui-test-sdl-emscripten PRIVATE
    lxgui::gui::sdl # SDL rendering implementation
    lxgui::input::sdl # SDL input implementation
    lxgui::lxgui) # core library

# Setup Emscripten-specific stuff
# Setup compilation options
target_link_options(lxgui-test-sdl-emscripten PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=1")
# Setup custom shell
target_link_options(lxgui-test-sdl-emscripten PRIVATE "SHELL:--shell-file ${PROJECT_SOURCE_DIR}/../common/emscripten_shell.html")
# Add "interface" folder to the preload-file list
target_link_options(lxgui-test-sdl-emscripten PRIVATE "SHELL:--preload-file ${PROJECT_SOURCE_DIR}/../../bin/interface@interface")
# Add "locale" folder to the preload-file list
target_link_options(lxgui-test-sdl-emscripten PRIVATE "SHELL:--preload-file ${PROJECT_SOURCE_DIR}/../../bin/locale@locale")

set(CMAKE_EXECUTABLE_SUFFIX .html)
