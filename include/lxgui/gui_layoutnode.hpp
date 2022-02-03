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

namespace lxgui { namespace gui {

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
        return sLocation_;
    }

    /// Returns this node's value location in the file as {file}:{line}.
    /** \return This node's value location in the file as {file}:{line}
     */
    std::string_view get_value_location() const noexcept {
        return sValueLocation_;
    }

    /// Returns the file from in which this node is located.
    /** \return The file from in which this node is located
     */
    std::string_view get_filename() const noexcept {
        auto uiPos = sLocation_.find(':');
        return std::string_view(
            sLocation_.c_str(), uiPos == sLocation_.npos ? sLocation_.size() : uiPos);
    }

    /// Returns the line number on which this node is located.
    /** \return The line number on which this node is located
     */
    std::size_t get_line_number() const noexcept {
        std::size_t uiLine = std::numeric_limits<std::size_t>::max();
        auto        uiPos  = sLocation_.find(':');
        if (uiPos != sLocation_.npos && uiPos < sLocation_.size() - 1)
            utils::from_string(sLocation_.substr(uiPos + 1), uiLine);
        return uiLine;
    }

    /// Returns the line number on which this node's value is located.
    /** \return The line number on which this node's value is located
     */
    std::size_t get_value_line_number() const noexcept {
        std::size_t uiLine = std::numeric_limits<std::size_t>::max();
        auto        uiPos  = sValueLocation_.find(':');
        if (uiPos != sValueLocation_.npos && uiPos < sValueLocation_.size() - 1)
            utils::from_string(sValueLocation_.substr(uiPos + 1), uiLine);
        return uiLine;
    }

    /// Returns this node's name.
    /** \return This node's name
     */
    std::string_view get_name() const noexcept {
        return sName_;
    }

    /// Returns this node's value as string.
    /** \return This node's value as string
     *   \note Returns an empty string if none
     */
    std::string_view get_value() const noexcept {
        bAccessed_ = true;
        return sValue_;
    }

    /// Returns this node's value as string, or a default value if empty.
    /** \return This node's value as string, or a default value if empty
     *   \note Returns an empty string if none
     */
    std::string_view get_value_or(std::string_view sFallback) const noexcept {
        bAccessed_ = true;
        if (sValue_.empty())
            return sFallback;
        else
            return sValue_;
    }

    /// Returns this node's value converted to a specific type.
    /** \return This node's value converted to a specific type
     *   \note Will throw if the value could not be converted. Use get_value_or()
     *         to avoid throwing.
     */
    template<typename T>
    T get_value() const {
        bAccessed_ = true;
        T mValue{};
        if (!utils::from_string(sValue_, mValue)) {
            throw utils::exception(
                std::string(get_location()) + ": could not parse value for '" +
                std::string(sName_) + "': '" + std::string(sValue_) + "'");
        }

        return mValue;
    }

    /// Returns this node's value converted to a specific type, or a default value.
    /** \return This node's value converted to a specific type, or a default value
     *   \note Will return the default value if the value could not be converted.
     */
    template<typename T>
    T get_value_or(T mFallback) const noexcept {
        bAccessed_ = true;
        T mValue{};
        if (!utils::from_string(sValue_, mValue))
            mValue = mFallback;

        return mValue;
    }

    /// Set this node's location.
    /** \param sLocation The new location
     */
    void set_location(std::string sLocation) noexcept {
        sLocation_ = std::move(sLocation);
    }

    /// Set this node's value location.
    /** \param sLocation The new value location
     */
    void set_value_location(std::string sLocation) noexcept {
        sValueLocation_ = std::move(sLocation);
    }

    /// Set this node's name.
    /** \param sName The new name
     */
    void set_name(std::string sName) noexcept {
        sName_ = std::move(sName);
    }

    /// Set this node's value.
    /** \param sValue The new value
     */
    void set_value(std::string sValue) noexcept {
        sValue_ = std::move(sValue);
    }

    /// Flag this node as "not accessed" for later warnings.
    void mark_as_not_accessed() const {
        bAccessed_ = false;
    }

    /// Flag this node as "fully accessed" for later warnings; no check will be done.
    void bypass_access_check() const {
        bAccessBypass_ = true;
    }

