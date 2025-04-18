cmake_minimum_required(VERSION 3.14..3.31)

# Start new project
project(lxgui-test-sdl LANGUAGES CXX VERSION 2.0)

if(NOT TARGET lxgui::lxgui)
    # Find lxgui and dependencies
    find_package(lxgui 2)
endif()

# Set output directory to ../bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin)

# Create new executable
add_executable(lxgui-test-sdl
    ${PROJECT_SOURCE_DIR}/main.cpp # main entry point
    ${PROJECT_SOURCE_DIR}/../common/examples_common.cpp) # common functions for all examples

# We need C++17
target_compile_features(lxgui-test-sdl PRIVATE cxx_std_17)

# Include dirs (shared headers for all examples)
target_include_directories(lxgui-test-sdl PRIVATE ${PROJECT_SOURCE_DIR}/../common)

# lxgui libraries
target_link_libraries(lxgui-test-sdl
    lxgui::gui::sdl # SDL rendering implementation
    lxgui::input::sdl # SDL input implementation
    lxgui::lxgui) # core library

# Platform-specific stuff
if(LXGUI_PLATFORM_WINDOWS AND MSVC)
    target_link_libraries(lxgui-test-sdl PRIVATE user32)
endif()
