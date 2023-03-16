#if defined(LXGUI_COMPILER_MSVC)
#elif defined(LXGUI_COMPILER_GCC)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wshadow"
#    pragma GCC diagnostic ignored "-Wsign-conversion"
#    pragma GCC diagnostic ignored "-Wconversion"
#elif defined(LXGUI_COMPILER_CLANG) || defined(LXGUI_COMPILER_EMSCRIPTEN)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wshadow"
#    pragma clang diagnostic ignored "-Wnewline-eof"
#    pragma clang diagnostic ignored "-Wcomma"
#    pragma clang diagnostic ignored "-Wextra-semi"
#    pragma clang diagnostic ignored "-Wundefined-reinterpret-cast"
#    pragma clang diagnostic ignored "-Wsign-conversion"
#    pragma clang diagnostic ignored "-Wconversion"
#endif

#include <sol/object.hpp>

#if defined(LXGUI_COMPILER_MSVC)
#elif defined(LXGUI_COMPILER_GCC)
#    pragma GCC diagnostic pop
#elif defined(LXGUI_COMPILER_CLANG)
#    pragma clang diagnostic pop
#endif
