cmake_minimum_required(VERSION 3.7)

# define a macro that helps defining an option
macro(lxgui_set_option var default type docstring)
    if(NOT DEFINED ${var})
        set(${var} ${default})
    endif()
    set(${var} ${${var}} CACHE ${type} ${docstring} FORCE)
endmacro()

# set defaults
# this has to be done before the project() instruction!
lxgui_set_option(CMAKE_BUILD_TYPE Release STRING "Choose the type of build (Debug or Release)")
lxgui_set_option(BUILD_SHARED_LIBS FALSE BOOL "Build lxgui as a shared library")
lxgui_set_option(LXGUI_BUILD_GUI_GL_IMPL TRUE BOOL "Build the OpenGL gui implementation")
lxgui_set_option(LXGUI_BUILD_GUI_SFML_IMPL TRUE BOOL "Build the SFML gui implementation")
lxgui_set_option(LXGUI_BUILD_GUI_SDL_IMPL TRUE BOOL "Build the SDL gui implementation")
lxgui_set_option(LXGUI_BUILD_INPUT_SFML_IMPL TRUE BOOL "Build the SFML input implementation")
lxgui_set_option(LXGUI_BUILD_INPUT_SDL_IMPL TRUE BOOL "Build the SDL input implementation")
lxgui_set_option(LXGUI_BUILD_TEST TRUE BOOL "Build the test program")
lxgui_set_option(LXGUI_BUILD_EXAMPLES FALSE BOOL "Build the example programs")
lxgui_set_option(LXGUI_OPENGL3 TRUE BOOL "Use OpenGL3 to build the OpenGL gui implementation")
lxgui_set_option(LXGUI_TEST_IMPLEMENTATION "SFML" STRING "Which implementation to test (SFML/SDL/OPENGL_SFML/OPENGL_SDL)")

# project name
project(lxgui)

# setup version numbers
set(VERSION_MAJOR 2)
set(VERSION_MINOR 0)

# check compiler version for C++17 features
if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    # Debugging options that can be added to EMSCRIPTEN_USE_FLAGS:
    # -g -s SAFE_HEAP=1 -s WARN_UNALIGNED=1 -s ASSERTIONS=2
    # -g -fsanitize=address -s INITIAL_MEMORY=200MB
    set(EMSCRIPTEN_USE_FLAGS "-s DISABLE_EXCEPTION_CATCHING=0 -s MAX_WEBGL_VERSION=3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_USE_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMSCRIPTEN_USE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMSCRIPTEN_USE_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX .html)

    set(EMSCRIPTEN 1)
elseif(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-Wall)

    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpfullversion -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)

    string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
    list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
    list(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)
    set(GCC_VERSION ${GCC_MAJOR}.${GCC_MINOR})

    if(GCC_VERSION VERSION_LESS 8.0)
        message(ERROR ": lxgui requires advanced features from the C++17 norm that are only available with gcc 8 or higher (your version: ${GCC_VERSION}). Please upgrade your compiler.")
    endif()
