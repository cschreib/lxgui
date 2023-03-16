# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindSDL2
-------

Locate SDL2 library

This module defines

::

  SDL2_LIBRARIES, the name of the library to link against
  SDL2_FOUND, if false, do not try to link to SDL
  SDL2_INCLUDE_DIRS, where to find SDL.h
  SDL2_VERSION_STRING, human-readable string containing the version of SDL



This module responds to the flag:

::

  SDL2_BUILDING_LIBRARY
    If this is defined, then no SDL2_main will be linked in because
    only applications need main().
    Otherwise, it is assumed you are building an application and this
    module will attempt to locate and set the proper link flags
    as part of the returned SDL2_LIBRARIES variable.



Don't forget to include SDLmain.h and SDLmain.m your project for the
OS X framework based version.  (Other versions link to -lSDLmain which
this module will try to find on your behalf.) Also for OS X, this
module will automatically add the -framework Cocoa on your behalf.



Additional Note: If you see an empty SDL2_LIBRARIES_TEMP in your
configuration and no SDL2_LIBRARIES, it means CMake did not find your SDL
library (SDL.dll, libsdl.so, SDL.framework, etc).  Set
SDL2_LIBRARIES_TEMP to point to your SDL library, and configure again.
Similarly, if you see an empty SDL2MAIN_LIBRARY, you should set this
value as appropriate.  These values are used to generate the final
SDL2_LIBRARIES variable, but when these values are unset, SDL2_LIBRARIES
does not get created.



$SDL2DIR is an environment variable that would correspond to the
./configure --prefix=$SDL2DIR used in building SDL.  l.e.galup 9-20-02

Modified by Eric Wing.  Added code to assist with automated building
by using environmental variables and providing a more
controlled/consistent search behavior.  Added new modifications to
recognize OS X frameworks and additional Unix paths (FreeBSD, etc).
Also corrected the header search path to follow "proper" SDL
guidelines.  Added a search for SDLmain which is needed by some
platforms.  Added a search for threads which is needed by some
platforms.  Added needed compile switches for MinGW.

On OSX, this will prefer the Framework version (if found) over others.
People will have to manually change the cache values of SDL2_LIBRARIES to
override this selection or set the CMake environment
CMAKE_INCLUDE_PATH to modify the search paths.

Note that the header path has changed from SDL/SDL.h to just SDL.h
This needed to change because "proper" SDL convention is #include
"SDL.h", not <SDL/SDL.h>.  This is done for portability reasons
because not all systems place things in SDL/ (see FreeBSD).
#]=======================================================================]

find_path(SDL2_INCLUDE_DIRS SDL.h
  HINTS
    ENV SDL2DIR
  PATH_SUFFIXES SDL2
                # path suffixes to search inside ENV{SDL2DIR}
                include/SDL2 include
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/x86)
endif()

find_library(SDL2_LIBRARY
  NAMES SDL2
  HINTS
    ENV SDL2DIR
  PATH_SUFFIXES lib ${VC_LIB_PATH_SUFFIX}
)

# Hide this cache variable from the user, it's an internal implementation
# detail. The documented library variable for the user is SDL2_LIBRARIES
# which is derived from SDL2_LIBRARY further below.
set_property(CACHE SDL2_LIBRARY PROPERTY TYPE INTERNAL)

if(NOT SDL2_BUILDING_LIBRARY)
  if(NOT SDL2_INCLUDE_DIRS MATCHES ".framework")
    # Non-OS X framework versions expect you to also dynamically link to
    # SDLmain. This is mainly for Windows and OS X. Other (Unix) platforms
    # seem to provide SDLmain for compatibility even though they don't
    # necessarily need it.
    find_library(SDL2MAIN_LIBRARY
      NAMES SDL2main
      HINTS
        ENV SDL2DIR
      PATH_SUFFIXES lib ${VC_LIB_PATH_SUFFIX}
      PATHS
      /opt
    )
  endif()
endif()

# SDL may require threads on your system.
# The Apple build may not need an explicit flag because one of the
# frameworks may already provide it.
# But for non-OSX systems, I will use the CMake Threads package.
if(NOT APPLE)
  find_package(Threads)
endif()

