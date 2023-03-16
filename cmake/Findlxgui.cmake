# Locate the lxgui library
#
# Use LXGUI_ROOT to specify a manual path.
#
# This module defines the following variables:
# LXGUI_LIBRARIES, the list of all libraries required to compile against lxgui.
# LXGUI_INCLUDE_DIRS, the list of all include directories, including dependencies.
# LXGUI_FOUND, true if both the LXGUI_LIBRARY and LXGUI_INCLUDE_DIR have been found.
# LXGUI_VERSION_STRING, the version of the library that has been found.
# LXGUI_IMPL_INCLUDE_DIR, where to find implementation include files.
# LXGUI_GUI_GL_LIBRARIES, the list of all libraries required to use the OpenGL implementation library.
# LXGUI_GUI_GL_INCLUDE_DIRS, the list of all include directories required to use the OpenGL implementation library.
# LXGUI_GUI_GL_FOUND, true if both LXGUI_FOUND and LXGUI_GUI_GL_LIBRARY have been found.
# LXGUI_OPENGL3, True if the OpenGL implementation was built for OpenGL3, or False for legacy OpenGL.
# LXGUI_GUI_SFML_LIBRARIES, the list of all libraries required to use the SFML input implementation library.
# LXGUI_GUI_SFML_INCLUDE_DIRS, the list of all include directories required to use the SFML input implementation library.
# LXGUI_GUI_SFML_FOUND, true if both LXGUI_FOUND and LXGUI_INPUT_SFML_LIBRARY have been found.
# LXGUI_INPUT_SFML_LIBRARIES, the list of all libraries required to use the SFML input implementation library.
# LXGUI_INPUT_SFML_INCLUDE_DIRS, the list of all include directories required to use the SFML input implementation library.
# LXGUI_INPUT_SFML_FOUND, true if both LXGUI_FOUND and LXGUI_INPUT_SFML_LIBRARY have been found.
# LXGUI_GUI_SDL_LIBRARIES, the list of all libraries required to use the SDL input implementation library.
# LXGUI_GUI_SDL_INCLUDE_DIRS, the list of all include directories required to use the SDL input implementation library.
# LXGUI_GUI_SDL_FOUND, true if both LXGUI_FOUND and LXGUI_INPUT_SDL_LIBRARY have been found.
# LXGUI_INPUT_SDL_LIBRARIES, the list of all libraries required to use the SDL input implementation library.
# LXGUI_INPUT_SDL_INCLUDE_DIRS, the list of all include directories required to use the SDL input implementation library.
# LXGUI_INPUT_SDL_FOUND, true if both LXGUI_FOUND and LXGUI_INPUT_SDL_LIBRARY have been found.
#
# Finally, the module exports the following targets:
# lxgui::lxgui, the core library
# lxgui::gui::gl, the OpenGL implementation library
# lxgui::gui::sdl, the SDL implementation library
# lxgui::gui::sfml, the SFML implementation library
# lxgui::input::sdl, the SDL input implementation library
# lxgui::input::sfml, the SFML input implementation library

if(TARGET lxgui::lxgui)
    return()
endif()

find_path(LXGUI_INCLUDE_DIR
    NAMES lxgui/lxgui.hpp DOC "Path to lxgui include directory."
    PATH_SUFFIXES include
    HINTS ${LXGUI_ROOT} $ENV{LXGUI_ROOT}
    PATHS /usr /usr/local
)

find_library(LXGUI_LIBRARY
    NAMES lxgui lxgui.lib DOC "Absolute path to lxgui library."
    PATH_SUFFIXES lib
    HINTS ${LXGUI_ROOT} $ENV{LXGUI_ROOT}
    PATHS /usr /usr/local
)

set(LXGUI_LIBRARIES ${LXGUI_LIBRARY})

find_path(LXGUI_IMPL_INCLUDE_DIR
    NAMES lxgui/impl/gui_gl_renderer.hpp DOC "Path to lxgui implementation include directory."
    PATH_SUFFIXES include
    HINTS ${LXGUI_ROOT} $ENV{LXGUI_ROOT}
    PATHS /usr /usr/local
)

find_library(LXGUI_GUI_GL_LIBRARY
    NAMES lxgui-gl lxgui-gl.lib
    PATH_SUFFIXES lib
    HINTS ${LXGUI_ROOT} $ENV{LXGUI_ROOT}
    PATHS /usr /usr/local
)