    /// Check if this node was accessed by the parser.
    /** \return 'true' if this node was accessed by the parser, 'false' otherwise.
     */
    bool was_accessed() const {
        return bAccessed_;
    }

    /// Check if this node should be bypassed for access checks.
    /** \return 'true' if this node should be bypassed, 'false' otherwise.
     */
    bool is_access_check_bypassed() const {
        return bAccessBypass_;
    }

protected:
    std::string sName_;
    std::string sValue_;
    std::string sLocation_;
    std::string sValueLocation_;

    mutable bool bAccessed_     = false;
    mutable bool bAccessBypass_ = false;
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
        return lChildList_.size();
    }

    /// Returns a specific child of this node, by index
    /** \param uiIndex The index (starting from 0) of this child
     *   \return The child at the specified index
     */
    const layout_node& get_child(std::size_t uiIndex) const noexcept {
        bAccessed_ = true;
        return lChildList_[uiIndex];
    }

    /// Returns a view to the list of children.
    /** \return A view to the list of children
     */
    children_view get_children() const noexcept {
        bAccessed_ = true;
        return children_view(lChildList_);
    }

    template<typename BaseIterator>
    struct name_filter {
        std::string_view sFilter;

        bool is_included(const BaseIterator& mIter) const {
            return mIter->get_name() == sFilter;
        }
    };

    using filtered_children_view =
        utils::view::adaptor<const child_list, utils::view::standard_dereferencer, name_filter>;

    /// Returns a view to the list of children with a given name.
    /** \param sName The name to look for
     *   \return A view to the list of children with a given name
     */
    filtered_children_view get_children(std::string_view sName) const noexcept {
        bAccessed_ = true;
        return filtered_children_view(lChildList_, {}, {sName});
    }

    /// Returns the first child with a given name, or null if none.
    /** \param sName The name to look for
     *   \return The first child with a given name, or null if none
     */
    const layout_node* try_get_child(std::string_view sName) const noexcept {
        bAccessed_ = true;
        for (const layout_node& mNode : get_children(sName)) {
            return &mNode;
        }

        return nullptr;
    }

    /// Returns the first child with a given name, and throws if none.
    /** \param sName The name to look for
     *   \return The first child with a given name, and throws if none
     *   \note Will throw if no child is found with this name. Use try_get_child()
     *         to avoid throwing.
     */
    const layout_node& get_child(std::string_view sName) const {
        bAccessed_ = true;
        if (const layout_node* pChild = try_get_child(sName))
            return *pChild;
        else
            throw utils::exception(
                std::string(get_location()) + ": no child found with name '" + std::string(sName) +
                "' in '" + std::string(sName_) + "'");
    }

    /// Checks if at least one child exists with the given name
    /** \param sName The name to look for
     *   \return 'true' if at least one child exists, 'false' otherwise
     */
    bool has_child(std::string_view sName) const noexcept {
        bAccessed_ = true;
        return try_get_child(sName) != nullptr;
    }

    /// Returns the attribute with the provided name, or null if none.
    /** \param sName The name to look for
     *   \return The attribute with the provided name, or null if none
     *   \note Will throw if no child is found with this name. Use get_attribute_value_or()
     *         to avoid throwing.
     */
    const layout_attribute* try_get_attribute(std::string_view sName) const noexcept {
        bAccessed_ = true;
        for (const layout_attribute& mNode : lAttrList_) {
            if (mNode.get_name() == sName)
                return &mNode;
        }

        return nullptr;
    }

    /// Returns the value of the first child with the provided name, throws if none.
    /** \param sName The name to look for
     *   \return The value of the first child with the provided name.
     *   \note Will throw if no attribute is found with this name. Use try_get_attribute()
     *         to avoid throwing.
     */
    const layout_attribute& get_attribute(std::string_view sName) const {
        bAccessed_ = true;
        if (const layout_attribute* pAttr = try_get_attribute(sName))
            return *pAttr;
        else
            throw utils::exception(
                std::string(get_location()) + ": no attribute found with name '" +
                std::string(sName) + "' in '" + std::string(sName_) + "'");
    }

    /// Checks if a given attribute has been specified
    /** \param sName The name to look for
     *   \return 'true' if attribute is specified, 'false' otherwise
     */
    bool has_attribute(std::string_view sName) const noexcept {
        bAccessed_ = true;
        return try_get_attribute(sName) != nullptr;
    }

    /// Returns the value of the attribute with the provided name, throws if none.
    /** \param sName The name to look for
     *   \return The value of the attribute with the provided name.
     *   \note Will throw if no attribute is found with this name. Use get_attribute_value_or()
     *         to avoid throwing.
     */
    std::string_view get_attribute_value(std::string_view sName) const {
        bAccessed_ = true;
        return get_attribute(sName).get_value();
    }

    /// Returns the value of the attribute with the provided name, throws if none.
    /** \param sName The name to look for
     *   \return The value of the attribute with the provided name.
     *   \note Will throw if no attribute is found with this name. Use get_attribute_value_or()
     *         to avoid throwing.
     */
    template<typename T>
    T get_attribute_value(std::string_view sName) const {
        bAccessed_ = true;
        return get_attribute(sName).get_value<T>();
    }

    /// Returns the value of the attribute with the provided name, or a default value if none.
    /** \param sName     The name to look for
     *   \param sFallback The fallback
     *   \return The value of the attribute with the provided name, or a default value if none
     */
    std::string_view
    get_attribute_value_or(std::string_view sName, std::string_view sFallback) const noexcept {
        bAccessed_ = true;
        if (const auto* pAttr = try_get_attribute(sName))
            return pAttr->get_value_or(sFallback);
        else
            return sFallback;
    }

    /// Returns the value of the attribute with the provided name, or a default value if none.
    /** \param sName     The name to look for
     *   \param mFallback The fallback
     *   \return The value of the attribute with the provided name, or a default value if none
     */
    template<typename T>
    T get_attribute_value_or(std::string_view sName, T mFallback) const noexcept {
        bAccessed_ = true;
        if (const auto* pAttr = try_get_attribute(sName))
            return pAttr->get_value_or<T>(mFallback);
        else
            return mFallback;
    }

    using attribute_list = std::vector<layout_attribute>;
    using attribute_view = utils::view::
        adaptor<const attribute_list, utils::view::standard_dereferencer, utils::view::no_filter>;

    /// Returns a view to the list of attributes.
    /** \return A view to the list of attributes
     */
    attribute_view get_attributes() const noexcept {
        bAccessed_ = true;
        return attribute_view(lAttrList_);
    }

    /// Add a new child to this node
    /** \return A reference to the added child
     */
    layout_node& add_child() {
        return lChildList_.emplace_back();
    }

    /// Add a new attribute to this node
    /** \return A reference to the added attribute
     */
    layout_attribute& add_attribute() {
        return lAttrList_.emplace_back();
    }

    /// Returns the value of the attribute with the provided name, or set it if none.
    /** \param sName  The name to look for
     *   \param sValue The value to set if the attribute is missing
     *   \return The value of the attribute with the provided name.
     *   \note This will modify the layout node object if the value is missing. If you need
     *         a non-modifying alternative, use get_attribute_value_or().
     */
    std::string_view get_or_set_attribute_value(std::string_view sName, std::string_view sValue) {
        bAccessed_ = true;
        if (const auto* pAttr = try_get_attribute(sName))
            return pAttr->get_value();
        else {
            auto& mAttr = add_attribute();
            mAttr.set_name(std::string(sName));
            mAttr.set_value(std::string(sValue));
            return sValue;
        }
    }

private:
    child_list     lChildList_;
    attribute_list lAttrList_;
};

template<>
inline std::string layout_attribute::get_value<std::string>() const {
    bAccessed_ = true;
    return sValue_;
}

template<>
inline bool layout_attribute::get_value<bool>() const {
    bAccessed_ = true;
    if (sValue_ == "true")
        return true;
    else if (sValue_ == "false")
        return false;
    else {
        throw utils::exception(
            std::string(get_location()) + ": could not parse value for '" + std::string(sName_) +
            "': '" + std::string(sValue_) + "'");
    }
}

}} // namespace lxgui::gui

#endif
