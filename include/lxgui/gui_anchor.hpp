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
    anchor_data(anchor_point input_point) :
        point(input_point), parent_name("$default"), parent_point(input_point) {}

    anchor_data(anchor_point input_point, const std::string& input_parent) :
        point(input_point), parent_name(input_parent), parent_point(input_point) {}

    anchor_data(
        anchor_point       input_point,
        const std::string& input_parent,
        anchor_point       input_parent_point) :
        point(input_point), parent_name(input_parent), parent_point(input_parent_point) {}

    anchor_data(
        anchor_point       input_point,
        const std::string& input_parent,
        anchor_point       input_parent_point,
        const vector2f&    input_offset,
        anchor_type        input_type = anchor_type::abs) :
        point(input_point),
        parent_name(input_parent),
        parent_point(input_parent_point),
        offset(input_offset),
        type(input_type) {}

    anchor_data(
        anchor_point       input_point,
        const std::string& input_parent,
        const vector2f&    input_offset,
        anchor_type        input_type = anchor_type::abs) :
        point(input_point),
        parent_name(input_parent),
        parent_point(input_point),
        offset(input_offset),
        type(input_type) {}

    anchor_data(
        anchor_point    input_point,
        const vector2f& input_offset,
        anchor_type     input_type = anchor_type::abs) :
        point(input_point),
        parent_name("$default"),
        parent_point(input_point),
        offset(input_offset),
        type(input_type) {}

    anchor_data(
        anchor_point    input_point,
        anchor_point    input_parent_point,
        const vector2f& input_offset,
        anchor_type     input_type = anchor_type::abs) :
        point(input_point),
        parent_name("$default"),
        parent_point(input_parent_point),
        offset(input_offset),
        type(input_type) {}

    anchor_data(anchor_point input_point, anchor_point input_parent_point) :
        point(input_point), parent_name("$default"), parent_point(input_parent_point) {}

    anchor_point point = anchor_point::top_left;
    std::string  parent_name;
    anchor_point parent_point = anchor_point::top_left;
    vector2f     offset;
    anchor_type  type = anchor_type::abs;
};

/// Stores a position for a UI region
class anchor : private anchor_data {
public:
    using anchor_data::offset;
    using anchor_data::parent_point;
    using anchor_data::point;
    using anchor_data::type;

    /// Constructor.
    /** \param object The object to which this anchor belongs
     *  \param data The data about the anchor
     */
    anchor(region& object, const anchor_data& data);

    // Non-copiable, non-movable
    anchor(const anchor&) = delete;
    anchor(anchor&&)      = delete;
    anchor& operator=(const anchor&) = delete;
    anchor& operator=(anchor&&) = delete;

    /// Returns this anchor's absolute coordinates (in pixels).
    /** \param object The object owning this anchor
     * \return The absolute coordinates of this anchor.
     */
    vector2f get_point(const region& object) const;

    /// Returns this anchor's parent region.
    /** \return This anchor's parent region
     */
    const utils::observer_ptr<region>& get_parent() {
        return parent_;
    }

    /// Returns this anchor's parent region.
    /** \return This anchor's parent region
     */
    utils::observer_ptr<const region> get_parent() const {
        return parent_;
    }

    /// Prints all relevant information about this anchor in a string.
    /** \param tab The offset to give to all lines
     * \return All relevant information about this anchor
     */
    std::string serialize(const std::string& tab) const;

    /// Returns the raw data used for this anchor.
    /** \return The raw data used for this anchor
     */
    const anchor_data& get_data() const {
        return *this;
    }

    /// Returns the name of an anchor point.
    /** \param point The anchor point
     */
    static std::string get_anchor_point_name(anchor_point point);

    /// Returns the anchor point from its name.
    /** \param point_name The name of the anchor point
     */
    static anchor_point get_anchor_point(const std::string& point_name);

private:
    /// Update the anchor parent object from the parent string.
    /** \param object The object owning this anchor
     */
    void update_parent_(region& object);

    utils::observer_ptr<region> parent_ = nullptr;
};

} // namespace lxgui::gui

#endif
