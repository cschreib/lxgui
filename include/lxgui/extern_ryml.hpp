#if defined(LXGUI_COMPILER_MSVC)
#elif defined(LXGUI_COMPILER_GCC)
#    pragma GCC diagnostic push
#elif defined(LXGUI_COMPILER_CLANG)
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wextra-semi-stmt"
#endif

#include <ryml.hpp>
#include <ryml_std.hpp>

#if defined(LXGUI_COMPILER_MSVC)
#elif defined(LXGUI_COMPILER_GCC)
#    pragma GCC diagnostic pop
#elif defined(LXGUI_COMPILER_CLANG)
#    pragma clang diagnostic pop
#endif
