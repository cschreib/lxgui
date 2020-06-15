#include "lxgui/gui_renderer_impl.hpp"

namespace lxgui {
namespace gui
{
void renderer_impl::set_parent(manager* pParent)
{
    pParent_ = pParent;
}

void renderer_impl::notify_window_resized(uint uiNewWidth, uint uiNewHeight)
{
}
}
}
