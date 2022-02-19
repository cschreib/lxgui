#ifndef LXGUI_GUI_LAYOUTNODE_HPP
#define LXGUI_GUI_LAYOUTNODE_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/utils_exception.hpp"
#include "lxgui/utils_string.hpp"
#include "lxgui/utils_view.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace lxgui::gui {

/// An attribute in a layout file
/** This is a format-agnostic representation of a GUI layout, as read
 *   for example from an XML or YAML file. The GUI uses this class to de-couple
 *   the layout parsing format (XML, YAML, etc) from the actual parsed layout.
 */
class layout_attribute {
public:
    layout_attribute()                        = default;
    layout_attribute(const layout_attribute&) = default;
    layout_attribute(layout_attribute&&)      = default;
    layout_attribute& operator=(const layout_attribute&) = default;
    layout_attribute& operator=(layout_attribute&&) = default;

    /// Returns this node's location in the file as {file}:{line}.
    /** \return This node's location in the file as {file}:{line}
     */
    std::string_view get_location() const noexcept {
        return location_;
    }

    /// Returns this node's value location in the file as {file}:{line}.
    /** \return This node's value location in the file as {file}:{line}
     */
    std::string_view get_value_location() const noexcept {
        return value_location_;
    }

    /// Returns the file from in which this node is located.
    /** \return The file from in which this node is located
     */
    std::string_view get_filename() const noexcept {
        auto ui_pos = location_.find(':');
        return std::string_view(
            location_.c_str(), ui_pos == location_.npos ? location_.size() : ui_pos);
    }

    /// Returns the line number on which this node is located.
    /** \return The line number on which this node is located
     */
    std::size_t get_line_number() const noexcept {
        std::size_t ui_line = std::numeric_limits<std::size_t>::max();
        auto        ui_pos  = location_.find(':');
        if (ui_pos != location_.npos && ui_pos < location_.size() - 1)
            utils::from_string(location_.substr(ui_pos + 1), ui_line);
        return ui_line;
    }

    /// Returns the line number on which this node's value is located.
    /** \return The line number on which this node's value is located
     */
    std::size_t get_value_line_number() const noexcept {
        std::size_t ui_line = std::numeric_limits<std::size_t>::max();
        auto        ui_pos  = value_location_.find(':');
        if (ui_pos != value_location_.npos && ui_pos < value_location_.size() - 1)
            utils::from_string(value_location_.substr(ui_pos + 1), ui_line);
        return ui_line;
    }

    /// Returns this node's name.
    /** \return This node's name
     */
    std::string_view get_name() const noexcept {
        return name_;
    }

    /// Returns this node's value as string.
    /** \return This node's value as string
     *   \note Returns an empty string if none
     */
    std::string_view get_value() const noexcept {
        accessed_ = true;
        return value_;
    }

    /// Returns this node's value as string, or a default value if empty.
    /** \param fallback The fallback value if the node has no value
     *   \return This node's value as string, or a default value if empty
     */
    std::string_view get_value_or(std::string_view fallback) const noexcept {
        accessed_ = true;
        if (value_.empty())
            return fallback;
        else
            return value_;
    }

    /// Returns this node's value converted to a specific type.
    /** \return This node's value converted to a specific type
     *   \note Will throw if the value could not be converted. Use get_value_or()
     *         to avoid throwing.
     */
    template<typename T>
    T get_value() const {
        accessed_ = true;
        T value{};
        if (!utils::from_string(value_, value)) {
            throw utils::exception(
                std::string(get_location()) + ": could not parse value for '" + std::string(name_) +
                "': '" + std::string(value_) + "'");
        }

        return value;
    }

    /// Returns this node's value converted to a specific type, or a default value.
    /** \return This node's value converted to a specific type, or a default value
     *   \note Will return the default value if the value could not be converted.
     */
    template<typename T>
    T get_value_or(T fallback) const noexcept {
        accessed_ = true;
        T value{};
        if (!utils::from_string(value_, value))
            value = fallback;

        return value;
    }

    /// Set this node's location.
    /** \param location The new location
     */
    void set_location(std::string location) noexcept {
        location_ = std::move(location);
    }

    /// Set this node's value location.
    /** \param location The new value location
     */
    void set_value_location(std::string location) noexcept {
        value_location_ = std::move(location);
    }

    /// Set this node's name.
    /** \param name The new name
     */
    void set_name(std::string name) noexcept {
        name_ = std::move(name);
    }

    /// Set this node's value.
    /** \param value The new value
     */
    void set_value(std::string value) noexcept {
        value_ = std::move(value);
    }

