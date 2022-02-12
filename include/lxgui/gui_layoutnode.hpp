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
        return s_location_;
    }

    /// Returns this node's value location in the file as {file}:{line}.
    /** \return This node's value location in the file as {file}:{line}
     */
    std::string_view get_value_location() const noexcept {
        return s_value_location_;
    }

    /// Returns the file from in which this node is located.
    /** \return The file from in which this node is located
     */
    std::string_view get_filename() const noexcept {
        auto ui_pos = s_location_.find(':');
        return std::string_view(
            s_location_.c_str(), ui_pos == s_location_.npos ? s_location_.size() : ui_pos);
    }

    /// Returns the line number on which this node is located.
    /** \return The line number on which this node is located
     */
    std::size_t get_line_number() const noexcept {
        std::size_t ui_line = std::numeric_limits<std::size_t>::max();
        auto        ui_pos  = s_location_.find(':');
        if (ui_pos != s_location_.npos && ui_pos < s_location_.size() - 1)
            utils::from_string(s_location_.substr(ui_pos + 1), ui_line);
        return ui_line;
    }

    /// Returns the line number on which this node's value is located.
    /** \return The line number on which this node's value is located
     */
    std::size_t get_value_line_number() const noexcept {
        std::size_t ui_line = std::numeric_limits<std::size_t>::max();
        auto        ui_pos  = s_value_location_.find(':');
        if (ui_pos != s_value_location_.npos && ui_pos < s_value_location_.size() - 1)
            utils::from_string(s_value_location_.substr(ui_pos + 1), ui_line);
        return ui_line;
    }

    /// Returns this node's name.
    /** \return This node's name
     */
    std::string_view get_name() const noexcept {
        return s_name_;
    }

    /// Returns this node's value as string.
    /** \return This node's value as string
     *   \note Returns an empty string if none
     */
    std::string_view get_value() const noexcept {
        b_accessed_ = true;
        return s_value_;
    }

    /// Returns this node's value as string, or a default value if empty.
    /** \return This node's value as string, or a default value if empty
     *   \note Returns an empty string if none
     */
    std::string_view get_value_or(std::string_view s_fallback) const noexcept {
        b_accessed_ = true;
        if (s_value_.empty())
            return s_fallback;
        else
            return s_value_;
    }

    /// Returns this node's value converted to a specific type.
    /** \return This node's value converted to a specific type
     *   \note Will throw if the value could not be converted. Use get_value_or()
     *         to avoid throwing.
     */
    template<typename T>
    T get_value() const {
        b_accessed_ = true;
        T m_value{};
        if (!utils::from_string(s_value_, m_value)) {
            throw utils::exception(
                std::string(get_location()) + ": could not parse value for '" +
                std::string(s_name_) + "': '" + std::string(s_value_) + "'");
        }

        return m_value;
    }

    /// Returns this node's value converted to a specific type, or a default value.
    /** \return This node's value converted to a specific type, or a default value
     *   \note Will return the default value if the value could not be converted.
     */
    template<typename T>
    T get_value_or(T m_fallback) const noexcept {
        b_accessed_ = true;
        T m_value{};
        if (!utils::from_string(s_value_, m_value))
            m_value = m_fallback;

        return m_value;
    }

    /// Set this node's location.
    /** \param sLocation The new location
     */
    void set_location(std::string s_location) noexcept {
        s_location_ = std::move(s_location);
    }

    /// Set this node's value location.
    /** \param sLocation The new value location
     */
    void set_value_location(std::string s_location) noexcept {
        s_value_location_ = std::move(s_location);
    }

    /// Set this node's name.
    /** \param sName The new name
     */
    void set_name(std::string s_name) noexcept {
        s_name_ = std::move(s_name);
    }

    /// Set this node's value.
    /** \param sValue The new value
     */
    void set_value(std::string s_value) noexcept {
        s_value_ = std::move(s_value);
    }

    /// Flag this node as "not accessed" for later warnings.
    void mark_as_not_accessed() const {
        b_accessed_ = false;
    }

    /// Flag this node as "fully accessed" for later warnings; no check will be done.
    void bypass_access_check() const {
        b_access_bypass_ = true;
    }

    /// Check if this node was accessed by the parser.
    /** \return 'true' if this node was accessed by the parser, 'false' otherwise.
     */
    bool was_accessed() const {
        return b_accessed_;
    }

    /// Check if this node should be bypassed for access checks.
    /** \return 'true' if this node should be bypassed, 'false' otherwise.
     */
    bool is_access_check_bypassed() const {
        return b_access_bypass_;
    }

