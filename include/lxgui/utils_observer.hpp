#ifndef LXGUI_UTILS_OBSERVER_HPP
#define LXGUI_UTILS_OBSERVER_HPP

#include <lxgui/lxgui.hpp>
#include <oup/observable_unique_ptr.hpp>

namespace lxgui {
namespace utils
{
using oup::observable_unique_ptr;
using oup::weak_ptr;
using oup::make_observable_unique;
}
}

#endif
