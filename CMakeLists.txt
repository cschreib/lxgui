##############################################################################
# Base setup
##############################################################################
cmake_minimum_required(VERSION 3.14)
cmake_policy(SET CMP0048 "NEW")
cmake_policy(SET CMP0072 "NEW")
cmake_policy(SET CMP0077 "NEW")
cmake_policy(SET CMP0083 "NEW")
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

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
lxgui_set_option(LXGUI_OPENGL3 TRUE BOOL "Use OpenGL3 to build the OpenGL gui implementation")
lxgui_set_option(LXGUI_BUILD_FMT TRUE BOOL "Build the fmtlib dependency (if false, will search for it in the system)")
lxgui_set_option(LXGUI_BUILD_SOL2 TRUE BOOL "Build the sol2 dependency (if false, will search for it in the system)")
lxgui_set_option(LXGUI_BUILD_PUGIXML TRUE BOOL "Build the pugixml dependency (if false, will search for it in the system)")
lxgui_set_option(LXGUI_BUILD_RAPIDYAML TRUE BOOL "Build the rapidyaml dependency (if false, will search for it in the system)")
lxgui_set_option(LXGUI_ENABLE_XML_PARSER TRUE BOOL "Enable the XML layout parser (if false, XML layout files cannot be read)")
lxgui_set_option(LXGUI_ENABLE_YAML_PARSER TRUE BOOL "Enable the YAML layout parser (if false, YAML layout files cannot be read)")
lxgui_set_option(LXGUI_TEST_IMPLEMENTATION "SFML" STRING "Which implementation to test (SFML/SDL/OPENGL_SFML/OPENGL_SDL)")

# project name
project(lxgui LANGUAGES CXX VERSION 2.0)

##############################################################################
# Compiler and system checks
##############################################################################

# check compiler versions
if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    # Debugging options that can be added to LXGUI_EMSCRIPTEN_FLAGS:
    # -g -s SAFE_HEAP=1 -s WARN_UNALIGNED=1 -s ASSERTIONS=2
    # -g -fsanitize=address -s INITIAL_MEMORY=200MB
    set(LXGUI_EMSCRIPTEN_FLAGS "-s DISABLE_EXCEPTION_CATCHING=0 -s MAX_WEBGL_VERSION=3")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpfullversion -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)

    string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})
    list(GET GCC_VERSION_COMPONENTS 0 GCC_MAJOR)
    list(GET GCC_VERSION_COMPONENTS 1 GCC_MINOR)
    set(GCC_VERSION ${GCC_MAJOR}.${GCC_MINOR})

    if(GCC_VERSION VERSION_LESS 8.0)
        message(ERROR ": lxgui requires advanced features from the C++17 norm that are only available with gcc 8 or higher (your version: ${GCC_VERSION}). Please upgrade your compiler.")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(MSVC_VERSION VERSION_LESS 1910)
        message(ERROR ": lxgui requires advanced features from the C++17 norm that are only available with MSVC 2017 or higher (version 1910, your version: ${MSVC_VERSION}). Please upgrade your compiler.")
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11)
            message(ERROR ": lxgui requires advanced features from the C++17 norm that are only available with Xcode 11 or higher (your version: ${CMAKE_CXX_COMPILER_VERSION}). Please upgrade your compiler.")
        endif()
    else()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
            message(ERROR ": lxgui requires advanced features from the C++17 norm that are only available with clang 7 or higher (your version: ${CMAKE_CXX_COMPILER_VERSION}). Please upgrade your compiler.")
        endif()
    endif()
else()
    message(WARNING "Warning: your compiler has not been setup by the CMake script, do not expect it to work")
endif()

