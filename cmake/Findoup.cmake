find_path(OUP_INCLUDE_DIR observable_unique_ptr.hpp
  HINTS ${OUP_DIR}
  PATH_SUFFIXES oup include/oup
)

set(OUP_INCLUDE_DIRS ${OUP_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(oup
                                  REQUIRED_VARS OUP_INCLUDE_DIRS
                                  VERSION_VAR OUP_VERSION_STRING)

mark_as_advanced(OUP_INCLUDE_DIR)

if (OUP_FOUND)
  if(NOT TARGET oup::oup)
      add_library(oup::oup IMPORTED INTERFACE)
      target_include_directories(oup::oup INTERFACE "${OUP_INCLUDE_DIRS}")
  endif()
endif()
