#include "lxgui/gui_layeredregion.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/gui_region_tpl.hpp"

#include <sstream>

namespace lxgui::gui {

layered_region::layered_region(utils::control_block& block, manager& mgr) : base(block, mgr) {
    type_.push_back(class_name);
}

std::string layered_region::serialize(const std::string& tab) const {
    std::ostringstream str;
    str << base::serialize(tab);

    str << tab << "  # Layer       : ";
    switch (layer_) {
    case layer::background: str << "BACKGROUND\n"; break;
    case layer::border: str << "BORDER\n"; break;
    case layer::artwork: str << "ARTWORK\n"; break;
    case layer::overlay: str << "OVERLAY\n"; break;
    case layer::highlight: str << "HIGHLIGHT\n"; break;
    case layer::specialhigh: str << "SPECIALHIGH\n"; break;
    default: str << "<error>\n"; break;
    }

    return str.str();
}

void layered_region::create_glue() {
    create_glue_(this);
}

utils::owner_ptr<region> layered_region::release_from_parent() {
    if (!p_parent_)
        return nullptr;

    return p_parent_->remove_region(
        utils::static_pointer_cast<layered_region>(observer_from_this()));
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
    return p_parent_->is_visible() && is_shown_;
}

layer layered_region::get_draw_layer() const {
    return layer_;
}

void layered_region::set_draw_layer(layer layer_id) {
    if (layer_ != layer_id) {
        layer_ = layer_id;
        notify_renderer_need_redraw();
        p_parent_->notify_layers_need_update();
    }
}

void layered_region::set_draw_layer(const std::string& layer_name) {
    set_draw_layer(parse_layer_type(layer_name));
}

void layered_region::notify_renderer_need_redraw() {
    if (is_virtual_)
        return;

    if (p_parent_)
        p_parent_->notify_renderer_need_redraw();
}

layer parse_layer_type(const std::string& layer_name) {
    layer layer_id;
    if (layer_name == "ARTWORK")
        layer_id = layer::artwork;
    else if (layer_name == "BACKGROUND")
        layer_id = layer::background;
    else if (layer_name == "BORDER")
        layer_id = layer::border;
    else if (layer_name == "HIGHLIGHT")
        layer_id = layer::highlight;
    else if (layer_name == "OVERLAY")
        layer_id = layer::overlay;
    else {
        gui::out << gui::warning << "gui::parse_layer_type : "
                 << "Unknown layer type : \"" << layer_name << "\". Using \"ARTWORK\"."
                 << std::endl;

        layer_id = layer::artwork;
    }

    return layer_id;
}

} // namespace lxgui::gui
