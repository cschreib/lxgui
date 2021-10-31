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
      add_library(oup::oup UNKNOWN IMPORTED)
      set_target_properties(oup::oup PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${OUP_INCLUDE_DIRS}")
  endif()
endif()
