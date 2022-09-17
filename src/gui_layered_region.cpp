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

layer layered_region::get_draw_layer() const {
    return layer_;
}

void layered_region::set_draw_layer(layer layer_id) {
    if (layer_ == layer_id)
        return;

    layer_ = layer_id;
    if (parent_)
        parent_->notify_layers_need_update();
}

int layered_region::get_region_level() const {
    return region_level_;
}

void layered_region::set_region_level(int region_level) {
    if (region_level_ == region_level)
        return;

    region_level = region_level_;
    if (parent_)
        parent_->notify_layers_need_update();
}

void layered_region::notify_renderer_need_redraw() {
    if (is_virtual_)
        return;

    if (parent_)
        parent_->notify_renderer_need_redraw();
}

const std::vector<std::string>& layered_region::get_type_list_() const {
    return get_type_list_impl_<layered_region>();
}

} // namespace lxgui::gui