find_library(LXGUI_GUI_SFML_LIBRARY
    NAMES lxgui-sfml lxgui-sfml.lib
    PATH_SUFFIXES lib
    HINTS ${LXGUI_ROOT} $ENV{LXGUI_ROOT}
    PATHS /usr /usr/local
)

find_library(LXGUI_INPUT_SFML_LIBRARY
    NAMES lxgui-input-sfml lxgui-input-sfml.lib
    PATH_SUFFIXES lib
    HINTS ${LXGUI_ROOT} $ENV{LXGUI_ROOT}
    PATHS /usr /usr/local
)

find_library(LXGUI_GUI_SDL_LIBRARY
    NAMES lxgui-sdl lxgui-sdl.lib
    PATH_SUFFIXES lib
    HINTS ${LXGUI_ROOT} $ENV{LXGUI_ROOT}
    PATHS /usr /usr/local
)

find_library(LXGUI_INPUT_SDL_LIBRARY
    NAMES lxgui-input-sdl lxgui-input-sdl.lib
    PATH_SUFFIXES lib
    HINTS ${LXGUI_ROOT} $ENV{LXGUI_ROOT}
    PATHS /usr /usr/local
)

mark_as_advanced(LXGUI_INCLUDE_DIR LXGUI_LIBRARY)
mark_as_advanced(LXGUI_IMPL_INCLUDE_DIR LXGUI_GUI_GL_LIBRARY LXGUI_GUI_SFML_LIBRARY LXGUI_INPUT_SFML_LIBRARY LXGUI_GUI_SDL_LIBRARY LXGUI_INPUT_SDL_LIBRARY)

if(LXGUI_INCLUDE_DIR AND EXISTS "${LXGUI_INCLUDE_DIR}/lxgui/lxgui.hpp")
    file(STRINGS "${LXGUI_INCLUDE_DIR}/lxgui/lxgui.hpp" lxgui_version_str
         REGEX "^#[\t ]*define[\t ]+LXGUI_VERSION_(MAJOR|MINOR)[\t ]+[0-9]+$")

    unset(LXGUI_VERSION_STRING)
    foreach(VPART MAJOR MINOR)
        foreach(VLINE ${lxgui_version_str})
            if(VLINE MATCHES "^#[\t ]*define[\t ]+LXGUI_VERSION_${VPART}")
                string(REGEX REPLACE "^#[\t ]*define[\t ]+LXGUI_VERSION_${VPART}[\t ]+([0-9]+)$" "\\1"
                       LXGUI_VERSION_PART "${VLINE}")
                if(LXGUI_VERSION_STRING)
                    set(LXGUI_VERSION_STRING "${LXGUI_VERSION_STRING}.${LXGUI_VERSION_PART}")
                else(LXGUI_VERSION_STRING)
                    set(LXGUI_VERSION_STRING "${LXGUI_VERSION_PART}")
                endif()
                unset(LXGUI_VERSION_PART)
            endif()
        endforeach()
    endforeach()

    unset(lxgui_version_str)

    file(STRINGS "${LXGUI_INCLUDE_DIR}/lxgui/lxgui.hpp" lxgui_opengl
         REGEX "^#[\t ]*define[\t ]+LXGUI_OPENGL3[\t ]*$")

    if(lxgui_opengl)
        set(LXGUI_OPENGL3 TRUE)
    else()
        set(LXGUI_OPENGL3 FALSE)
    endif()

    unset(lxgui_opengl)

    file(STRINGS "${LXGUI_INCLUDE_DIR}/lxgui/lxgui.hpp" lxgui_xml
         REGEX "^#[\t ]*define[\t ]+LXGUI_ENABLE_XML_PARSER[\t ]*$")
    if(lxgui_xml)
        message(STATUS "lxgui found with XML parser")
        set(LXGUI_ENABLE_XML_PARSER TRUE)
    else()
        set(LXGUI_ENABLE_XML_PARSER FALSE)
    endif()

    unset(lxgui_xml)

    file(STRINGS "${LXGUI_INCLUDE_DIR}/lxgui/lxgui.hpp" lxgui_yaml
         REGEX "^#[\t ]*define[\t ]+LXGUI_ENABLE_YAML_PARSER[\t ]*$")

    if(lxgui_yaml)
        message(STATUS "lxgui found with YAML parser")
        set(LXGUI_ENABLE_YAML_PARSER TRUE)
    else()
        set(LXGUI_ENABLE_YAML_PARSER FALSE)
    endif()

    unset(lxgui_yaml)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(lxgui
                                  REQUIRED_VARS LXGUI_LIBRARY LXGUI_INCLUDE_DIR
                                  VERSION_VAR LXGUI_VERSION_STRING)

