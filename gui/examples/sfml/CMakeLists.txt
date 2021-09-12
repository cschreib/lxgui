cmake_minimum_required(VERSION 3.8)

# Start new project
project(lxgui-test-sfml CXX)

if (APPLE)
    add_definitions(-DMACOSX)
elseif (UNIX)
    add_definitions(-DLINUX)
elseif (WINDOWS)
    add_definitions(-DWIN32)
endif()

# Add the directory containing Findlxgui.cmake to the model path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/../../../cmake/Modules")

# Find lxgui and dependencies
find_package(lxgui 2)

# Set output directory to ../bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin)

# Create new executable
add_executable(lxgui-test-sfml
    ${PROJECT_SOURCE_DIR}/main.cpp # main entry point
    ${PROJECT_SOURCE_DIR}/../common/examples_common.cpp) # common functions for all examples

# We need C++17
target_compile_features(lxgui-test-sfml PRIVATE cxx_std_17)

# Include dirs (shared headers for all examples)
target_include_directories(lxgui-test-sfml PRIVATE ${PROJECT_SOURCE_DIR}/../common)

# lxgui libraries
target_link_libraries(lxgui-test-sfml PRIVATE
    lxgui::gui::sfml # SFML rendering implementation
    lxgui::input::sfml # SFML input implementation
    lxgui::lxgui) # core library

# Platform-specific stuff
if(WINDOWS AND MSVC)
    target_link_libraries(lxgui-test-sfml PRIVATE user32)
endif()
