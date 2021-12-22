cmake_minimum_required(VERSION 3.8)

project(lxgui-examples LANGUAGES CXX VERSION 2.0)

if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    add_subdirectory(opengl-sdl-emscripten)
    add_subdirectory(sdl-emscripten)
else()
    add_subdirectory(opengl-sdl)
    add_subdirectory(sdl)
    add_subdirectory(opengl-sfml)
    add_subdirectory(sfml)
endif()
