#ifndef LXGUI_UTILS_OBSERVER_HPP
#define LXGUI_UTILS_OBSERVER_HPP

#include <lxgui/lxgui.hpp>
#include <oup/observable_unique_ptr.hpp>

namespace lxgui {
namespace utils
{
    template<typename T>
    using owner_ptr = oup::observable_sealed_ptr<T>;

    using oup::observer_ptr;

    using oup::static_pointer_cast;
    using oup::dynamic_pointer_cast;
    using oup::const_pointer_cast;

    using oup::enable_observer_from_this;

    template<typename T, typename ... Args>
    owner_ptr<T> make_owned(Args&& ... args)
    {
        return oup::make_observable_sealed<T>(std::forward<Args>(args)...);
    }
}
}

#endif
