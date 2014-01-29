# Locate the lxgui library
# This module defines the following variables:
# LXGUI_LIBRARIES, the name of the library files that one has to link to.
# LXGUI_INCLUDE_DIR, where to find include files.
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

find_path(LXGUI_INCLUDE_DIR lxgui.hpp DOC "Path to lxgui include directory."
    HINTS $ENV{LXGUI_DIR}
    PATH_SUFFIX include/lxgui PATHS /usr/include/lxgui/ /usr/local/include/lxgui/ ${LXGUI_DIR}/include/lxgui/
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
mark_as_advanced(LXGUI_IMPL_INCLUDE_DIR LXGUI_GUI_GL_LIBRARY LXGUI_INPUT_GLFW_LIBRARY LXGUI_INPUT_SFML_LIBRARY LXGUI_INPUT_OIS_LIBRARY)

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

set(LXGUI_GUI_GL_FOUND FALSE)
set(LXGUI_INPUT_GLFW_FOUND FALSE)
set(LXGUI_INPUT_SFML_FOUND FALSE)
set(LXGUI_INPUT_OIS_FOUND FALSE)

if(LXGUI_FOUND AND LXGUI_GUI_GL_LIBRARY)
    set(LXGUI_GUI_GL_FOUND TRUE)
    message(STATUS "Found lxgui-gl")
endif()

if(LXGUI_FOUND AND LXGUI_INPUT_GLFW_LIBRARY)
    set(LXGUI_INPUT_GLFW_FOUND TRUE)
    message(STATUS "Found lxgui-input-glfw")
endif()

if(LXGUI_FOUND AND LXGUI_INPUT_SFML_LIBRARY)
    set(LXGUI_INPUT_SFML_FOUND TRUE)
    message(STATUS "Found lxgui-input-sfml")
endif()

if(LXGUI_FOUND AND LXGUI_INPUT_OIS_LIBRARY)
    set(LXGUI_INPUT_OIS_FOUND TRUE)
    message(STATUS "Found lxgui-input-ois")
endif()

