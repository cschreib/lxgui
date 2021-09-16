cmake_minimum_required(VERSION 3.8)

# Start new project
project(lxgui-test-sdl-emscripten LANGUAGES CXX VERSION 2.0)

# Add the directory containing Findlxgui.cmake to the model path
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../../../cmake" ${CMAKE_MODULE_PATH})

# Find lxgui and dependencies
find_package(lxgui 2)

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
set(EMSCRIPTEN_USE_FLAGS "${LXGUI_EMSCRIPTEN_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
# Setup dependencies
set(EMSCRIPTEN_USE_FLAGS "${EMSCRIPTEN_USE_FLAGS} ${LXGUI_GUI_SDL_LIBRARIES_EMSCRIPTEN} ${LXGUI_INPUT_SDL_LIBRARIES_EMSCRIPTEN}")
# Add "interface" folder to the preload-file list
set(EMSCRIPTEN_USE_FLAGS "${EMSCRIPTEN_USE_FLAGS} --preload-file ${PROJECT_SOURCE_DIR}/../../bin/interface@interface")
# Setup custom shell
set(EMSCRIPTEN_LINK_FLAGS "${EMSCRIPTEN_USE_FLAGS} --shell-file ${PROJECT_SOURCE_DIR}/../common/emscripten_shell.html")

set_target_properties(lxgui-test-sdl-emscripten PROPERTIES COMPILE_FLAGS ${EMSCRIPTEN_USE_FLAGS})
set_target_properties(lxgui-test-sdl-emscripten PROPERTIES LINK_FLAGS ${EMSCRIPTEN_LINK_FLAGS})

set(CMAKE_EXECUTABLE_SUFFIX .html)
