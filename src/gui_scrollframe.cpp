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

scroll_frame::scroll_frame(utils::control_block& m_block, manager& m_manager) :
    frame(m_block, m_manager) {
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

bool scroll_frame::can_use_script(const std::string& s_script_name) const {
    if (frame::can_use_script(s_script_name))
        return true;
    else if (
        (s_script_name == "OnHorizontalScroll") || (s_script_name == "OnScrollRangeChanged") ||
        (s_script_name == "OnVerticalScroll"))
        return true;
    else
        return false;
}

void scroll_frame::fire_script(const std::string& s_script_name, const event_data& m_data) {
    if (!is_loaded())
        return;

    alive_checker m_checker(*this);
    base::fire_script(s_script_name, m_data);
    if (!m_checker.is_alive())
        return;

    if (s_script_name == "OnSizeChanged")
        b_rebuild_scroll_render_target_ = true;
}

void scroll_frame::copy_from(const region& m_obj) {
    base::copy_from(m_obj);

    const scroll_frame* p_scroll_frame = down_cast<scroll_frame>(&m_obj);
    if (!p_scroll_frame)
        return;

    this->set_horizontal_scroll(p_scroll_frame->get_horizontal_scroll());
    this->set_vertical_scroll(p_scroll_frame->get_vertical_scroll());

    if (const frame* p_other_child = p_scroll_frame->get_scroll_child().get()) {
        region_core_attributes m_attr;
        m_attr.s_object_type = p_other_child->get_object_type();
        m_attr.s_name        = p_other_child->get_raw_name();
        m_attr.inheritance   = {p_scroll_frame->get_scroll_child()};

        utils::observer_ptr<frame> p_scroll_child = create_child(std::move(m_attr));

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

        b_rebuild_scroll_render_target_ = true;
    }

    p_scroll_child_ = p_frame;

    if (p_scroll_child_) {
        add_child(std::move(p_frame));

        p_scroll_child_->set_special();
        if (!is_virtual())
            p_scroll_child_->set_renderer(observer_from(this));

        p_scroll_child_->clear_all_points();
        p_scroll_child_->set_point(anchor_point::top_left, get_name(), -m_scroll_);

        update_scroll_range_();
        b_update_scroll_range_ = false;
    }

    b_redraw_scroll_render_target_ = true;
}

void scroll_frame::set_horizontal_scroll(float f_horizontal_scroll) {
    if (m_scroll_.x != f_horizontal_scroll) {
        m_scroll_.x = f_horizontal_scroll;

        alive_checker m_checker(*this);
        fire_script("OnHorizontalScroll");
        if (!m_checker.is_alive())
            return;

        p_scroll_child_->modify_point(anchor_point::top_left).m_offset = -m_scroll_;
        p_scroll_child_->notify_borders_need_update();

        b_redraw_scroll_render_target_ = true;
    }
}

float scroll_frame::get_horizontal_scroll() const {
    return m_scroll_.x;
}

float scroll_frame::get_horizontal_scroll_range() const {
    return m_scroll_range_.x;
}

void scroll_frame::set_vertical_scroll(float f_vertical_scroll) {
    if (m_scroll_.y != f_vertical_scroll) {
        m_scroll_.y = f_vertical_scroll;

        alive_checker m_checker(*this);
        fire_script("OnVerticalScroll");
        if (!m_checker.is_alive())
            return;

        p_scroll_child_->modify_point(anchor_point::top_left).m_offset = -m_scroll_;
        p_scroll_child_->notify_borders_need_update();

        b_redraw_scroll_render_target_ = true;
    }
}

float scroll_frame::get_vertical_scroll() const {
    return m_scroll_.y;
}

float scroll_frame::get_vertical_scroll_range() const {
    return m_scroll_range_.y;
}

void scroll_frame::update(float f_delta) {
    vector2f m_old_child_size;
    if (p_scroll_child_)
        m_old_child_size = p_scroll_child_->get_apparent_dimensions();

    alive_checker m_checker(*this);
    base::update(f_delta);
    if (!m_checker.is_alive())
        return;

    if (p_scroll_child_ && m_old_child_size != p_scroll_child_->get_apparent_dimensions()) {
        b_update_scroll_range_         = true;
        b_redraw_scroll_render_target_ = true;
    }

    if (is_visible()) {
        if (b_rebuild_scroll_render_target_ && p_scroll_texture_) {
            rebuild_scroll_render_target_();
            b_rebuild_scroll_render_target_ = false;
            b_redraw_scroll_render_target_  = true;
        }

        if (b_update_scroll_range_) {
            update_scroll_range_();
            b_update_scroll_range_ = false;
        }

        if (p_scroll_child_ && p_scroll_render_target_ && b_redraw_scroll_render_target_) {
            render_scroll_strata_list_();
            b_redraw_scroll_render_target_ = false;
        }
    }
}

void scroll_frame::update_scroll_range_() {
    const vector2f m_apparent_size       = get_apparent_dimensions();
    const vector2f m_child_apparent_size = p_scroll_child_->get_apparent_dimensions();

    m_scroll_range_ = m_child_apparent_size - m_apparent_size;

    if (m_scroll_range_.x < 0)
        m_scroll_range_.x = 0;
    if (m_scroll_range_.y < 0)
        m_scroll_range_.y = 0;

    if (!is_virtual()) {
        alive_checker m_checker(*this);
        fire_script("OnScrollRangeChanged");
        if (!m_checker.is_alive())
            return;
    }
}

void scroll_frame::notify_scaling_factor_updated() {
    base::notify_scaling_factor_updated();

    b_rebuild_scroll_render_target_ = true;
}

void scroll_frame::rebuild_scroll_render_target_() {
    const vector2f m_apparent_size = get_apparent_dimensions();

    if (m_apparent_size.x <= 0 || m_apparent_size.y <= 0)
        return;

    float     f_factor      = get_manager().get_interface_scaling_factor();
    vector2ui m_scaled_size = vector2ui(
        std::round(m_apparent_size.x * f_factor), std::round(m_apparent_size.y * f_factor));

    if (p_scroll_render_target_) {
        p_scroll_render_target_->set_dimensions(m_scaled_size);
        p_scroll_texture_->set_tex_rect(std::array<float, 4>{0.0f, 0.0f, 1.0f, 1.0f});
        b_update_scroll_range_ = true;
    } else {
        auto& m_renderer        = get_manager().get_renderer();
        p_scroll_render_target_ = m_renderer.create_render_target(m_scaled_size);

        if (p_scroll_render_target_)
            p_scroll_texture_->set_texture(p_scroll_render_target_);
    }
}

void scroll_frame::render_scroll_strata_list_() {
    renderer& m_renderer = get_manager().get_renderer();

    m_renderer.begin(p_scroll_render_target_);

    vector2f m_view = vector2f(p_scroll_render_target_->get_canvas_dimensions()) /
                      get_manager().get_interface_scaling_factor();

    m_renderer.set_view(matrix4f::translation(-get_borders().top_left()) * matrix4f::view(m_view));

    p_scroll_render_target_->clear(color::empty);

    for (const auto& m_strata : strata_list_) {
        render_strata_(m_strata);
    }

    m_renderer.end();
}

utils::observer_ptr<const frame>
scroll_frame::find_topmost_frame(const std::function<bool(const frame&)>& m_predicate) const {
    if (base::find_topmost_frame(m_predicate)) {
        if (auto p_hovered_frame = frame_renderer::find_topmost_frame(m_predicate))
            return p_hovered_frame;

        return observer_from(this);
    }

    return nullptr;
}

void scroll_frame::notify_strata_needs_redraw(frame_strata m_strata) {
    frame_renderer::notify_strata_needs_redraw(m_strata);

    b_redraw_scroll_render_target_ = true;
    notify_renderer_need_redraw();
}

void scroll_frame::create_glue() {
    create_glue_(this);
}

void scroll_frame::notify_rendered_frame(
    const utils::observer_ptr<frame>& p_frame, bool b_rendered) {
    if (!p_frame)
        return;

    frame_renderer::notify_rendered_frame(p_frame, b_rendered);

    b_redraw_scroll_render_target_ = true;
}

vector2f scroll_frame::get_target_dimensions() const {
    return get_apparent_dimensions();
}

} // namespace lxgui::gui
