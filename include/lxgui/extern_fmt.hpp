#if defined(LXGUI_COMPILER_MSVC)
#elif defined(LXGUI_COMPILER_GCC)
#    pragma GCC diagnostic push
#elif defined(LXGUI_COMPILER_CLANG) || defined(LXGUI_COMPILER_EMSCRIPTEN)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wsigned-enum-bitfield"
#    if __clang_major__ >= 13
#        pragma clang diagnostic ignored "-Wreserved-identifier"
#    endif
#endif

#include <fmt/format.h>

#if defined(LXGUI_COMPILER_MSVC)
#elif defined(LXGUI_COMPILER_GCC)
#    pragma GCC diagnostic pop
#elif defined(LXGUI_COMPILER_CLANG)
#    pragma clang diagnostic pop
#endif
