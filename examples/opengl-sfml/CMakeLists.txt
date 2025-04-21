cmake_minimum_required(VERSION 3.14..3.31)

# Start new project
project(lxgui-test-opengl-sfml LANGUAGES CXX VERSION 2.0)

if(NOT TARGET lxgui::lxgui)
    # Find lxgui and dependencies
    find_package(lxgui 2)
endif()

# Set output directory to ../bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin)

# Create new executable
add_executable(lxgui-test-opengl-sfml
    ${PROJECT_SOURCE_DIR}/main.cpp # main entry point
    ${PROJECT_SOURCE_DIR}/../common/examples_common.cpp) # common functions for all examples

# We need C++17
target_compile_features(lxgui-test-opengl-sfml PRIVATE cxx_std_17)

# Include dirs (shared headers for all examples)
target_include_directories(lxgui-test-opengl-sfml PRIVATE ${PROJECT_SOURCE_DIR}/../common)

# lxgui libraries
target_link_libraries(lxgui-test-opengl-sfml PRIVATE
    lxgui::gui::gl # OpenGL rendering implementation
    lxgui::input::sfml # SFML input implementation
    lxgui::lxgui) # core library

# Platform-specific stuff
if(LXGUI_PLATFORM_WINDOWS AND MSVC)
    target_link_libraries(lxgui-test-opengl-sfml PRIVATE user32)
endif()