# MinGW needs an additional link flag, -mwindows
# It's total link flags should look like -lmingw32 -lSDLmain -lSDL -mwindows
if(MINGW)
  set(MINGW32_LIBRARY mingw32 "-mwindows" CACHE STRING "link flags for MinGW")
endif()

if(SDL2_LIBRARY)
  set(SDL2_LIBRARIES_TEMP ${SDL2_LIBRARY})
  # For SDLmain
  if(SDL2MAIN_LIBRARY AND NOT SDL2_BUILDING_LIBRARY)
    list(FIND SDL2_LIBRARY "${SDL2MAIN_LIBRARY}" _SDL2_MAIN_INDEX)
    if(_SDL2_MAIN_INDEX EQUAL -1)
      set(SDL2_LIBRARIES_TEMP "${SDL2MAIN_LIBRARY}" ${SDL2_LIBRARIES_TEMP})
    endif()
    unset(_SDL2_MAIN_INDEX)
  endif()

  # For OS X, SDL uses Cocoa as a backend so it must link to Cocoa.
  # CMake doesn't display the -framework Cocoa string in the UI even
  # though it actually is there if I modify a pre-used variable.
  # I think it has something to do with the CACHE STRING.
  # So I use a temporary variable until the end so I can set the
  # "real" variable in one-shot.
  if(APPLE)
    set(SDL2_LIBRARIES_TEMP ${SDL2_LIBRARIES_TEMP} "-framework Cocoa")
  endif()

  # For threads, as mentioned Apple doesn't need this.
  # In fact, there seems to be a problem if I used the Threads package
  # and try using this line, so I'm just skipping it entirely for OS X.
  if(NOT APPLE)
    set(SDL2_LIBRARIES_TEMP ${SDL2_LIBRARIES_TEMP} ${CMAKE_THREAD_LIBS_INIT})
  endif()

  # For MinGW library
  if(MINGW)
    set(SDL2_LIBRARIES_TEMP ${MINGW32_LIBRARY} ${SDL2_LIBRARIES_TEMP})
  endif()

  # Set the final string here so the GUI reflects the final state.
  set(SDL2_LIBRARIES ${SDL2_LIBRARIES_TEMP} CACHE STRING "Where the SDL Library can be found")
endif()

if(SDL2_INCLUDE_DIRS AND EXISTS "${SDL2_INCLUDE_DIRS}/SDL_version.h")
  file(STRINGS "${SDL2_INCLUDE_DIRS}/SDL_version.h" SDL2_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_INCLUDE_DIRS}/SDL_version.h" SDL2_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL2_INCLUDE_DIRS}/SDL_version.h" SDL2_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_VERSION_MAJOR "${SDL2_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL2_VERSION_MINOR "${SDL2_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL2_VERSION_PATCH "${SDL2_VERSION_PATCH_LINE}")
  set(SDL2_VERSION_STRING ${SDL2_VERSION_MAJOR}.${SDL2_VERSION_MINOR}.${SDL2_VERSION_PATCH})
  unset(SDL2_VERSION_MAJOR_LINE)
  unset(SDL2_VERSION_MINOR_LINE)
  unset(SDL2_VERSION_PATCH_LINE)
  unset(SDL2_VERSION_MAJOR)
  unset(SDL2_VERSION_MINOR)
  unset(SDL2_VERSION_PATCH)
endif()

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL2
                                  REQUIRED_VARS SDL2_LIBRARIES SDL2_INCLUDE_DIRS
                                  VERSION_VAR SDL2_VERSION_STRING)

if (SDL2_FOUND)
  if(NOT TARGET SDL2::SDL2)
      add_library(SDL2::SDL2 UNKNOWN IMPORTED)
      set_target_properties(SDL2::SDL2 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}")
      set_target_properties(SDL2::SDL2 PROPERTIES INTERFACE_LINK_LIBRARIES "${SDL2_LIBRARIES}")
      set_target_properties(SDL2::SDL2 PROPERTIES IMPORTED_LOCATION "${SDL2_LIBRARY}")
      if(NOT APPLE)
        target_link_libraries(SDL2::SDL2 INTERFACE Threads::Threads)
      endif()
  endif()
endif()