elseif(MSVC)
    add_definitions(/DMSVC)
    add_definitions(/W4)
    add_definitions(/GR)
    add_definitions(/EHs)
    add_definitions(/wd4267) # narrowing implicit conversion from size_t
    add_definitions(/wd4245) # signed/unsigned mismatch
    add_definitions(/wd4244) # narrowing implicit conversion
    add_definitions(/wd4100) # unused parameter
    add_definitions(/D_CRT_SECURE_NO_DEPRECATE)

    if(MSVC_VERSION VERSION_LESS 1910)
        message(ERROR ": lxgui requires advanced features from the C++17 norm that are only available with MSVC 2017 or higher (version 1910, your version: ${MSVC_VERSION}). Please upgrade your compiler.")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11)
            message(ERROR ": lxgui requires advanced features from the C++17 norm that are only available with Xcode 11 or higher (your version: ${CMAKE_CXX_COMPILER_VERSION}). Please upgrade your compiler.")
        endif()
    else()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
            message(ERROR ": lxgui requires advanced features from the C++17 norm that are only available with clang 7 or higher (your version: ${CMAKE_CXX_COMPILER_VERSION}). Please upgrade your compiler.")
        endif()
    endif()

    add_definitions(-Weverything)
    add_definitions(-Wno-c++98-compat-pedantic)
    add_definitions(-Wno-c++98-compat)
    add_definitions(-Wno-unused-parameter)
    add_definitions(-Wno-sign-conversion)
    add_definitions(-Wno-conversion)
    add_definitions(-Wno-missing-variable-declarations)
    add_definitions(-Wno-missing-prototypes)
    add_definitions(-Wno-padded)
    add_definitions(-Wno-float-equal)
    add_definitions(-Wno-unused-variable)
    add_definitions(-Wno-global-constructors)
    add_definitions(-Wno-exit-time-destructors)
    add_definitions(-Wno-weak-vtables)
    add_definitions(-Wno-covered-switch-default)
    add_definitions(-Wno-documentation-unknown-command)
    add_definitions(-Wno-unneeded-internal-declaration)
    add_definitions(-Wno-unused-function)
    add_definitions(-Wno-unused-macros)
    add_definitions(-Wno-switch-enum)
    add_definitions(-Wno-zero-as-null-pointer-constant)
    add_definitions(-Wno-reserved-id-macro)
    add_definitions(-Wno-old-style-cast)
    add_definitions(-Wno-double-promotion)
    add_definitions(-Wno-unused-template)
    add_definitions(-Wno-used-but-marked-unused)
    add_definitions(-Wno-shadow-field-in-constructor)
    add_definitions(-Wno-disabled-macro-expansion)
    add_definitions(-Wno-extra-semi)
    add_definitions(-Wno-documentation)
    add_definitions(-Wno-undef)
    add_definitions(-Wno-newline-eof)
    add_definitions(-Wno-redundant-parens)
    add_definitions(-Wno-comma)
    add_definitions(-Wno-c++2a-compat)
    add_definitions(-Wno-missing-noreturn)
    add_definitions(-Wno-shadow-uncaptured-local)
    add_definitions(-Wno-shadow)
    add_definitions(-Wno-undefined-reinterpret-cast)

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        add_definitions(-Wno-poison-system-directories)
        add_definitions(-DGL_SILENCE_DEPRECATION)
    endif()
else()
    message(WARNING "Warning: your compiler has not been setup by the CMake script, do not expect it to work")
endif()

# set OS preprocessor defines
if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    if(NOT ${LXGUI_OPENGL3})
        message(WARNING "Warning: WebAssembly build does not support OpenGL version prior to 3")
        set(LXGUI_OPENGL3 TRUE)
    endif()
    add_definitions(-DWASM)
elseif (APPLE)
    add_definitions(-DMACOSX)
elseif (UNIX)
    add_definitions(-DLINUX)
elseif (WINDOWS)
    add_definitions(-DWIN32)
endif()

# find libraries
cmake_policy(SET CMP0072 "NEW")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(Lua REQUIRED)
find_package(Freetype)
if(NOT EMSCRIPTEN)
find_package(PNG)
find_package(ZLIB)
find_package(SFML 2 COMPONENTS system window graphics)
find_package(GLEW)
find_package(OpenGL)
find_package(SDL2)
find_package(SDL2_image)
find_package(SDL2_ttf)
endif()
if (LUA_VERSION_STRING VERSION_GREATER_EQUAL 5.1 AND LUA_VERSION_STRING VERSION_LESS 5.2)
    set(LXGUI_LUA51 TRUE)
endif()

set(SOL_SAFE_NUMERICS TRUE)
set(SOL_PRINT_ERRORS FALSE)

# add the subdirectories
add_subdirectory(utils)
add_subdirectory(xml)
add_subdirectory(luapp)
add_subdirectory(gui)
if(LXGUI_BUILD_GUI_GL_IMPL)
    if((OPENGL_FOUND AND GLEW_FOUND AND FREETYPE_FOUND AND PNG_FOUND AND ZLIB_FOUND) OR EMSCRIPTEN)
        add_subdirectory(gui/impl/gui/gl)
    else()
        message(ERROR ": the OpenGL implementation of the GUI requires OpenGL, GLEW, freetype, libpng and zlib")
        set(LXGUI_BUILD_GUI_GL_IMPL FALSE)
    endif()
