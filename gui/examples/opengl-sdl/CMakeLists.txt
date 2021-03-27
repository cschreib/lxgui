cmake_minimum_required(VERSION 3.7)

# Start new project
project(lxgui-test-opengl-sdl CXX)

# Add the directory containing Findlxgui.cmake to the model path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/../../../cmake/Modules")

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

# lxgui include dirs
target_include_directories(lxgui-test-opengl-sdl PRIVATE
    ${PROJECT_SOURCE_DIR}/../common # shared headers for all examples
    ${LXGUI_INCLUDE_DIRS} # core library
    ${LXGUI_GUI_GL_INCLUDE_DIRS} # OpenGL rendering implementation
    ${LXGUI_INPUT_SDL_INCLUDE_DIRS}) # SDL input implementation

# lxgui libraries
target_link_libraries(lxgui-test-opengl-sdl
    ${LXGUI_GUI_GL_LIBRARIES} # OpenGL rendering implementation
    ${LXGUI_INPUT_SDL_LIBRARIES} # SDL input implementation
    ${LXGUI_LIBRARIES}) # core library

# Platform-specific stuff
if(WINDOWS AND MSVC)
    target_link_libraries(lxgui-test-opengl-sdl user32)
endif()
if(UNIX AND NOT (APPLE OR EMSCRIPTEN))
    target_link_libraries(lxgui-test-opengl-sdl X11 m stdc++)
endif()
