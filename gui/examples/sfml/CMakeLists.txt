cmake_minimum_required(VERSION 3.7)

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

# lxgui include dirs
target_include_directories(lxgui-test-sfml PRIVATE
    ${PROJECT_SOURCE_DIR}/../common # shared headers for all examples
    ${LXGUI_INCLUDE_DIRS} # core library
    ${LXGUI_GUI_SFML_INCLUDE_DIRS} # SFML rendering implementation
    ${LXGUI_INPUT_SFML_INCLUDE_DIRS}) # SFML input implementation

# lxgui libraries
target_link_libraries(lxgui-test-sfml
    ${LXGUI_GUI_SFML_LIBRARIES} # SFML rendering implementation
    ${LXGUI_INPUT_SFML_LIBRARIES} # SFML input implementation
    ${LXGUI_LIBRARIES}) # core library

# Platform-specific stuff
if(WINDOWS AND MSVC)
    target_link_libraries(lxgui-test-sfml user32)
endif()
if(UNIX AND NOT (APPLE OR EMSCRIPTEN))
    target_link_libraries(lxgui-test-sfml X11 m stdc++)
endif()
