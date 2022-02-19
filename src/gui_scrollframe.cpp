#include "lxgui/gui_scrollframe.hpp"

#include "lxgui/gui_alive_checker.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_region_tpl.hpp"
#include "lxgui/gui_renderer.hpp"
#include "lxgui/gui_rendertarget.hpp"
#include "lxgui/gui_texture.hpp"

namespace lxgui::gui {

scroll_frame::scroll_frame(utils::control_block& block, manager& mgr) : frame(block, mgr) {
    type_.push_back(class_name);
}

scroll_frame::~scroll_frame() {
    // Make sure the scroll child is destroyed now.
    // It relies on this scroll_frame still being alive
    // when being destroyed, but the scroll_frame destructor
    // would be called before its inherited frame destructor
    // (which would otherwise take care of destroying the scroll child).
    if (p_scroll_child_)
        remove_child(p_scroll_child_);
}

bool scroll_frame::can_use_script(const std::string& script_name) const {
    if (frame::can_use_script(script_name))
        return true;
    else if (
        (script_name == "OnHorizontalScroll") || (script_name == "OnScrollRangeChanged") ||
        (script_name == "OnVerticalScroll"))
        return true;
    else
        return false;
}

void scroll_frame::fire_script(const std::string& script_name, const event_data& data) {
    if (!is_loaded())
        return;

    alive_checker checker(*this);
    base::fire_script(script_name, data);
    if (!checker.is_alive())
        return;

    if (script_name == "OnSizeChanged")
        rebuild_scroll_render_target_flag_ = true;
}

void scroll_frame::copy_from(const region& obj) {
    base::copy_from(obj);

    const scroll_frame* p_scroll_frame = down_cast<scroll_frame>(&obj);
    if (!p_scroll_frame)
        return;

    this->set_horizontal_scroll(p_scroll_frame->get_horizontal_scroll());
    this->set_vertical_scroll(p_scroll_frame->get_vertical_scroll());

    if (const frame* p_other_child = p_scroll_frame->get_scroll_child().get()) {
        region_core_attributes attr;
        attr.object_type = p_other_child->get_object_type();
        attr.name        = p_other_child->get_raw_name();
        attr.inheritance = {p_scroll_frame->get_scroll_child()};

        utils::observer_ptr<frame> p_scroll_child = create_child(std::move(attr));

        if (p_scroll_child) {
            p_scroll_child->set_special();
            p_scroll_child->notify_loaded();
            this->set_scroll_child(remove_child(p_scroll_child));
        }
    }
}

void scroll_frame::set_scroll_child(utils::owner_ptr<frame> p_frame) {
    if (p_scroll_child_) {
        p_scroll_child_->set_renderer(nullptr);
        clear_strata_list_();
    } else if (!is_virtual() && !p_scroll_texture_) {
        // Create the scroll texture
        auto p_scroll_texture =
            create_layered_region<texture>(layer::artwork, "$parentScrollTexture");

        if (!p_scroll_texture)
            return;

        p_scroll_texture->set_special();
        p_scroll_texture->set_all_points(observer_from(this));

        if (p_scroll_render_target_)
            p_scroll_texture->set_texture(p_scroll_render_target_);

        p_scroll_texture->notify_loaded();
        p_scroll_texture_ = p_scroll_texture;

        rebuild_scroll_render_target_flag_ = true;
    }

    p_scroll_child_ = p_frame;

    if (p_scroll_child_) {
        add_child(std::move(p_frame));

        p_scroll_child_->set_special();
        if (!is_virtual())
            p_scroll_child_->set_renderer(observer_from(this));

        p_scroll_child_->clear_all_points();
        p_scroll_child_->set_point(anchor_point::top_left, get_name(), -scroll_);

        update_scroll_range_();
        update_scroll_range_flag_ = false;
    }

    redraw_scroll_render_target_flag_ = true;
}

void scroll_frame::set_horizontal_scroll(float horizontal_scroll) {
    if (scroll_.x != horizontal_scroll) {
        scroll_.x = horizontal_scroll;

        alive_checker checker(*this);
        fire_script("OnHorizontalScroll");
        if (!checker.is_alive())
            return;

        p_scroll_child_->modify_point(anchor_point::top_left).offset = -scroll_;
        p_scroll_child_->notify_borders_need_update();

        redraw_scroll_render_target_flag_ = true;
    }
}

float scroll_frame::get_horizontal_scroll() const {
    return scroll_.x;
}

float scroll_frame::get_horizontal_scroll_range() const {
    return scroll_range_.x;
}

void scroll_frame::set_vertical_scroll(float vertical_scroll) {
    if (scroll_.y != vertical_scroll) {
        scroll_.y = vertical_scroll;

        alive_checker checker(*this);
        fire_script("OnVerticalScroll");
        if (!checker.is_alive())
            return;

        p_scroll_child_->modify_point(anchor_point::top_left).offset = -scroll_;
        p_scroll_child_->notify_borders_need_update();

        redraw_scroll_render_target_flag_ = true;
    }
}

float scroll_frame::get_vertical_scroll() const {
    return scroll_.y;
}

float scroll_frame::get_vertical_scroll_range() const {
    return scroll_range_.y;
}

void scroll_frame::update(float delta) {
    vector2f old_child_size;
    if (p_scroll_child_)
        old_child_size = p_scroll_child_->get_apparent_dimensions();

    alive_checker checker(*this);
    base::update(delta);
    if (!checker.is_alive())
        return;

    if (p_scroll_child_ && old_child_size != p_scroll_child_->get_apparent_dimensions()) {
        update_scroll_range_flag_         = true;
        redraw_scroll_render_target_flag_ = true;
    }

    if (is_visible()) {
        if (rebuild_scroll_render_target_flag_ && p_scroll_texture_) {
            rebuild_scroll_render_target_();
            rebuild_scroll_render_target_flag_ = false;
            redraw_scroll_render_target_flag_  = true;
        }

        if (update_scroll_range_flag_) {
            update_scroll_range_();
            update_scroll_range_flag_ = false;
        }

        if (p_scroll_child_ && p_scroll_render_target_ && redraw_scroll_render_target_flag_) {
            render_scroll_strata_list_();
            redraw_scroll_render_target_flag_ = false;
        }
    }
}

void scroll_frame::update_scroll_range_() {
    const vector2f apparent_size       = get_apparent_dimensions();
    const vector2f child_apparent_size = p_scroll_child_->get_apparent_dimensions();

    scroll_range_ = child_apparent_size - apparent_size;

    if (scroll_range_.x < 0)
        scroll_range_.x = 0;
    if (scroll_range_.y < 0)
        scroll_range_.y = 0;

    if (!is_virtual()) {
        alive_checker checker(*this);
        fire_script("OnScrollRangeChanged");
        if (!checker.is_alive())
            return;
    }
}

void scroll_frame::notify_scaling_factor_updated() {
    base::notify_scaling_factor_updated();

    rebuild_scroll_render_target_flag_ = true;
}

void scroll_frame::rebuild_scroll_render_target_() {
    const vector2f apparent_size = get_apparent_dimensions();

    if (apparent_size.x <= 0 || apparent_size.y <= 0)
        return;

    float     factor = get_manager().get_interface_scaling_factor();
    vector2ui scaled_size =
        vector2ui(std::round(apparent_size.x * factor), std::round(apparent_size.y * factor));

    if (p_scroll_render_target_) {
        p_scroll_render_target_->set_dimensions(scaled_size);
        p_scroll_texture_->set_tex_rect(std::array<float, 4>{0.0f, 0.0f, 1.0f, 1.0f});
        update_scroll_range_flag_ = true;
    } else {
        auto& renderer          = get_manager().get_renderer();
        p_scroll_render_target_ = renderer.create_render_target(scaled_size);

        if (p_scroll_render_target_)
            p_scroll_texture_->set_texture(p_scroll_render_target_);
    }
}

void scroll_frame::render_scroll_strata_list_() {
    renderer& renderer = get_manager().get_renderer();

    renderer.begin(p_scroll_render_target_);

    vector2f view = vector2f(p_scroll_render_target_->get_canvas_dimensions()) /
                    get_manager().get_interface_scaling_factor();

    renderer.set_view(matrix4f::translation(-get_borders().top_left()) * matrix4f::view(view));

    p_scroll_render_target_->clear(color::empty);

    for (const auto& s : strata_list_) {
        render_strata_(s);
    }

    renderer.end();
}

utils::observer_ptr<const frame>
scroll_frame::find_topmost_frame(const std::function<bool(const frame&)>& predicate) const {
    if (base::find_topmost_frame(predicate)) {
        if (auto p_hovered_frame = frame_renderer::find_topmost_frame(predicate))
            return p_hovered_frame;

        return observer_from(this);
    }

    return nullptr;
}

void scroll_frame::notify_strata_needs_redraw(frame_strata strata_id) {
    frame_renderer::notify_strata_needs_redraw(strata_id);

    redraw_scroll_render_target_flag_ = true;
    notify_renderer_need_redraw();
}

void scroll_frame::create_glue() {
    create_glue_(this);
}

void scroll_frame::notify_rendered_frame(const utils::observer_ptr<frame>& p_frame, bool rendered) {
    if (!p_frame)
        return;

    frame_renderer::notify_rendered_frame(p_frame, rendered);

    redraw_scroll_render_target_flag_ = true;
}

vector2f scroll_frame::get_target_dimensions() const {
    return get_apparent_dimensions();
}

} // namespace lxgui::gui
