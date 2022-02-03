#ifndef LXGUI_GUI_ANCHOR_HPP
#define LXGUI_GUI_ANCHOR_HPP

#include "lxgui/lxgui.hpp"
#include "lxgui/utils.hpp"
#include "lxgui/utils_observer.hpp"
#include "lxgui/gui_vector2.hpp"

#include <string>

namespace lxgui {
namespace gui
{
    class region;

    enum class anchor_type
    {
        ABS,
        REL
    };

    enum class anchor_point
    {
        TOP_LEFT = 0,
        TOP,
        TOP_RIGHT,
        RIGHT,
        BOTTOM_RIGHT,
        BOTTOM,
        BOTTOM_LEFT,
        LEFT,
        CENTER
    };

    enum class constraint
    {
        NONE,
        X,
        Y
    };

    /// Stores a position for a UI region
    struct anchor_data
    {
        anchor_data(anchor_point mInputPoint) :
            mPoint(mInputPoint), sParent("$default"), mParentPoint(mInputPoint) {}

        anchor_data(anchor_point mInputPoint, const std::string& sInputParent) :
            mPoint(mInputPoint), sParent(sInputParent), mParentPoint(mInputPoint) {}

        anchor_data(anchor_point mInputPoint, const std::string& sInputParent,
            anchor_point mInputParentPoint) :
            mPoint(mInputPoint), sParent(sInputParent), mParentPoint(mInputParentPoint) {}

        anchor_data(anchor_point mInputPoint, const std::string& sInputParent,
            anchor_point mInputParentPoint, const vector2f& mInputOffset,
            anchor_type mInputType = anchor_type::ABS) :
            mPoint(mInputPoint), sParent(sInputParent), mParentPoint(mInputParentPoint),
            mOffset(mInputOffset), mType(mInputType) {}

        anchor_data(anchor_point mInputPoint, const std::string& sInputParent,
            const vector2f& mInputOffset,
            anchor_type mInputType = anchor_type::ABS) :
            mPoint(mInputPoint), sParent(sInputParent), mParentPoint(mInputPoint),
            mOffset(mInputOffset), mType(mInputType) {}

        anchor_data(anchor_point mInputPoint, const vector2f& mInputOffset,
            anchor_type mInputType = anchor_type::ABS) :
            mPoint(mInputPoint), sParent("$default"), mParentPoint(mInputPoint),
            mOffset(mInputOffset), mType(mInputType) {}

        anchor_data(anchor_point mInputPoint, anchor_point mInputParentPoint,
            const vector2f& mInputOffset, anchor_type mInputType = anchor_type::ABS) :
            mPoint(mInputPoint), sParent("$default"), mParentPoint(mInputParentPoint),
            mOffset(mInputOffset), mType(mInputType) {}

        anchor_data(anchor_point mInputPoint, anchor_point mInputParentPoint) :
            mPoint(mInputPoint), sParent("$default"), mParentPoint(mInputParentPoint) {}

        anchor_point mPoint = anchor_point::TOP_LEFT;
        std::string  sParent;
        anchor_point mParentPoint = anchor_point::TOP_LEFT;
        vector2f     mOffset;
        anchor_type  mType = anchor_type::ABS;
    };

    /// Stores a position for a UI region
    class anchor : private anchor_data
    {
    public :
        using anchor_data::mPoint;
        using anchor_data::mParentPoint;
        using anchor_data::mOffset;
        using anchor_data::mType;

        /// Constructor.
        anchor(region& mObject, const anchor_data& mAnchor);

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
        vector2f get_point(const region& mObject) const;

        /// Returns this anchor's parent region.
        /** \return This anchor's parent region
        */
        const utils::observer_ptr<region>& get_parent() { return pParent_; }

        /// Returns this anchor's parent region.
        /** \return This anchor's parent region
        */
        utils::observer_ptr<const region> get_parent() const { return pParent_; }

        /// Prints all relevant information about this anchor in a string.
        /** \param sTab The offset to give to all lines
        *   \return All relevant information about this anchor
        */
        std::string serialize(const std::string& sTab) const;

        /// Returns the raw data used for this anchor.
        /** \return The raw data used for this anchor
        */
        const anchor_data& get_data() const { return *this; }

        /// Returns the name of an anchor point.
        /** \param mPoint The anchor point
        */
        static std::string get_string_point(anchor_point mPoint);

        /// Returns the anchor point from its name.
        /** \param sPoint The name of the anchor point
        */
        static anchor_point get_anchor_point(const std::string& sPoint);

    private :

        /// Update the anchor parent object from the parent string.
        /** \param mObject The object owning this anchor
        */
        void update_parent_(region& mObject);

        utils::observer_ptr<region> pParent_ = nullptr;
    };
}
}

#endif