# define macro to enable relevant warnings for a target
macro(lxgui_set_warning_level target)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE -Wall)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${target} PRIVATE /W4)
        target_compile_options(${target} PRIVATE /GR)
        target_compile_options(${target} PRIVATE /EHs)
        target_compile_options(${target} PRIVATE /wd4267) # narrowing implicit conversion from size_t
        target_compile_options(${target} PRIVATE /wd4245) # signed/unsigned mismatch
        target_compile_options(${target} PRIVATE /wd4244) # narrowing implicit conversion
        target_compile_options(${target} PRIVATE /wd4100) # unused parameter
        target_compile_options(${target} PRIVATE /wd4121) # alignment sensitive to padding
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        target_compile_options(${target} PRIVATE -Weverything)
        target_compile_options(${target} PRIVATE -Wno-c++98-compat-pedantic)
        target_compile_options(${target} PRIVATE -Wno-c++98-compat)
        target_compile_options(${target} PRIVATE -Wno-unused-parameter)
        target_compile_options(${target} PRIVATE -Wno-sign-conversion)
        target_compile_options(${target} PRIVATE -Wno-signed-enum-bitfield)
        target_compile_options(${target} PRIVATE -Wno-conversion)
        target_compile_options(${target} PRIVATE -Wno-missing-variable-declarations)
        target_compile_options(${target} PRIVATE -Wno-missing-prototypes)
        target_compile_options(${target} PRIVATE -Wno-padded)
        target_compile_options(${target} PRIVATE -Wno-float-equal)
        target_compile_options(${target} PRIVATE -Wno-unused-variable)
        target_compile_options(${target} PRIVATE -Wno-global-constructors)
        target_compile_options(${target} PRIVATE -Wno-exit-time-destructors)
        target_compile_options(${target} PRIVATE -Wno-weak-vtables)
        target_compile_options(${target} PRIVATE -Wno-covered-switch-default)
        target_compile_options(${target} PRIVATE -Wno-documentation-unknown-command)
        target_compile_options(${target} PRIVATE -Wno-unneeded-internal-declaration)
        target_compile_options(${target} PRIVATE -Wno-unused-function)
        target_compile_options(${target} PRIVATE -Wno-unused-macros)
        target_compile_options(${target} PRIVATE -Wno-switch-enum)
        target_compile_options(${target} PRIVATE -Wno-zero-as-null-pointer-constant)
        target_compile_options(${target} PRIVATE -Wno-reserved-id-macro)
        target_compile_options(${target} PRIVATE -Wno-old-style-cast)
        target_compile_options(${target} PRIVATE -Wno-double-promotion)
        target_compile_options(${target} PRIVATE -Wno-unused-template)
        target_compile_options(${target} PRIVATE -Wno-used-but-marked-unused)
        target_compile_options(${target} PRIVATE -Wno-shadow-field-in-constructor)
        target_compile_options(${target} PRIVATE -Wno-disabled-macro-expansion)
        target_compile_options(${target} PRIVATE -Wno-extra-semi)
        target_compile_options(${target} PRIVATE -Wno-documentation)
        target_compile_options(${target} PRIVATE -Wno-undef)
        target_compile_options(${target} PRIVATE -Wno-newline-eof)
        target_compile_options(${target} PRIVATE -Wno-redundant-parens)
        target_compile_options(${target} PRIVATE -Wno-comma)
        target_compile_options(${target} PRIVATE -Wno-c++2a-compat)
        target_compile_options(${target} PRIVATE -Wno-missing-noreturn)
        target_compile_options(${target} PRIVATE -Wno-shadow-uncaptured-local)
        target_compile_options(${target} PRIVATE -Wno-shadow)
        target_compile_options(${target} PRIVATE -Wno-undefined-reinterpret-cast)
        target_compile_options(${target} PRIVATE -Wno-undefined-func-template)
        target_compile_options(${target} PRIVATE -fno-limit-debug-info)

        if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
            target_compile_options(${target} PRIVATE -Wno-poison-system-directories)
        endif()
    endif()
endmacro()

# set OS preprocessor defines
if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    set(LXGUI_PLATFORM_WASM TRUE)
    set(LXGUI_COMPILER_EMSCRIPTEN TRUE)
    if(NOT ${LXGUI_OPENGL3})
        message(WARNING "Warning: WebAssembly build does not support OpenGL version prior to 3")
        set(LXGUI_OPENGL3 TRUE)
    endif()
