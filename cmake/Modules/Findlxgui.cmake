# Locate the lxgui library
# This module defines the following variables:
# LXGUI_LIBRARIES, the list of all libraries required to compile against lxgui.
# LXGUI_INCLUDE_DIRS, the list of all include directories, including dependencies.
# LXGUI_FOUND, true if both the LXGUI_LIBRARY and LXGUI_INCLUDE_DIR have been found.
# LXGUI_VERSION_STRING, the version of the library that has been found.
# LXGUI_IMPL_INCLUDE_DIR, where to find implementation include files.
# LXGUI_GUI_GL_LIBRARY, the name of the OpenGL implementation library.
# LXGUI_GUI_GL_FOUND, true if both LXGUI_FOUND and LXGUI_GUI_GL_LIBRARY have been found.
# LXGUI_INPUT_GLFW_LIBRARY, the name of the GLFW input implementation library.
# LXGUI_INPUT_GLFW_FOUND, true if both LXGUI_FOUND and LXGUI_INPUT_GLFW_LIBRARY have been found.
# LXGUI_INPUT_SFML_LIBRARY, the name of the SFML input implementation library.
# LXGUI_INPUT_SFML_FOUND, true if both LXGUI_FOUND and LXGUI_INPUT_SFML_LIBRARY have been found.
# LXGUI_INPUT_OIS_LIBRARY, the name of the OIS input implementation library.
# LXGUI_INPUT_OIS_FOUND, true if both LXGUI_FOUND and LXGUI_INPUT_OIS_LIBRARY have been found.
#

find_path(LXGUI_INCLUDE_DIR lxgui/lxgui.hpp DOC "Path to lxgui include directory."
    HINTS $ENV{LXGUI_DIR}
    PATH_SUFFIXES include
    PATHS /usr/include/ /usr/local/include/ ${LXGUI_DIR}/include/
)

find_library(LXGUI_LIBRARY DOC "Absolute path to lxgui library."
    NAMES lxgui lxgui.lib HINTS $ENV{LXGUI_DIR}
    PATH_SUFFIXES lib PATHS /usr/lib /usr/local/lib ${LXGUI_DIR}/lib
)

find_library(LXGUI_UTILS_LIBRARY
    NAMES lxgui-utils lxgui-utils.lib HINTS $ENV{LXGUI_DIR}
    PATH_SUFFIXES lib PATHS /usr/lib /usr/local/lib ${LXGUI_DIR}/lib
)

find_library(LXGUI_LUAPP_LIBRARY
    NAMES lxgui-luapp lxgui-luapp.lib HINTS $ENV{LXGUI_DIR}
    PATH_SUFFIXES lib PATHS /usr/lib /usr/local/lib ${LXGUI_DIR}/lib
)

find_library(LXGUI_XML_LIBRARY
    NAMES lxgui-xml lxgui-xml.lib HINTS $ENV{LXGUI_DIR}
    PATH_SUFFIXES lib PATHS /usr/lib /usr/local/lib ${LXGUI_DIR}/lib
)

set(LXGUI_LIBRARIES ${LXGUI_LIBRARY}${LXGUI_LUAPP_LIBRARY}${LXGUI_XML_LIBRARY}${LXGUI_UTILS_LIBRARY})

find_path(LXGUI_IMPL_INCLUDE_DIR gui_gl_manager.hpp DOC "Path to lxgui implementation include directory."
    HINTS $ENV{LXGUI_DIR}
    PATH_SUFFIX include/lxgui/impl PATHS /usr/include/lxgui/impl/ /usr/local/include/lxgui/impl/ ${LXGUI_DIR}/include/lxgui/impl/
)

find_library(LXGUI_GUI_GL_LIBRARY
    NAMES lxgui-gl lxgui-gl.lib HINTS $ENV{LXGUI_DIR}
    PATH_SUFFIXES lib PATHS /usr/lib /usr/local/lib ${LXGUI_DIR}/lib
)

find_library(LXGUI_GUI_SFML_LIBRARY
    NAMES lxgui-sfml lxgui-sfml.lib HINTS $ENV{LXGUI_DIR}
    PATH_SUFFIXES lib PATHS /usr/lib /usr/local/lib ${LXGUI_DIR}/lib
)

find_library(LXGUI_INPUT_GLFW_LIBRARY
    NAMES lxgui-input-glfw lxgui-input-glfw.lib HINTS $ENV{LXGUI_DIR}
    PATH_SUFFIXES lib PATHS /usr/lib /usr/local/lib ${LXGUI_DIR}/lib
)

find_library(LXGUI_INPUT_SFML_LIBRARY
    NAMES lxgui-input-sfml lxgui-input-sfml.lib HINTS $ENV{LXGUI_DIR}
    PATH_SUFFIXES lib PATHS /usr/lib /usr/local/lib ${LXGUI_DIR}/lib
)

find_library(LXGUI_INPUT_OIS_LIBRARY
    NAMES lxgui-input-ois lxgui-input-ois.lib HINTS $ENV{LXGUI_DIR}
    PATH_SUFFIXES lib PATHS /usr/lib /usr/local/lib ${LXGUI_DIR}/lib
)

