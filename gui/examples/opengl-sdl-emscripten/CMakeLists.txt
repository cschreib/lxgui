cmake_minimum_required(VERSION 3.7)

# Start new project
project(lxgui-test-opengl-sdl-emscripten CXX)

# Add the directory containing Findlxgui.cmake to the model path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/../../../cmake/Modules")

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

# lxgui include dirs
target_include_directories(lxgui-test-opengl-sdl-emscripten PRIVATE
    ${PROJECT_SOURCE_DIR}/../common # shared headers for all examples
    ${LXGUI_INCLUDE_DIRS} # core library
    ${LXGUI_GUI_GL_INCLUDE_DIRS} # OpenGL rendering implementation
    ${LXGUI_INPUT_SDL_INCLUDE_DIRS}) # SDL input implementation

# lxgui libraries
target_link_libraries(lxgui-test-opengl-sdl-emscripten
    ${LXGUI_GUI_GL_LIBRARIES} # OpenGL rendering implementation
    ${LXGUI_INPUT_SDL_LIBRARIES} # SDL input implementation
    ${LXGUI_LIBRARIES}) # core library

# Setup Emscripten-specific stuff
# Setup compilation options
set(EMSCRIPTEN_USE_FLAGS "${LXGUI_EMSCRIPTEN_FLAGS}")
# Setup dependencies
set(EMSCRIPTEN_USE_FLAGS "${EMSCRIPTEN_USE_FLAGS} ${LXGUI_GUI_GL_LIBRARIES_EMSCRIPTEN} ${LXGUI_INPUT_SDL_LIBRARIES_EMSCRIPTEN}")
# Add "interface" folder to the preload-file list
set(EMSCRIPTEN_USE_FLAGS "${EMSCRIPTEN_USE_FLAGS} --preload-file ${PROJECT_SOURCE_DIR}/../../bin/interface@interface")
# Setup custom shell
set(EMSCRIPTEN_LINK_FLAGS "${EMSCRIPTEN_USE_FLAGS} --shell-file ${PROJECT_SOURCE_DIR}/../common/emscripten_shell.html")

set_target_properties(lxgui-test-opengl-sdl-emscripten PROPERTIES COMPILE_FLAGS ${EMSCRIPTEN_USE_FLAGS})
set_target_properties(lxgui-test-opengl-sdl-emscripten PROPERTIES LINK_FLAGS ${EMSCRIPTEN_LINK_FLAGS})

set(CMAKE_EXECUTABLE_SUFFIX .html)