elseif (APPLE)
    set(LXGUI_PLATFORM_OSX TRUE)
    set(GL_SILENCE_DEPRECATION TRUE)
elseif (UNIX)
    set(LXGUI_PLATFORM_LINUX TRUE)
elseif (WIN32)
    set(LXGUI_PLATFORM_WINDOWS TRUE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(LXGUI_COMPILER_MSVC TRUE)
    endif()
endif()

if (NOT LXGUI_COMPILER_EMSCRIPTEN)
    set(LXGUI_COMPILER_EMSCRIPTEN FALSE)
endif()

##############################################################################
# Dependencies
##############################################################################

# configure dependencies
set(SDL2_BUILDING_LIBRARY TRUE)

# find dependencies
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/" ${CMAKE_MODULE_PATH})

find_package(Lua REQUIRED)
find_package(Freetype)

if(NOT LXGUI_COMPILER_EMSCRIPTEN)
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

# add the subdirectories for dependencies
set(UTF8_INSTALL 1)
add_subdirectory(extern/utfcpp)
add_subdirectory(extern/observable_unique_ptr)

if(LXGUI_BUILD_FMT)
    set(FMT_INSTALL TRUE)
    add_subdirectory(extern/fmt)
else()
    find_package(fmt REQUIRED)
endif()

if(LXGUI_ENABLE_XML_PARSER)
    if(LXGUI_BUILD_PUGIXML)
        add_subdirectory(extern/pugixml)
    else()
        find_package(pugixml)
        if(NOT TARGET pugixml::pugixml)
            message(ERROR ": the XML parser requires pugixml, which is missing. XML parsing will be disabled.")
            set(LXGUI_ENABLE_XML_PARSER FALSE)
        endif()
    endif()
endif()

if(LXGUI_ENABLE_YAML_PARSER)
    if(LXGUI_BUILD_RAPIDYAML)
        add_subdirectory(extern/rapidyaml)
    else()
        find_package(ryml)
        if(NOT TARGET ryml::ryml)
            message(ERROR ": the YAML parser requires rapidyaml, which is missing. YAML parsing will be disabled.")
            set(LXGUI_ENABLE_YAML_PARSER FALSE)
        endif()
    endif()
endif()

set(SOL_ALL_SAFETIES_ON TRUE)
set(SOL_PRINT_ERRORS FALSE)
if(LXGUI_BUILD_SOL2)
    add_subdirectory(extern/sol2)
else()
    find_package(sol2 REQUIRED)
endif()

##############################################################################
# Main library
##############################################################################

add_library(lxgui
    ${PROJECT_SOURCE_DIR}/src/gui_anchor.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_alive_checker.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_atlas.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_backdrop.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_button.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_button_glues.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_button_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_checkbutton.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_checkbutton_glues.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_checkbutton_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_color.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_editbox.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_editbox_glues.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_editbox_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_event.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_eventmanager.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_eventreceiver.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_focusframe.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_focusframe_glues.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_focusframe_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_fontstring.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_fontstring_glues.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_fontstring_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_frame.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_frame_glues.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_frame_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_framerenderer.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_frame_container.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_gradient.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_layeredregion.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_layeredregion_glues.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_layeredregion_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_localizer.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_localizer_glues.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_manager.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_manager_glues.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_manager_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_material.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_matrix4.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_out.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_parser_common.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_region.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_region_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_registry.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_renderer.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_scrollframe.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_scrollframe_glues.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_scrollframe_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_slider.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_slider_glues.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_slider_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_statusbar.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_statusbar_glues.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_statusbar_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_text.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_texture.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_texture_glues.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_texture_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_uiobject.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_uiobject_glues.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_uiobject_parser.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_uiroot.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_vertexcache.cpp
    ${PROJECT_SOURCE_DIR}/src/gui_virtual_registry.cpp
    ${PROJECT_SOURCE_DIR}/src/input.cpp
    ${PROJECT_SOURCE_DIR}/src/input_source.cpp
    ${PROJECT_SOURCE_DIR}/src/utils_exception.cpp
    ${PROJECT_SOURCE_DIR}/src/utils_filesystem.cpp
    ${PROJECT_SOURCE_DIR}/src/utils_maths.cpp
    ${PROJECT_SOURCE_DIR}/src/utils_string.cpp
)

add_library(lxgui::lxgui ALIAS lxgui)

# need C++17
target_compile_features(lxgui PUBLIC cxx_std_17)

# compilation options
lxgui_set_warning_level(lxgui)
target_include_directories(lxgui PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>)

# needed dependencies
target_link_libraries(lxgui PUBLIC Lua::Lua)
target_link_libraries(lxgui PUBLIC sol2::sol2)
target_link_libraries(lxgui PUBLIC fmt::fmt)
target_link_libraries(lxgui PUBLIC oup::oup)
target_compile_definitions(lxgui PRIVATE -DUTF_CPP_CPLUSPLUS=201703L)
target_link_libraries(lxgui PRIVATE utf8::cpp)
if(LXGUI_ENABLE_XML_PARSER)
    target_link_libraries(lxgui PRIVATE pugixml::pugixml)
endif()
if(LXGUI_ENABLE_YAML_PARSER)
    target_link_libraries(lxgui PRIVATE ryml::ryml)
endif()
if(LXGUI_PLATFORM_LINUX)
    target_link_libraries(lxgui PRIVATE m stdc++ stdc++fs)
endif()

configure_file("${PROJECT_SOURCE_DIR}/include/lxgui/lxgui.hpp.config"
               "${PROJECT_SOURCE_DIR}/include/lxgui/lxgui.hpp")

file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/gui*.hpp)
install(FILES ${files} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lxgui)
file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/input*.hpp)
install(FILES ${files} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lxgui)
file(GLOB files ${PROJECT_SOURCE_DIR}/include/lxgui/utils*.hpp)
install(FILES ${files} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lxgui)
install(FILES ${PROJECT_SOURCE_DIR}/include/lxgui/lxgui.hpp DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lxgui)