mark_as_advanced(LXGUI_INCLUDE_DIR LXGUI_LIBRARY LXGUI_LUAPP_LIBRARY LXGUI_XML_LIBRARY LXGUI_UTILS_LIBRARY)
mark_as_advanced(LXGUI_IMPL_INCLUDE_DIR LXGUI_GUI_GL_LIBRARY LXGUI_GUI_SFML_LIBRARY LXGUI_INPUT_GLFW_LIBRARY LXGUI_INPUT_SFML_LIBRARY LXGUI_INPUT_OIS_LIBRARY)

if(LXGUI_INCLUDE_DIR AND EXISTS "${LXGUI_INCLUDE_DIR}/lxgui.hpp")
    file(STRINGS "${LXGUI_INCLUDE_DIR}/lxgui.hpp" lxgui_version_str
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
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LXGUI
                                  REQUIRED_VARS LXGUI_LIBRARY LXGUI_LUAPP_LIBRARY LXGUI_XML_LIBRARY LXGUI_UTILS_LIBRARY LXGUI_INCLUDE_DIR
                                  VERSION_VAR LXGUI_VERSION_STRING)

find_package(Lua REQUIRED)

set(LXGUI_GUI_GL_FOUND FALSE)
set(LXGUI_INPUT_GLFW_FOUND FALSE)
set(LXGUI_INPUT_SFML_FOUND FALSE)
set(LXGUI_INPUT_OIS_FOUND FALSE)

set(LXGUI_INCLUDE_DIRS ${LXGUI_INCLUDE_DIR} ${LUA_INCLUDE_DIR})
set(LXGUI_LIBRARIES ${LXGUI_LIBRARY} ${LXGUI_LUAPP_LIBRARY} ${LXGUI_XML_LIBRARY} ${LXGUI_UTILS_LIBRARY} ${LUA_LIBRARIES})

if(LXGUI_FOUND AND LXGUI_GUI_GL_LIBRARY)
    find_package(Freetype)
    find_package(PNG)
    find_package(ZLIB)
    find_package(GLEW)
    find_package(OpenGL)

    if(OPENGL_FOUND AND GLEW_FOUND AND FREETYPE_FOUND AND PNG_FOUND AND ZLIB_FOUND)
        message(STATUS "Found lxgui-gl")
        set(LXGUI_GUI_GL_FOUND TRUE)

        set(LXGUI_INCLUDE_DIRS ${LXGUI_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})
        set(LXGUI_INCLUDE_DIRS ${LXGUI_INCLUDE_DIRS} ${PNG_INCLUDE_DIR})
        set(LXGUI_INCLUDE_DIRS ${LXGUI_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIR})
        set(LXGUI_INCLUDE_DIRS ${LXGUI_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR})
        set(LXGUI_INCLUDE_DIRS ${LXGUI_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR})
        set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} ${LXGUI_GUI_GL_LIBRARY})
        set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} ${FREETYPE_LIBRARY})
        set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} ${PNG_LIBRARY})
        set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} ${ZLIB_LIBRARY})
        set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} ${GLEW_LIBRARY})
        set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} ${OPENGL_LIBRARY})
    else()
        message(ERROR ": the OpenGL implementation of the GUI requires OpenGL, GLEW, freetype, libpng and zlib")
    endif()
endif()

if(LXGUI_FOUND AND LXGUI_INPUT_GLFW_LIBRARY)
    find_package(GLFW 2 EXACT)

    if(GLFW_FOUND)
        message(STATUS "Found lxgui-input-glfw")
        set(LXGUI_INPUT_GLFW_FOUND TRUE)

        set(LXGUI_INCLUDE_DIRS ${LXGUI_INCLUDE_DIRS} ${GLFW_INCLUDE_DIR})
        set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} ${LXGUI_INPUT_GLFW_LIBRARY})
        set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} ${GLFW_LIBRARY})
    else()
        message(ERROR ": the GLFW implementation of the input requires the GLFW library")
    endif()
endif()

if(LXGUI_FOUND AND LXGUI_INPUT_SFML_LIBRARY)
    find_package(SFML 2 COMPONENTS system window)

    if(SFML_FOUND)
        set(LXGUI_INPUT_SFML_FOUND TRUE)
        message(STATUS "Found lxgui-input-sfml")

        set(LXGUI_INCLUDE_DIRS ${LXGUI_INCLUDE_DIRS} ${SFML_INCLUDE_DIR})
        set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} ${LXGUI_GUI_SFML_LIBRARY})
        set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} ${LXGUI_INPUT_SFML_LIBRARY})
        set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} ${SFML_WINDOW_LIBRARY})
        set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} ${SFML_SYSTEM_LIBRARY})
    else()
        message(ERROR ": the SFML implementation of the input requires the SFML library")
    endif()
endif()

if(LXGUI_FOUND AND LXGUI_INPUT_OIS_LIBRARY)
    find_package(OIS)

    if(OIS_FOUND)
        set(LXGUI_INPUT_OIS_FOUND TRUE)
        message(STATUS "Found lxgui-input-ois")

        set(LXGUI_INCLUDE_DIRS ${LXGUI_INCLUDE_DIRS} ${OIS_INCLUDE_DIR})
        set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} ${LXGUI_INPUT_OIS_LIBRARY})
        set(LXGUI_LIBRARIES ${LXGUI_LIBRARIES} ${OIS_LIBRARY})
    else()
        message(ERROR ": the OIS implementation of the input requires the OIS library")
    endif()
endif()

mark_as_advanced(LXGUI_INCLUDE_DIRS LXGUI_LIBRARIES)