protected:
    std::string s_name_;
    std::string s_value_;
    std::string s_location_;
    std::string s_value_location_;

    mutable bool b_accessed_     = false;
    mutable bool b_access_bypass_ = false;
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
        return l_child_list_.size();
    }

    /// Returns a specific child of this node, by index
    /** \param uiIndex The index (starting from 0) of this child
     *   \return The child at the specified index
     */
    const layout_node& get_child(std::size_t ui_index) const noexcept {
        b_accessed_ = true;
        return l_child_list_[ui_index];
    }

    /// Returns a view to the list of children.
    /** \return A view to the list of children
     */
    children_view get_children() const noexcept {
        b_accessed_ = true;
        return children_view(l_child_list_);
    }

    template<typename BaseIterator>
    struct name_filter {
        std::string_view s_filter;

        bool is_included(const BaseIterator& m_iter) const {
            return m_iter->get_name() == s_filter;
        }
    };

    using filtered_children_view =
        utils::view::adaptor<const child_list, utils::view::standard_dereferencer, name_filter>;

    /// Returns a view to the list of children with a given name.
    /** \param sName The name to look for
     *   \return A view to the list of children with a given name
     */
    filtered_children_view get_children(std::string_view s_name) const noexcept {
        b_accessed_ = true;
        return filtered_children_view(l_child_list_, {}, {s_name});
    }

    /// Returns the first child with a given name, or null if none.
    /** \param sName The name to look for
     *   \return The first child with a given name, or null if none
     */
    const layout_node* try_get_child(std::string_view s_name) const noexcept {
        b_accessed_ = true;
        for (const layout_node& m_node : get_children(s_name)) {
            return &m_node;
        }

        return nullptr;
    }

    /// Returns the first child with a given name, and throws if none.
    /** \param sName The name to look for
     *   \return The first child with a given name, and throws if none
     *   \note Will throw if no child is found with this name. Use try_get_child()
     *         to avoid throwing.
     */
    const layout_node& get_child(std::string_view s_name) const {
        b_accessed_ = true;
        if (const layout_node* p_child = try_get_child(s_name))
            return *p_child;
        else
            throw utils::exception(
                std::string(get_location()) + ": no child found with name '" + std::string(s_name) +
                "' in '" + std::string(s_name_) + "'");
    }

    /// Checks if at least one child exists with the given name
    /** \param sName The name to look for
     *   \return 'true' if at least one child exists, 'false' otherwise
     */
    bool has_child(std::string_view s_name) const noexcept {
        b_accessed_ = true;
        return try_get_child(s_name) != nullptr;
    }

    /// Returns the attribute with the provided name, or null if none.
    /** \param sName The name to look for
     *   \return The attribute with the provided name, or null if none
     *   \note Will throw if no child is found with this name. Use get_attribute_value_or()
     *         to avoid throwing.
     */
    const layout_attribute* try_get_attribute(std::string_view s_name) const noexcept {
        b_accessed_ = true;
        for (const layout_attribute& m_node : l_attr_list_) {
            if (m_node.get_name() == s_name)
                return &m_node;
        }

        return nullptr;
    }

    /// Returns the value of the first child with the provided name, throws if none.
    /** \param sName The name to look for
     *   \return The value of the first child with the provided name.
     *   \note Will throw if no attribute is found with this name. Use try_get_attribute()
     *         to avoid throwing.
     */
    const layout_attribute& get_attribute(std::string_view s_name) const {
        b_accessed_ = true;
        if (const layout_attribute* p_attr = try_get_attribute(s_name))
            return *p_attr;
        else
            throw utils::exception(
                std::string(get_location()) + ": no attribute found with name '" +
                std::string(s_name) + "' in '" + std::string(s_name_) + "'");
    }

    /// Checks if a given attribute has been specified
    /** \param sName The name to look for
     *   \return 'true' if attribute is specified, 'false' otherwise
     */
    bool has_attribute(std::string_view s_name) const noexcept {
        b_accessed_ = true;
        return try_get_attribute(s_name) != nullptr;
    }

    /// Returns the value of the attribute with the provided name, throws if none.
    /** \param sName The name to look for
     *   \return The value of the attribute with the provided name.
     *   \note Will throw if no attribute is found with this name. Use get_attribute_value_or()
     *         to avoid throwing.
     */
    std::string_view get_attribute_value(std::string_view s_name) const {
        b_accessed_ = true;
        return get_attribute(s_name).get_value();
    }

    /// Returns the value of the attribute with the provided name, throws if none.
    /** \param sName The name to look for
     *   \return The value of the attribute with the provided name.
     *   \note Will throw if no attribute is found with this name. Use get_attribute_value_or()
     *         to avoid throwing.
     */
    template<typename T>
    T get_attribute_value(std::string_view s_name) const {
        b_accessed_ = true;
        return get_attribute(s_name).get_value<T>();
    }

    /// Returns the value of the attribute with the provided name, or a default value if none.
    /** \param sName     The name to look for
     *   \param sFallback The fallback
     *   \return The value of the attribute with the provided name, or a default value if none
     */
    std::string_view
    get_attribute_value_or(std::string_view s_name, std::string_view s_fallback) const noexcept {
        b_accessed_ = true;
        if (const auto* p_attr = try_get_attribute(s_name))
            return p_attr->get_value_or(s_fallback);
        else
            return s_fallback;
    }

    /// Returns the value of the attribute with the provided name, or a default value if none.
    /** \param sName     The name to look for
     *   \param mFallback The fallback
     *   \return The value of the attribute with the provided name, or a default value if none
     */
    template<typename T>
    T get_attribute_value_or(std::string_view s_name, T m_fallback) const noexcept {
        b_accessed_ = true;
        if (const auto* p_attr = try_get_attribute(s_name))
            return p_attr->get_value_or<T>(m_fallback);
        else
            return m_fallback;
    }

    using attribute_list = std::vector<layout_attribute>;
    using attribute_view = utils::view::
        adaptor<const attribute_list, utils::view::standard_dereferencer, utils::view::no_filter>;

    /// Returns a view to the list of attributes.
    /** \return A view to the list of attributes
     */
    attribute_view get_attributes() const noexcept {
        b_accessed_ = true;
        return attribute_view(l_attr_list_);
    }

    /// Add a new child to this node
    /** \return A reference to the added child
     */
    layout_node& add_child() {
        return l_child_list_.emplace_back();
    }

    /// Add a new attribute to this node
    /** \return A reference to the added attribute
     */
    layout_attribute& add_attribute() {
        return l_attr_list_.emplace_back();
    }

    /// Returns the value of the attribute with the provided name, or set it if none.
    /** \param sName  The name to look for
     *   \param sValue The value to set if the attribute is missing
     *   \return The value of the attribute with the provided name.
     *   \note This will modify the layout node object if the value is missing. If you need
     *         a non-modifying alternative, use get_attribute_value_or().
     */
    std::string_view get_or_set_attribute_value(std::string_view s_name, std::string_view s_value) {
        b_accessed_ = true;
        if (const auto* p_attr = try_get_attribute(s_name))
            return p_attr->get_value();
        else {
            auto& m_attr = add_attribute();
            m_attr.set_name(std::string(s_name));
            m_attr.set_value(std::string(s_value));
            return s_value;
        }
    }

private:
    child_list     l_child_list_;
    attribute_list l_attr_list_;
};

template<>
inline std::string layout_attribute::get_value<std::string>() const {
    b_accessed_ = true;
    return s_value_;
}

template<>
inline bool layout_attribute::get_value<bool>() const {
    b_accessed_ = true;
    if (s_value_ == "true")
        return true;
    else if (s_value_ == "false")
        return false;
    else {
        throw utils::exception(
            std::string(get_location()) + ": could not parse value for '" + std::string(s_name_) +
            "': '" + std::string(s_value_) + "'");
    }
}

} // namespace lxgui::gui

#endif