set(LXGUI_INSTALL_TARGETS lxgui)

##############################################################################
# Back-ends
##############################################################################

if(LXGUI_BUILD_GUI_GL_IMPL)
    if((OPENGL_FOUND AND GLEW_FOUND AND FREETYPE_FOUND AND PNG_FOUND AND ZLIB_FOUND) OR LXGUI_COMPILER_EMSCRIPTEN)
        add_subdirectory(impl/gui/gl)
    else()
        message(ERROR ": the OpenGL implementation of the GUI requires OpenGL, GLEW, freetype, libpng and zlib")
        set(LXGUI_BUILD_GUI_GL_IMPL FALSE)
    endif()
endif()
if(LXGUI_BUILD_GUI_SFML_IMPL)
    if(LXGUI_COMPILER_EMSCRIPTEN)
        message(ERROR ": the SFML implementation of the GUI is not supported with Emscripten")
        set(LXGUI_BUILD_GUI_SFML_IMPL FALSE)
    elseif(SFML_FOUND)
        add_subdirectory(impl/gui/sfml)
    else()
        message(ERROR ": the SFML implementation of the GUI requires the SFML library")
        set(LXGUI_BUILD_GUI_SFML_IMPL FALSE)
    endif()
endif()
if(LXGUI_BUILD_INPUT_SFML_IMPL)
    if(LXGUI_COMPILER_EMSCRIPTEN)
        message(ERROR ": the SFML implementation of the input is not supported with Emscripten")
        set(LXGUI_BUILD_INPUT_SFML_IMPL FALSE)
    elseif(SFML_FOUND)
        add_subdirectory(impl/input/sfml)
    else()
        message(ERROR ": the SFML implementation of the input requires the SFML library")
        set(LXGUI_BUILD_INPUT_SFML_IMPL FALSE)
    endif()
endif()
if(LXGUI_BUILD_GUI_SDL_IMPL)
    if((SDL2_FOUND AND SDL2_TTF_FOUND AND SDL2_IMAGE_FOUND) OR LXGUI_COMPILER_EMSCRIPTEN)
        add_subdirectory(impl/gui/sdl)
    else()
        message(ERROR ": the SDL implementation of the GUI requires the SDL2, SDL2_ttf, and SDL2_image libraries")
        set(LXGUI_BUILD_GUI_SDL_IMPL FALSE)
    endif()