endif()
if(LXGUI_BUILD_GUI_SFML_IMPL)
    if(EMSCRIPTEN)
        message(ERROR ": the SFML implementation of the GUI is not supported with Emscripten")
        set(LXGUI_BUILD_GUI_SFML_IMPL FALSE)
    elseif(SFML_FOUND)
        add_subdirectory(gui/impl/gui/sfml)
    else()
        message(ERROR ": the SFML implementation of the GUI requires the SFML library")
        set(LXGUI_BUILD_GUI_SFML_IMPL FALSE)
    endif()
endif()
if(LXGUI_BUILD_INPUT_SFML_IMPL)
    if(EMSCRIPTEN)
        message(ERROR ": the SFML implementation of the input is not supported with Emscripten")
        set(LXGUI_BUILD_INPUT_SFML_IMPL FALSE)
    elseif(SFML_FOUND)
        add_subdirectory(gui/impl/input/sfml)
    else()
        message(ERROR ": the SFML implementation of the input requires the SFML library")
        set(LXGUI_BUILD_INPUT_SFML_IMPL FALSE)
    endif()
endif()
if(LXGUI_BUILD_GUI_SDL_IMPL)
    if((SDL2_FOUND AND SDL2_TTF_FOUND AND SDL2_IMAGE_FOUND) OR EMSCRIPTEN)
        add_subdirectory(gui/impl/gui/sdl)
    else()
        message(ERROR ": the SDL implementation of the GUI requires the SDL2, SDL2_ttf, and SDL2_image libraries")
        set(LXGUI_BUILD_GUI_SDL_IMPL FALSE)
    endif()
endif()
if(LXGUI_BUILD_INPUT_SDL_IMPL)
    if(SDL2_FOUND OR EMSCRIPTEN)
        add_subdirectory(gui/impl/input/sdl)
    else()
        message(ERROR ": the SDL implementation of the input requires the SDL library")
        set(LXGUI_BUILD_INPUT_SDL_IMPL FALSE)
    endif()
endif()
if(LXGUI_BUILD_TEST)
    if(EMSCRIPTEN AND NOT (LXGUI_TEST_IMPLEMENTATION STREQUAL "SDL" OR LXGUI_TEST_IMPLEMENTATION STREQUAL "OPENGL_SDL"))
        message(WARNING "Warning: the test program only supports SDL or OPENGL_SDL when compiled with Emscripten")
        set(LXGUI_TEST_IMPLEMENTATION "SDL")
    endif()
    if (LXGUI_TEST_IMPLEMENTATION STREQUAL "OPENGL_SFML")
        if(OPENGL_FOUND AND GLEW_FOUND AND FREETYPE_FOUND AND PNG_FOUND AND ZLIB_FOUND AND SFML_FOUND)
            add_subdirectory(gui/test)
        else()
            message(ERROR ": the test program requires OpenGL, GLEW, freetype, libpng, zlib and SFML.")
        endif()
    elseif (LXGUI_TEST_IMPLEMENTATION STREQUAL "OPENGL_SDL")
        if((OPENGL_FOUND AND GLEW_FOUND AND FREETYPE_FOUND AND PNG_FOUND AND ZLIB_FOUND AND SDL2_FOUND) OR EMSCRIPTEN)
            add_subdirectory(gui/test)
        else()
            message(ERROR ": the test program requires OpenGL, GLEW, freetype, libpng, zlib and SDL.")
        endif()
    elseif(LXGUI_TEST_IMPLEMENTATION STREQUAL "SDL")
        if((SDL2_FOUND AND SDL2_TTF_FOUND AND SDL2_IMAGE_FOUND) OR EMSCRIPTEN)
            add_subdirectory(gui/test)
        else()
            message(ERROR ": the test program requires SDL2, SDL2_ttf, and SDL2_image.")
        endif()
    elseif(LXGUI_TEST_IMPLEMENTATION STREQUAL "SFML")
        if(SFML_FOUND)
            add_subdirectory(gui/test)
        else()
            message(ERROR ": the test program requires SFML.")
        endif()
    else()
        message(ERROR ": unknown implementation ${LXGUI_TEST_IMPLEMENTATION}")
    endif()
endif()

if(LXGUI_BUILD_EXAMPLES)
    add_subdirectory(gui/examples/opengl-sdl)
    add_subdirectory(gui/examples/sdl)
    add_subdirectory(gui/examples/opengl-sfml)
    add_subdirectory(gui/examples/sfml)
endif()