find_package(Lua REQUIRED)
find_package(fmt REQUIRED)
find_package(oup REQUIRED)
find_package(sol2 REQUIRED)

if(LXGUI_ENABLE_XML_PARSER)
    find_package(pugixml REQUIRED)
endif()
if(LXGUI_ENABLE_YAML_PARSER)
    find_package(ryml REQUIRED)
endif()

set(LXGUI_GUI_GL_FOUND FALSE)
set(LXGUI_GUI_SFML_FOUND FALSE)
set(LXGUI_INPUT_SFML_FOUND FALSE)
set(LXGUI_GUI_SDL_FOUND FALSE)
set(LXGUI_INPUT_SDL_FOUND FALSE)

set(LXGUI_INCLUDE_DIRS ${LXGUI_INCLUDE_DIR})
set(LXGUI_LIBRARIES ${LXGUI_LIBRARY})

if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    set(LXGUI_EMSCRIPTEN TRUE)
else()
    set(LXGUI_EMSCRIPTEN FALSE)
endif()

if(UNIX AND NOT (APPLE OR LXGUI_EMSCRIPTEN))
    set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} stdc++fs)
endif()

if(LXGUI_FOUND)
    if(NOT TARGET lxgui::lxgui)
        add_library(lxgui::lxgui UNKNOWN IMPORTED)
        set_target_properties(lxgui::lxgui PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LXGUI_INCLUDE_DIRS}")
        set_target_properties(lxgui::lxgui PROPERTIES INTERFACE_LINK_LIBRARIES "${LXGUI_LIBRARIES}")
        set_target_properties(lxgui::lxgui PROPERTIES IMPORTED_LOCATION "${LXGUI_LIBRARY}")
        target_link_libraries(lxgui::lxgui INTERFACE Lua::Lua)
        target_link_libraries(lxgui::lxgui INTERFACE sol2::sol2)
        target_link_libraries(lxgui::lxgui INTERFACE fmt::fmt)
        target_link_libraries(lxgui::lxgui INTERFACE oup::oup)
        if(LXGUI_ENABLE_XML_PARSER)
            target_link_libraries(lxgui::lxgui INTERFACE pugixml::pugixml)
        endif()
        if(LXGUI_ENABLE_YAML_PARSER)
            target_link_libraries(lxgui::lxgui INTERFACE ryml::ryml)
        endif()
        if(LXGUI_EMSCRIPTEN)
            target_compile_options(lxgui::lxgui INTERFACE "SHELL:-s DISABLE_EXCEPTION_CATCHING=0")
            target_link_options(lxgui::lxgui INTERFACE "SHELL:-s DISABLE_EXCEPTION_CATCHING=0")
            target_link_options(lxgui::lxgui INTERFACE "SHELL:-s MAX_WEBGL_VERSION=3")
        endif()
    endif()

    if(LXGUI_GUI_GL_LIBRARY)
        if(NOT LXGUI_EMSCRIPTEN)
            cmake_policy(SET CMP0072 "NEW")
            find_package(Freetype)
            find_package(PNG)
            find_package(ZLIB)
            find_package(GLEW)
            find_package(OpenGL)

            if(OPENGL_FOUND AND GLEW_FOUND AND FREETYPE_FOUND AND PNG_FOUND AND ZLIB_FOUND)
                message(STATUS "Found lxgui-gl")
                set(LXGUI_GUI_GL_FOUND TRUE)

                set(LXGUI_GUI_GL_INCLUDE_DIRS ${LXGUI_IMPL_INCLUDE_DIR})
                set(LXGUI_GUI_GL_LIBRARIES ${LXGUI_GUI_GL_LIBRARY})

                mark_as_advanced(LXGUI_GUI_GL_FOUND)
            else()
                message(ERROR ": the OpenGL implementation of the GUI requires OpenGL, GLEW, freetype, libpng and zlib")
            endif()
        else()
            find_package(Freetype)

            if(FREETYPE_FOUND)
                message(STATUS "Found lxgui-gl")
                set(LXGUI_GUI_GL_FOUND TRUE)

                set(LXGUI_GUI_GL_INCLUDE_DIRS ${LXGUI_IMPL_INCLUDE_DIR})
                set(LXGUI_GUI_GL_LIBRARIES ${LXGUI_GUI_GL_LIBRARY})

                mark_as_advanced(LXGUI_GUI_GL_FOUND)
            else()
                message(ERROR ": the OpenGL implementation of the GUI requires freetype")
            endif()

        endif()

        if(LXGUI_GUI_GL_FOUND AND NOT TARGET lxgui::gui::gl)
            add_library(lxgui::gui::gl UNKNOWN IMPORTED)
            set_target_properties(lxgui::gui::gl PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LXGUI_GUI_GL_INCLUDE_DIRS}")
            set_target_properties(lxgui::gui::gl PROPERTIES INTERFACE_LINK_LIBRARIES "${LXGUI_GUI_GL_LIBRARIES}")
            set_target_properties(lxgui::gui::gl PROPERTIES IMPORTED_LOCATION "${LXGUI_GUI_GL_LIBRARY}")
            target_link_libraries(lxgui::gui::gl INTERFACE lxgui::lxgui)
            target_link_libraries(lxgui::gui::gl INTERFACE Freetype::Freetype)
            if(NOT LXGUI_EMSCRIPTEN)
                target_link_libraries(lxgui::gui::gl INTERFACE OpenGL::GL)
                target_link_libraries(lxgui::gui::gl INTERFACE PNG::PNG)
                target_link_libraries(lxgui::gui::gl INTERFACE GLEW::GLEW)
            else()
                target_compile_options(lxgui::gui::gl INTERFACE "SHELL:-s USE_LIBPNG=1")
                target_link_options(lxgui::gui::gl INTERFACE "SHELL:-s USE_LIBPNG=1")
                target_link_options(lxgui::gui::gl INTERFACE "SHELL:-s MIN_WEBGL_VERSION=2")
            endif()
        endif()
    endif()

    if(NOT LXGUI_EMSCRIPTEN)
        find_package(SFML 2 COMPONENTS graphics system window)
    endif()

    if(LXGUI_GUI_SFML_LIBRARY)
        if(NOT LXGUI_EMSCRIPTEN)
            if(SFML_FOUND)
                set(LXGUI_GUI_SFML_FOUND TRUE)
                message(STATUS "Found lxgui-sfml")

                set(LXGUI_GUI_SFML_INCLUDE_DIRS ${LXGUI_IMPL_INCLUDE_DIR})
                set(LXGUI_GUI_SFML_LIBRARIES ${LXGUI_GUI_SFML_LIBRARY})

                mark_as_advanced(LXGUI_GUI_SFML_FOUND)
            else()
                message(ERROR ": the SFML implementation of the GUI requires the SFML library")
            endif()
        else()
            unset(LXGUI_GUI_SFML_LIBRARY)
        endif()

        if(LXGUI_GUI_SFML_FOUND AND NOT TARGET lxgui::gui::sfml)
            add_library(lxgui::gui::sfml UNKNOWN IMPORTED)
            set_target_properties(lxgui::gui::sfml PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LXGUI_GUI_SFML_INCLUDE_DIRS}")
            set_target_properties(lxgui::gui::sfml PROPERTIES INTERFACE_LINK_LIBRARIES "${LXGUI_GUI_SFML_LIBRARIES}")
            set_target_properties(lxgui::gui::sfml PROPERTIES IMPORTED_LOCATION "${LXGUI_GUI_SFML_LIBRARY}")
            target_link_libraries(lxgui::gui::sfml INTERFACE lxgui::lxgui)
            target_link_libraries(lxgui::gui::sfml INTERFACE sfml::graphics)
        endif()
    endif()

    if(LXGUI_INPUT_SFML_LIBRARY)
        if(NOT LXGUI_EMSCRIPTEN)
            if(SFML_FOUND)
                set(LXGUI_INPUT_SFML_FOUND TRUE)
                message(STATUS "Found lxgui-input-sfml")

                set(LXGUI_INPUT_SFML_INCLUDE_DIRS ${LXGUI_IMPL_INCLUDE_DIR})
                set(LXGUI_INPUT_SFML_LIBRARIES ${LXGUI_INPUT_SFML_LIBRARY})

                mark_as_advanced(LXGUI_INPUT_SFML_FOUND)
            else()
                message(ERROR ": the SFML implementation of the input requires the SFML library")
            endif()
        else()
            unset(LXGUI_INPUT_SFML_LIBRARY)
        endif()

        if(LXGUI_INPUT_SFML_FOUND AND NOT TARGET lxgui::input::sfml)
            add_library(lxgui::input::sfml UNKNOWN IMPORTED)
            set_target_properties(lxgui::input::sfml PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LXGUI_INPUT_SFML_INCLUDE_DIRS}")
            set_target_properties(lxgui::input::sfml PROPERTIES INTERFACE_LINK_LIBRARIES "${LXGUI_INPUT_SFML_LIBRARIES}")
            set_target_properties(lxgui::input::sfml PROPERTIES IMPORTED_LOCATION "${LXGUI_INPUT_SFML_LIBRARY}")
            target_link_libraries(lxgui::input::sfml INTERFACE lxgui::lxgui)
            target_link_libraries(lxgui::input::sfml INTERFACE sfml::graphics)
        endif()
    endif()

    if(NOT LXGUI_EMSCRIPTEN)
        if((LXGUI_GUI_SDL_LIBRARY OR LXGUI_INPUT_SDL_LIBRARY))
            find_package(SDL2)
        endif()
    endif()

    if(LXGUI_GUI_SDL_LIBRARY)
        if(NOT LXGUI_EMSCRIPTEN)
            find_package(SDL2_image)
            find_package(SDL2_ttf)

            if(SDL2_FOUND AND SDL2_TTF_FOUND AND SDL2_IMAGE_FOUND)
                set(LXGUI_GUI_SDL_FOUND TRUE)
                message(STATUS "Found lxgui-sdl")

                set(LXGUI_GUI_SDL_INCLUDE_DIRS ${LXGUI_IMPL_INCLUDE_DIR})
                set(LXGUI_GUI_SDL_LIBRARIES ${LXGUI_GUI_SDL_LIBRARY})

                mark_as_advanced(LXGUI_GUI_SDL_FOUND)
            else()
                message(ERROR ": the SDL implementation of the GUI requires the SDL, SDL_ttf, and SDL_image libraries")
            endif()
        else()
            set(LXGUI_GUI_SDL_FOUND TRUE)
            message(STATUS "Found lxgui-sdl")

            set(LXGUI_GUI_SDL_INCLUDE_DIRS ${LXGUI_IMPL_INCLUDE_DIR})
            set(LXGUI_GUI_SDL_LIBRARIES ${LXGUI_GUI_SDL_LIBRARY})
        endif()

        if(LXGUI_GUI_SDL_FOUND AND NOT TARGET lxgui::gui::sdl)
            add_library(lxgui::gui::sdl UNKNOWN IMPORTED)
            set_target_properties(lxgui::gui::sdl PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LXGUI_GUI_SDL_INCLUDE_DIRS}")
            set_target_properties(lxgui::gui::sdl PROPERTIES INTERFACE_LINK_LIBRARIES "${LXGUI_GUI_SDL_LIBRARIES}")
            set_target_properties(lxgui::gui::sdl PROPERTIES IMPORTED_LOCATION "${LXGUI_GUI_SDL_LIBRARY}")
            target_link_libraries(lxgui::gui::sdl INTERFACE lxgui::lxgui)
            if(NOT LXGUI_EMSCRIPTEN)
                target_link_libraries(lxgui::gui::sdl INTERFACE SDL2::SDL2)
                target_link_libraries(lxgui::gui::sdl INTERFACE SDL2::image)
                target_link_libraries(lxgui::gui::sdl INTERFACE SDL2::TTF)
            else()
                target_compile_options(lxgui::gui::sdl INTERFACE "SHELL:-s USE_SDL=2")
                target_compile_options(lxgui::gui::sdl INTERFACE "SHELL:-s USE_SDL_IMAGE=2")
                target_compile_options(lxgui::gui::sdl INTERFACE "SHELL:-s SDL2_IMAGE_FORMATS='[\"png\"]'")
                target_compile_options(lxgui::gui::sdl INTERFACE "SHELL:-s USE_SDL_TTF=2")
                target_link_options(lxgui::gui::sdl INTERFACE "SHELL:-s USE_SDL=2")
                target_link_options(lxgui::gui::sdl INTERFACE "SHELL:-s USE_SDL_IMAGE=2")
                target_link_options(lxgui::gui::sdl INTERFACE "SHELL:-s SDL2_IMAGE_FORMATS='[\"png\"]'")
                target_link_options(lxgui::gui::sdl INTERFACE "SHELL:-s USE_SDL_TTF=2")
                target_link_options(lxgui::gui::sdl INTERFACE "SHELL:-s MIN_WEBGL_VERSION=1")
            endif()
        endif()
    endif()

    if(LXGUI_INPUT_SDL_LIBRARY)
        if(NOT LXGUI_EMSCRIPTEN)
            if(SDL2_FOUND)
                set(LXGUI_INPUT_SDL_FOUND TRUE)
                message(STATUS "Found lxgui-input-sdl")

                set(LXGUI_INPUT_SDL_INCLUDE_DIRS ${LXGUI_IMPL_INCLUDE_DIR})
                set(LXGUI_INPUT_SDL_LIBRARIES ${LXGUI_INPUT_SDL_LIBRARY})

                mark_as_advanced(LXGUI_INPUT_SDL_FOUND)
            else()
                message(ERROR ": the SDL implementation of the input requires the SDL library")
            endif()
        else()
            set(LXGUI_INPUT_SDL_FOUND TRUE)
            message(STATUS "Found lxgui-input-sdl")

            set(LXGUI_INPUT_SDL_INCLUDE_DIRS ${LXGUI_IMPL_INCLUDE_DIR})
            set(LXGUI_INPUT_SDL_LIBRARIES ${LXGUI_INPUT_SDL_LIBRARY})
        endif()

        if(LXGUI_INPUT_SDL_FOUND AND NOT TARGET lxgui::input::sdl)
            add_library(lxgui::input::sdl UNKNOWN IMPORTED)
            set_target_properties(lxgui::input::sdl PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LXGUI_INPUT_SDL_INCLUDE_DIRS}")
            set_target_properties(lxgui::input::sdl PROPERTIES INTERFACE_LINK_LIBRARIES "${LXGUI_INPUT_SDL_LIBRARIES}")
            set_target_properties(lxgui::input::sdl PROPERTIES IMPORTED_LOCATION "${LXGUI_INPUT_SDL_LIBRARY}")
            target_link_libraries(lxgui::input::sdl INTERFACE lxgui::lxgui)
            if(NOT LXGUI_EMSCRIPTEN)
                target_link_libraries(lxgui::input::sdl INTERFACE SDL2::SDL2)
                target_link_libraries(lxgui::input::sdl INTERFACE SDL2::image)
            else()
                target_compile_options(lxgui::input::sdl INTERFACE "SHELL:-s USE_SDL=2")
                target_compile_options(lxgui::input::sdl INTERFACE "SHELL:-s USE_SDL_IMAGE=2")
                target_compile_options(lxgui::input::sdl INTERFACE "SHELL:-s SDL2_IMAGE_FORMATS='[\"png\"]'")
                target_link_options(lxgui::input::sdl INTERFACE "SHELL:-s USE_SDL=2")
                target_link_options(lxgui::input::sdl INTERFACE "SHELL:-s USE_SDL_IMAGE=2")
                target_link_options(lxgui::input::sdl INTERFACE "SHELL:-s SDL2_IMAGE_FORMATS='[\"png\"]'")
            endif()
        endif()
    endif()
endif()

mark_as_advanced(LXGUI_OPENGL3 LXGUI_ENABLE_XML_PARSER LXGUI_ENABLE_YAML_PARSER
    LXGUI_GUI_GL_FOUND LXGUI_GUI_SFML_FOUND LXGUI_INPUT_SFML_FOUND LXGUI_GUI_SDL_FOUND LXGUI_INPUT_SDL_FOUND)

unset(LXGUI_EMSCRIPTEN)
