#ifndef GUI_ANCHOR_HPP
#define GUI_ANCHOR_HPP

#include <lxgui/utils.hpp>
#include <string>
#include "lxgui/gui_vector2.hpp"

namespace gui
{
    class uiobject;

    enum class anchor_type
    {
        ABS,
        REL
    };

    enum class anchor_point
    {
        TOPLEFT = 0,
        TOP,
        TOPRIGHT,
        RIGHT,
        BOTTOMRIGHT,
        BOTTOM,
        BOTTOMLEFT,
        LEFT,
        CENTER
    };

    enum class constraint
    {
        NONE,
        X,
        Y
    };

    /// Stores a position for a UI widget
    class anchor
    {
    public :

        /// Default constructor.
        anchor();

        /// Constructor.
        anchor(uiobject* pObj, anchor_point mPoint, const std::string& pParent, anchor_point mParentPoint);

        /// Returns this anchor absolute X (in pixel).
        /** \return This anchor absolute X.
        */
        int get_abs_x() const;

        /// Returns this anchor absolute Y (in pixel).
        /** \return This anchor absolute Y.
        */
        int get_abs_y() const;

        /// Returns this anchor's base widget.
        /** \return This anchor's base widget
        */
        const uiobject* get_object() const;

        /// Returns this anchor's parent widget.
        /** \return This anchor's parent widget
        */
        const uiobject* get_parent() const;

        /// Returns this anchor's parent's raw name (unmodified).
        /** \return This anchor's parent's raw name (unmodified)
        */
        const std::string& get_parent_raw_name() const;

        /// Returns this anchor's point.
        /** \return This anchor's point
        */
        anchor_point get_point() const;

        /// Returns this anchor's parent point.
        /** \return This anchor's parent point
        */
        anchor_point get_parent_point() const;

        /// Returns the type of this anchor (abs or rel).
        /** \return The type of this anchor (abs or rel)
        */
        anchor_type get_type() const;

        /// Returns this anchor's absolute horizontal offset.
        /** \return This anchor's absolute horizontal offset
        */
        int get_abs_offset_x() const;

        /// Returns this anchor's absolute vertical offset.
        /** \return This anchor's absolute vertical offset
        */
        int get_abs_offset_y() const;

        /// Returns this anchor's absolute offset.
        /** \return This anchor's absolute offset
        */
        vector2i get_abs_offset() const;

        /// Returns this anchor's relative horizontal offset.
        /** \return This anchor's relative horizontal offset
        */
        float get_rel_offset_x() const;

        /// Returns this anchor's relative vertical offset.
        /** \return This anchor's relative vertical offset
        */
        float get_rel_offset_y() const;

        /// Returns this anchor's relative offset.
        /** \return This anchor's relative offset
        */
        vector2f get_rel_offset() const;

        /// Sets this anchor's base widget.
        /** \param pObj The new base widget
        */
        void set_object(uiobject* pObj);

        /// Sets this anchor's parent's raw name.
        /** \param sName The parent's raw name
        */
        void set_parent_raw_name(const std::string& sName);

        /// Sets this anchor's point.
        /** \param mPoint The new point
        */
        void set_point(anchor_point mPoint);

        /// Sets this anchor's parent point.
        /** \param mParentPoint The new parent point
        */
        void set_parent_point(anchor_point mParentPoint);

        /// Sets this anchor's absolute offset.
        /** \param iX The new horizontal offset
        *   \param iY The new vertical offset
        */
        void set_abs_offset(int iX, int iY);

        /// Sets this anchor's absolute offset.
        /** \param mOffset The new offset
        */
        void set_abs_offset(const vector2i& mOffset);

        /// Sets this anchor's relative offset.
        /** \param fX The new horizontal offset
        *   \param fY The new vertical offset
        */
        void set_rel_offset(float fX, float fY);

        /// Sets this anchor's relative offset.
        /** \param mOffset The new offset
        */
        void set_rel_offset(const vector2f& mOffset);

        /// Prints all relevant information about this anchor in a string.
        /** \param sTab The offset to give to all lines
        *   \return All relevant information about this anchor
        */
        std::string serialize(const std::string& sTab) const;

        /// get_s the anchor parent from the parent string.
        void update_parent() const;

        /// Returns the name of an anchor point.
        /** \param mPoint The anchor point
        */
        static std::string get_string_point(anchor_point mPoint);

        /// Returns the anchor point from its name.
        /** \param sPoint The name of the anchor point
        */
        static anchor_point get_anchor_point(const std::string& sPoint);

    private :

        const uiobject* pObj_;
        anchor_point    mParentPoint_;
        anchor_point    mPoint_;
        anchor_type     mType_;

        int   iAbsOffX_, iAbsOffY_;
        float fRelOffX_, fRelOffY_;

        mutable int iParentWidth_, iParentHeight_;

        mutable const uiobject* pParent_;
        mutable std::string     sParent_;
        mutable bool            bParentUpdated_;
    };
}

#endif
