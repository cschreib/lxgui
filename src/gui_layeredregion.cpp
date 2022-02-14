#include "lxgui/gui_layeredregion.hpp"

#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region.hpp"
#include "lxgui/gui_region_tpl.hpp"

#include <sstream>

namespace lxgui::gui {

layered_region::layered_region(utils::control_block& m_block, manager& m_manager) :
    base(m_block, m_manager) {
    type_.push_back(class_name);
}

std::string layered_region::serialize(const std::string& s_tab) const {
    std::ostringstream s_str;
    s_str << base::serialize(s_tab);

    s_str << s_tab << "  # Layer       : ";
    switch (m_layer_) {
    case layer::background: s_str << "BACKGROUND\n"; break;
    case layer::border: s_str << "BORDER\n"; break;
    case layer::artwork: s_str << "ARTWORK\n"; break;
    case layer::overlay: s_str << "OVERLAY\n"; break;
    case layer::highlight: s_str << "HIGHLIGHT\n"; break;
    case layer::specialhigh: s_str << "SPECIALHIGH\n"; break;
    default: s_str << "<error>\n"; break;
    }

    return s_str.str();
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
    if (!b_is_shown_) {
        b_is_shown_ = true;
        notify_renderer_need_redraw();
    }
}

void layered_region::hide() {
    if (b_is_shown_) {
        b_is_shown_ = false;
        notify_renderer_need_redraw();
    }
}

bool layered_region::is_visible() const {
    return p_parent_->is_visible() && b_is_shown_;
}

layer layered_region::get_draw_layer() const {
    return m_layer_;
}

void layered_region::set_draw_layer(layer m_layer) {
    if (m_layer_ != m_layer) {
        m_layer_ = m_layer;
        notify_renderer_need_redraw();
        p_parent_->notify_layers_need_update();
    }
}

void layered_region::set_draw_layer(const std::string& s_layer) {
    set_draw_layer(parse_layer_type(s_layer));
}

void layered_region::notify_renderer_need_redraw() {
    if (b_virtual_)
        return;

    if (p_parent_)
        p_parent_->notify_renderer_need_redraw();
}

layer parse_layer_type(const std::string& s_layer) {
    layer m_layer;
    if (s_layer == "ARTWORK")
        m_layer = layer::artwork;
    else if (s_layer == "BACKGROUND")
        m_layer = layer::background;
    else if (s_layer == "BORDER")
        m_layer = layer::border;
    else if (s_layer == "HIGHLIGHT")
        m_layer = layer::highlight;
    else if (s_layer == "OVERLAY")
        m_layer = layer::overlay;
    else {
        gui::out << gui::warning << "gui::parse_layer_type : "
                 << "Unknown layer type : \"" << s_layer << "\". Using \"ARTWORK\"." << std::endl;

        m_layer = layer::artwork;
    }

    return m_layer;
}

} // namespace lxgui::gui