    /// Flag this node as "not accessed" for later warnings.
    void mark_as_not_accessed() const {
        accessed_ = false;
    }

    /// Flag this node as "fully accessed" for later warnings; no check will be done.
    void bypass_access_check() const {
        access_bypass_ = true;
    }

    /// Check if this node was accessed by the parser.
    /** \return 'true' if this node was accessed by the parser, 'false' otherwise.
     */
    bool was_accessed() const {
        return accessed_;
    }

    /// Check if this node should be bypassed for access checks.
    /** \return 'true' if this node should be bypassed, 'false' otherwise.
     */
    bool is_access_check_bypassed() const {
        return access_bypass_;
    }

protected:
    std::string name_;
    std::string value_;
    std::string location_;
    std::string value_location_;

    mutable bool accessed_      = false;
    mutable bool access_bypass_ = false;
};

/// An node in a layout file
/** This is a format-agnostic representation of a GUI layout, as read
 *   for example from an XML or YAML file. The GUI uses this class to de-couple
 *   the layout parsing format (XML, YAML, etc) from the actual parsed layout.
 */
class layout_node : public layout_attribute {
public:
    layout_node()                   = default;
    layout_node(const layout_node&) = default;
    layout_node(layout_node&&)      = default;
    layout_node& operator=(const layout_node&) = default;
    layout_node& operator=(layout_node&&) = default;

    using child_list    = std::vector<layout_node>;
    using children_view = utils::view::
        adaptor<const child_list, utils::view::standard_dereferencer, utils::view::no_filter>;

    /// Returns the number of children of this node.
    /** \return The number of children of this node
     */
    std::size_t get_children_count() const noexcept {
        return child_list_.size();
    }

    /// Returns a specific child of this node, by index
    /** \param uiIndex The index (starting from 0) of this child
     *   \return The child at the specified index
     */
    const layout_node& get_child(std::size_t ui_index) const noexcept {
        accessed_ = true;
        return child_list_[ui_index];
    }

    /// Returns a view to the list of children.
    /** \return A view to the list of children
     */
    children_view get_children() const noexcept {
        accessed_ = true;
        return children_view(child_list_);
    }

    template<typename BaseIterator>
    struct name_filter {
        std::string_view filter;

        bool is_included(const BaseIterator& iter) const {
            return iter->get_name() == filter;
        }
    };

    using filtered_children_view =
        utils::view::adaptor<const child_list, utils::view::standard_dereferencer, name_filter>;

    /// Returns a view to the list of children with a given name.
    /** \param name The name to look for
     *   \return A view to the list of children with a given name
     */
    filtered_children_view get_children(std::string_view name) const noexcept {
        accessed_ = true;
        return filtered_children_view(child_list_, {}, {name});
    }

    /// Returns the first child with a given name, or null if none.
    /** \param name The name to look for
     *   \return The first child with a given name, or null if none
     */
    const layout_node* try_get_child(std::string_view name) const noexcept {
        accessed_ = true;
        for (const layout_node& node : get_children(name)) {
            return &node;
        }

        return nullptr;
    }

    /// Returns the first child with a given name, and throws if none.
    /** \param name The name to look for
     *   \return The first child with a given name, and throws if none
     *   \note Will throw if no child is found with this name. Use try_get_child()
     *         to avoid throwing.
     */
    const layout_node& get_child(std::string_view name) const {
        accessed_ = true;
        if (const layout_node* p_child = try_get_child(name))
            return *p_child;
        else
            throw utils::exception(
                std::string(get_location()) + ": no child found with name '" + std::string(name) +
                "' in '" + std::string(name_) + "'");
    }

    /// Checks if at least one child exists with the given name
    /** \param name The name to look for
     *   \return 'true' if at least one child exists, 'false' otherwise
     */
    bool has_child(std::string_view name) const noexcept {
        accessed_ = true;
        return try_get_child(name) != nullptr;
    }

    /// Returns the attribute with the provided name, or null if none.
    /** \param name The name to look for
     *   \return The attribute with the provided name, or null if none
     *   \note Will throw if no child is found with this name. Use get_attribute_value_or()
     *         to avoid throwing.
     */
    const layout_attribute* try_get_attribute(std::string_view name) const noexcept {
        accessed_ = true;
        for (const layout_attribute& node : attr_list_) {
            if (node.get_name() == name)
                return &node;
        }

        return nullptr;
    }

