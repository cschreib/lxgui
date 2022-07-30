#include "lxgui/gui_scroll_frame.hpp"

#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_render_target.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_texture.hpp"

namespace lxgui::gui {

scroll_frame::scroll_frame(
    utils::control_block& block, manager& mgr, const frame_core_attributes& attr) :
    frame(block, mgr, attr) {

    initialize_(*this, attr);
}

scroll_frame::~scroll_frame() {
    // Make sure the scroll child is destroyed now.
    // It relies on this scroll_frame still being alive
    // when being destroyed, but the scroll_frame destructor
    // would be called before its inherited frame destructor
    // (which would otherwise take care of destroying the scroll child).
    if (scroll_child_)
        remove_child(scroll_child_);
}

bool scroll_frame::can_use_script(const std::string& script_name) const {
    return base::can_use_script(script_name) || script_name == "OnHorizontalScroll" ||
           script_name == "OnScrollRangeChanged" || script_name == "OnVerticalScroll";
}

void scroll_frame::fire_script(const std::string& script_name, const event_data& data) {
    if (!is_loaded())
        return;

    alive_checker checker(*this);
    base::fire_script(script_name, data);
    if (!checker.is_alive())
        return;

    if (script_name == "OnSizeChanged") {
        rebuild_scroll_render_target_();

        update_scroll_range_();
        if (!checker.is_alive())
            return;
    }
}

void scroll_frame::copy_from(const region& obj) {
    base::copy_from(obj);

    const scroll_frame* scroll_obj = down_cast<scroll_frame>(&obj);
    if (!scroll_obj)
        return;

    this->set_horizontal_scroll(scroll_obj->get_horizontal_scroll());
    this->set_vertical_scroll(scroll_obj->get_vertical_scroll());

    if (const frame* other_child = scroll_obj->get_scroll_child().get()) {
        frame_core_attributes attr;
        attr.object_type = other_child->get_region_type();
        attr.name        = other_child->get_raw_name();
        attr.inheritance = {scroll_obj->get_scroll_child()};

        utils::observer_ptr<frame> scroll_child = create_child(std::move(attr));

        if (scroll_child) {
            scroll_child->set_manually_inherited(true);
            scroll_child->notify_loaded();
            this->set_scroll_child(remove_child(scroll_child));
        }
    }
}

void scroll_frame::set_scroll_child(utils::owner_ptr<frame> obj) {
    if (scroll_child_) {
        scroll_child_on_resize_connection_.disconnect();
        clear_strata_list_();
    } else if (!is_virtual() && !scroll_texture_) {
        // Create the scroll texture
        auto scroll_texture =
            create_layered_region<texture>(layer::artwork, "$parentScrollTexture");

        if (!scroll_texture)
            return;

        scroll_texture->set_manually_inherited(true);
        scroll_texture->set_all_anchors(observer_from(this));

        if (scroll_render_target_)
            scroll_texture->set_texture(scroll_render_target_);

        scroll_texture->notify_loaded();
        scroll_texture_ = scroll_texture;
    }

    scroll_child_ = obj;

    if (scroll_child_) {
        add_child(std::move(obj));

        scroll_child_->set_manually_inherited(true);
        if (!is_virtual())
            scroll_child_->set_frame_renderer(observer_from(this));

        scroll_child_->clear_all_anchors();
        if (!is_virtual())
            scroll_child_->set_anchor(point::top_left, get_name(), -scroll_);

        scroll_child_on_resize_connection_ = scroll_child_->add_script(
            "OnSizeChanged", [&](frame&, const event_data&) { update_scroll_range_(); });

        update_scroll_range_();
    }

    rebuild_scroll_render_target_();
}

void scroll_frame::set_horizontal_scroll(float horizontal_scroll) {
    if (scroll_.x == horizontal_scroll)
        return;

    scroll_.x = horizontal_scroll;

    alive_checker checker(*this);
    fire_script("OnHorizontalScroll");
    if (!checker.is_alive())
        return;

    scroll_child_->modify_anchor(point::top_left).offset = -scroll_;
    scroll_child_->notify_borders_need_update();

    redraw_scroll_render_target_flag_ = true;
}

float scroll_frame::get_horizontal_scroll() const {
    return scroll_.x;
}

float scroll_frame::get_horizontal_scroll_range() const {
    return scroll_range_.x;
}

void scroll_frame::set_vertical_scroll(float vertical_scroll) {
    if (scroll_.y == vertical_scroll)
        return;

    scroll_.y = vertical_scroll;

    alive_checker checker(*this);
    fire_script("OnVerticalScroll");
    if (!checker.is_alive())
        return;

    scroll_child_->modify_anchor(point::top_left).offset = -scroll_;
    scroll_child_->notify_borders_need_update();

    redraw_scroll_render_target_flag_ = true;
}

float scroll_frame::get_vertical_scroll() const {
    return scroll_.y;
}

float scroll_frame::get_vertical_scroll_range() const {
    return scroll_range_.y;
}

void scroll_frame::update_(float delta) {
    alive_checker checker(*this);
    base::update_(delta);
    if (!checker.is_alive())
        return;

    if (is_visible()) {
        if (scroll_child_ && scroll_render_target_ && redraw_scroll_render_target_flag_) {
            render_scroll_strata_list_();
            redraw_scroll_render_target_flag_ = false;
        }
    }
}

void scroll_frame::update_scroll_range_() {
    const vector2f apparent_size       = get_apparent_dimensions();
    const vector2f child_apparent_size = scroll_child_->get_apparent_dimensions();
    const auto     old_scroll_range    = scroll_range_;

    scroll_range_ = child_apparent_size - apparent_size;

    if (scroll_range_.x < 0)
        scroll_range_.x = 0;
    if (scroll_range_.y < 0)
        scroll_range_.y = 0;

    if (!is_virtual() && scroll_range_ != old_scroll_range) {
        alive_checker checker(*this);
        fire_script("OnScrollRangeChanged");
        if (!checker.is_alive())
            return;
    }
}

void scroll_frame::notify_scaling_factor_updated() {
    base::notify_scaling_factor_updated();

    rebuild_scroll_render_target_();
}

void scroll_frame::rebuild_scroll_render_target_() {
    if (!scroll_texture_)
        return;

    const vector2f apparent_size = get_apparent_dimensions();

    if (apparent_size.x <= 0 || apparent_size.y <= 0)
        return;

    float     factor = get_manager().get_interface_scaling_factor();
    vector2ui scaled_size =
        vector2ui(std::round(apparent_size.x * factor), std::round(apparent_size.y * factor));

    if (scroll_render_target_) {
        scroll_render_target_->set_dimensions(scaled_size);
        scroll_texture_->set_tex_rect(std::array<float, 4>{0.0f, 0.0f, 1.0f, 1.0f});
    } else {
        auto& renderer        = get_manager().get_renderer();
        scroll_render_target_ = renderer.create_render_target(scaled_size);

        if (scroll_render_target_)
            scroll_texture_->set_texture(scroll_render_target_);
    }

    if (scroll_render_target_) {
        render_scroll_strata_list_();
        redraw_scroll_render_target_flag_ = false;
    } else {
        notify_renderer_need_redraw();
    }
}

void scroll_frame::render_scroll_strata_list_() {
    renderer& renderer = get_manager().get_renderer();

    renderer.begin(scroll_render_target_);

    vector2f view = vector2f(scroll_render_target_->get_canvas_dimensions()) /
                    get_manager().get_interface_scaling_factor();

    renderer.set_view(matrix4f::translation(-get_borders().top_left()) * matrix4f::view(view));

    scroll_render_target_->clear(color::empty);

    for (const auto& s : strata_list_) {
        render_strata_(s);
    }

    renderer.end();

    notify_renderer_need_redraw();
}

utils::observer_ptr<const frame>
scroll_frame::find_topmost_frame(const std::function<bool(const frame&)>& predicate) const {
    if (base::find_topmost_frame(predicate)) {
        if (auto hovered_frame = frame_renderer::find_topmost_frame(predicate))
            return hovered_frame;

        return observer_from(this);
    }

    return nullptr;
}

void scroll_frame::notify_strata_needs_redraw(strata strata_id) {
    frame_renderer::notify_strata_needs_redraw(strata_id);
    redraw_scroll_render_target_flag_ = true;
}

vector2f scroll_frame::get_target_dimensions() const {
    return get_apparent_dimensions();
}

const std::vector<std::string>& scroll_frame::get_type_list_() const {
    return get_type_list_impl_<scroll_frame>();
}

} // namespace lxgui::gui
