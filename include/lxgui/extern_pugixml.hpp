#if defined(LXGUI_COMPILER_MSVC)
#elif defined(LXGUI_COMPILER_GCC)
#    pragma GCC diagnostic push
#elif defined(LXGUI_COMPILER_CLANG) || defined(LXGUI_COMPILER_EMSCRIPTEN)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wdeprecated-dynamic-exception-spec"
#endif

#include <pugixml.hpp>

#if defined(LXGUI_COMPILER_MSVC)
#elif defined(LXGUI_COMPILER_GCC)
#    pragma GCC diagnostic pop
#elif defined(LXGUI_COMPILER_CLANG)
#    pragma clang diagnostic pop
#endif