endif()
if(LXGUI_BUILD_INPUT_SDL_IMPL)
    if(SDL2_FOUND OR LXGUI_COMPILER_EMSCRIPTEN)
        add_subdirectory(impl/input/sdl)
    else()
        message(ERROR ": the SDL implementation of the input requires the SDL library")
        set(LXGUI_BUILD_INPUT_SDL_IMPL FALSE)
    endif()
endif()

##############################################################################
# Config file
##############################################################################

install(TARGETS ${LXGUI_INSTALL_TARGETS}
  EXPORT lxgui-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib COMPONENT Library
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib COMPONENT Development
  INCLUDES DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

install(EXPORT lxgui-targets
  NAMESPACE lxgui::
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/lxgui COMPONENT Development)

export(EXPORT lxgui-targets
  NAMESPACE lxgui::)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/lxgui-config.cmake.in"
  "${PROJECT_BINARY_DIR}/lxgui-config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  NO_SET_AND_CHECK_MACRO)

write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/lxgui-config-version.cmake"
  COMPATIBILITY SameMajorVersion)

install(FILES
  "${PROJECT_BINARY_DIR}/lxgui-config.cmake"
  "${PROJECT_BINARY_DIR}/lxgui-config-version.cmake"
  "${PROJECT_SOURCE_DIR}/cmake/FindLua.cmake"
  "${PROJECT_SOURCE_DIR}/cmake/FindSDL2_image.cmake"
  "${PROJECT_SOURCE_DIR}/cmake/FindSDL2_ttf.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lxgui COMPONENT Development)

##############################################################################
# Test
##############################################################################

if(LXGUI_BUILD_TEST)
    if(LXGUI_COMPILER_EMSCRIPTEN AND NOT (LXGUI_TEST_IMPLEMENTATION STREQUAL "SDL" OR LXGUI_TEST_IMPLEMENTATION STREQUAL "OPENGL_SDL"))
        message(WARNING "Warning: the test program only supports SDL or OPENGL_SDL when compiled with Emscripten")
        set(LXGUI_TEST_IMPLEMENTATION "SDL")
    endif()
    if(LXGUI_TEST_IMPLEMENTATION STREQUAL "OPENGL_SFML")
        if(OPENGL_FOUND AND GLEW_FOUND AND FREETYPE_FOUND AND PNG_FOUND AND ZLIB_FOUND AND SFML_FOUND)
            add_subdirectory(test)
        else()
            message(ERROR ": the test program requires OpenGL, GLEW, freetype, libpng, zlib and SFML.")
        endif()
    elseif(LXGUI_TEST_IMPLEMENTATION STREQUAL "OPENGL_SDL")
        if((OPENGL_FOUND AND GLEW_FOUND AND FREETYPE_FOUND AND PNG_FOUND AND ZLIB_FOUND AND SDL2_FOUND) OR LXGUI_COMPILER_EMSCRIPTEN)
            add_subdirectory(test)
        else()
            message(ERROR ": the test program requires OpenGL, GLEW, freetype, libpng, zlib and SDL.")
        endif()
    elseif(LXGUI_TEST_IMPLEMENTATION STREQUAL "SDL")
        if((SDL2_FOUND AND SDL2_TTF_FOUND AND SDL2_IMAGE_FOUND) OR LXGUI_COMPILER_EMSCRIPTEN)
            add_subdirectory(test)
        else()
            message(ERROR ": the test program requires SDL2, SDL2_ttf, and SDL2_image.")
        endif()
    elseif(LXGUI_TEST_IMPLEMENTATION STREQUAL "SFML")
        if(SFML_FOUND)
            add_subdirectory(test)
        else()
            message(ERROR ": the test program requires SFML.")
        endif()
    else()
        message(ERROR ": unknown implementation ${LXGUI_TEST_IMPLEMENTATION}")
    endif()
endif()
