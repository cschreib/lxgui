#ifndef LXGUI_GUI_ANCHOR_HPP
#define LXGUI_GUI_ANCHOR_HPP

#include "lxgui/gui_vector2.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/utils_observer.hpp"

#include <string>

namespace lxgui::gui {

class region;

enum class anchor_type { abs, rel };

enum class anchor_point {
    top_left = 0,
    top,
    top_right,
    right,
    bottom_right,
    bottom,
    bottom_left,
    left,
    center
};

enum class constraint { none, x, y };

/// Stores a position for a UI region
struct anchor_data {
    anchor_data(anchor_point m_input_point) :
        m_point(m_input_point), s_parent("$default"), m_parent_point(m_input_point) {}

    anchor_data(anchor_point m_input_point, const std::string& s_input_parent) :
        m_point(m_input_point), s_parent(s_input_parent), m_parent_point(m_input_point) {}

    anchor_data(
        anchor_point m_input_point, const std::string& s_input_parent, anchor_point m_input_parent_point) :
        m_point(m_input_point), s_parent(s_input_parent), m_parent_point(m_input_parent_point) {}

    anchor_data(
        anchor_point       m_input_point,
        const std::string& s_input_parent,
        anchor_point       m_input_parent_point,
        const vector2f&    m_input_offset,
        anchor_type        m_input_type = anchor_type::abs) :
        m_point(m_input_point),
        s_parent(s_input_parent),
        m_parent_point(m_input_parent_point),
        m_offset(m_input_offset),
        m_type(m_input_type) {}

    anchor_data(
        anchor_point       m_input_point,
        const std::string& s_input_parent,
        const vector2f&    m_input_offset,
        anchor_type        m_input_type = anchor_type::abs) :
        m_point(m_input_point),
        s_parent(s_input_parent),
        m_parent_point(m_input_point),
        m_offset(m_input_offset),
        m_type(m_input_type) {}

    anchor_data(
        anchor_point    m_input_point,
        const vector2f& m_input_offset,
        anchor_type     m_input_type = anchor_type::abs) :
        m_point(m_input_point),
        s_parent("$default"),
        m_parent_point(m_input_point),
        m_offset(m_input_offset),
        m_type(m_input_type) {}

    anchor_data(
        anchor_point    m_input_point,
        anchor_point    m_input_parent_point,
        const vector2f& m_input_offset,
        anchor_type     m_input_type = anchor_type::abs) :
        m_point(m_input_point),
        s_parent("$default"),
        m_parent_point(m_input_parent_point),
        m_offset(m_input_offset),
        m_type(m_input_type) {}

    anchor_data(anchor_point m_input_point, anchor_point m_input_parent_point) :
        m_point(m_input_point), s_parent("$default"), m_parent_point(m_input_parent_point) {}

    anchor_point m_point = anchor_point::top_left;
    std::string  s_parent;
    anchor_point m_parent_point = anchor_point::top_left;
    vector2f     m_offset;
    anchor_type  m_type = anchor_type::abs;
};

/// Stores a position for a UI region
class anchor : private anchor_data {
public:
    using anchor_data::m_offset;
    using anchor_data::m_parent_point;
    using anchor_data::m_point;
    using anchor_data::m_type;

    /// Constructor.
    anchor(region& m_object, const anchor_data& m_anchor);

    /// Non-copiable
    anchor(const anchor&) = delete;

    /// Non-movable
    anchor(anchor&&) = delete;

    /// Non-assignable
    anchor& operator=(const anchor&) = delete;

    /// Non-assignable
    anchor& operator=(anchor&&) = delete;

    /// Returns this anchor's absolute coordinates (in pixels).
    /** \param mObject The object owning this anchor
     *   \return The absolute coordinates of this anchor.
     */
    vector2f get_point(const region& m_object) const;

    /// Returns this anchor's parent region.
    /** \return This anchor's parent region
     */
    const utils::observer_ptr<region>& get_parent() {
        return p_parent_;
    }

    /// Returns this anchor's parent region.
    /** \return This anchor's parent region
     */
    utils::observer_ptr<const region> get_parent() const {
        return p_parent_;
    }

    /// Prints all relevant information about this anchor in a string.
    /** \param sTab The offset to give to all lines
     *   \return All relevant information about this anchor
     */
    std::string serialize(const std::string& s_tab) const;

    /// Returns the raw data used for this anchor.
    /** \return The raw data used for this anchor
     */
    const anchor_data& get_data() const {
        return *this;
    }

    /// Returns the name of an anchor point.
    /** \param mPoint The anchor point
     */
    static std::string get_string_point(anchor_point m_point);

    /// Returns the anchor point from its name.
    /** \param sPoint The name of the anchor point
     */
    static anchor_point get_anchor_point(const std::string& s_point);

private:
    /// Update the anchor parent object from the parent string.
    /** \param mObject The object owning this anchor
     */
    void update_parent_(region& m_object);

    utils::observer_ptr<region> p_parent_ = nullptr;
};

} // namespace lxgui::gui

#endif
