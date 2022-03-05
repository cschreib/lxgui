#include "lxgui/gui_layered_region.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/gui_region_tpl.hpp"

#include <sstream>

namespace lxgui::gui {

layered_region::layered_region(
    utils::control_block& block, manager& mgr, const region_core_attributes& attr) :
    base(block, mgr, attr) {

    initialize_(*this, attr);
}

std::string layered_region::serialize(const std::string& tab) const {
    std::ostringstream str;
    str << base::serialize(tab);
    str << tab << "  # Layer      : " << utils::to_string(layer_) << "\n";

    return str.str();
}

utils::owner_ptr<region> layered_region::release_from_parent() {
    if (!parent_)
        return nullptr;

    return parent_->remove_region(utils::static_pointer_cast<layered_region>(observer_from_this()));
}

void layered_region::show() {
    if (!is_shown_) {
        is_shown_ = true;
        notify_renderer_need_redraw();
    }
}

void layered_region::hide() {
    if (is_shown_) {
        is_shown_ = false;
        notify_renderer_need_redraw();
    }
}

bool layered_region::is_visible() const {
    return parent_->is_visible() && is_shown_;
}

layer layered_region::get_draw_layer() const {
    return layer_;
}

void layered_region::set_draw_layer(layer layer_id) {
    if (layer_ != layer_id) {
        layer_ = layer_id;
        notify_renderer_need_redraw();
        parent_->notify_layers_need_update();
    }
}

void layered_region::notify_renderer_need_redraw() {
    if (is_virtual_)
        return;

    if (parent_)
        parent_->notify_renderer_need_redraw();
}

} // namespace lxgui::gui
