cmake_minimum_required(VERSION 3.14..3.31)

# Start new project
project(lxgui-test-opengl-sdl LANGUAGES CXX VERSION 2.0)

# Add the directory containing Findlxgui.cmake to the model path
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../../cmake" ${CMAKE_MODULE_PATH})

# Find lxgui and dependencies
find_package(lxgui 2)

# Set output directory to ../bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin)

# Create new executable
add_executable(lxgui-test-opengl-sdl
    ${PROJECT_SOURCE_DIR}/main.cpp # main entry point
    ${PROJECT_SOURCE_DIR}/../common/examples_common.cpp) # common functions for all examples

# We need C++17
target_compile_features(lxgui-test-opengl-sdl PRIVATE cxx_std_17)

# Include dirs (shared headers for all examples)
target_include_directories(lxgui-test-opengl-sdl PRIVATE ${PROJECT_SOURCE_DIR}/../common)

# lxgui libraries
target_link_libraries(lxgui-test-opengl-sdl PRIVATE
    lxgui::gui::gl # OpenGL rendering implementation
    lxgui::input::sdl # SDL input implementation
    lxgui::lxgui) # core library

# Platform-specific stuff
if(LXGUI_PLATFORM_WINDOWS AND MSVC)
    target_link_libraries(lxgui-test-opengl-sdl PRIVATE user32)
endif()