    /// Returns the value of the first child with the provided name, throws if none.
    /** \param name The name to look for
     *   \return The value of the first child with the provided name.
     *   \note Will throw if no attribute is found with this name. Use try_get_attribute()
     *         to avoid throwing.
     */
    const layout_attribute& get_attribute(std::string_view name) const {
        accessed_ = true;
        if (const layout_attribute* p_attr = try_get_attribute(name))
            return *p_attr;
        else
            throw utils::exception(
                std::string(get_location()) + ": no attribute found with name '" +
                std::string(name) + "' in '" + std::string(name_) + "'");
    }

    /// Checks if a given attribute has been specified
    /** \param name The name to look for
     *   \return 'true' if attribute is specified, 'false' otherwise
     */
    bool has_attribute(std::string_view name) const noexcept {
        accessed_ = true;
        return try_get_attribute(name) != nullptr;
    }

    /// Returns the value of the attribute with the provided name, throws if none.
    /** \param name The name to look for
     *   \return The value of the attribute with the provided name.
     *   \note Will throw if no attribute is found with this name. Use get_attribute_value_or()
     *         to avoid throwing.
     */
    std::string_view get_attribute_value(std::string_view name) const {
        accessed_ = true;
        return get_attribute(name).get_value();
    }

    /// Returns the value of the attribute with the provided name, throws if none.
    /** \param name The name to look for
     *   \return The value of the attribute with the provided name.
     *   \note Will throw if no attribute is found with this name. Use get_attribute_value_or()
     *         to avoid throwing.
     */
    template<typename T>
    T get_attribute_value(std::string_view name) const {
        accessed_ = true;
        return get_attribute(name).get_value<T>();
    }

    /// Returns the value of the attribute with the provided name, or a default value if none.
    /** \param name     The name to look for
     *   \param fallback The fallback value
     *   \return The value of the attribute with the provided name, or a default value if none
     */
    std::string_view
    get_attribute_value_or(std::string_view name, std::string_view fallback) const noexcept {
        accessed_ = true;
        if (const auto* p_attr = try_get_attribute(name))
            return p_attr->get_value_or(fallback);
        else
            return fallback;
    }

    /// Returns the value of the attribute with the provided name, or a default value if none.
    /** \param name     The name to look for
     *   \param fallback The fallback value
     *   \return The value of the attribute with the provided name, or a default value if none
     */
    template<typename T>
    T get_attribute_value_or(std::string_view name, T fallback) const noexcept {
        accessed_ = true;
        if (const auto* p_attr = try_get_attribute(name))
            return p_attr->get_value_or<T>(fallback);
        else
            return fallback;
    }

    using attribute_list = std::vector<layout_attribute>;
    using attribute_view = utils::view::
        adaptor<const attribute_list, utils::view::standard_dereferencer, utils::view::no_filter>;

    /// Returns a view to the list of attributes.
    /** \return A view to the list of attributes
     */
    attribute_view get_attributes() const noexcept {
        accessed_ = true;
        return attribute_view(attr_list_);
    }

    /// Add a new child to this node
    /** \return A reference to the added child
     */
    layout_node& add_child() {
        return child_list_.emplace_back();
    }

    /// Add a new attribute to this node
    /** \return A reference to the added attribute
     */
    layout_attribute& add_attribute() {
        return attr_list_.emplace_back();
    }

    /// Returns the value of the attribute with the provided name, or set it if none.
    /** \param name  The name to look for
     *   \param sValue The value to set if the attribute is missing
     *   \return The value of the attribute with the provided name.
     *   \note This will modify the layout node object if the value is missing. If you need
     *         a non-modifying alternative, use get_attribute_value_or().
     */
    std::string_view get_or_set_attribute_value(std::string_view name, std::string_view value) {
        accessed_ = true;
        if (const auto* p_attr = try_get_attribute(name))
            return p_attr->get_value();
        else {
            auto& attr = add_attribute();
            attr.set_name(std::string(name));
            attr.set_value(std::string(value));
            return value;
        }
    }

private:
    child_list     child_list_;
    attribute_list attr_list_;
};

template<>
inline std::string layout_attribute::get_value<std::string>() const {
    accessed_ = true;
    return value_;
}

template<>
inline bool layout_attribute::get_value<bool>() const {
    accessed_ = true;
    if (value_ == "true")
        return true;
    else if (value_ == "false")
        return false;
    else {
        throw utils::exception(
            std::string(get_location()) + ": could not parse value for '" + std::string(name_) +
            "': '" + std::string(value_) + "'");
    }
}

} // namespace lxgui::gui

#endif
