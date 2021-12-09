#ifndef LXGUI_GUI_UIOBJECT_HPP
#define LXGUI_GUI_UIOBJECT_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_vector2.hpp"
#include "lxgui/gui_bounds2.hpp"
#include "lxgui/gui_color.hpp"
#include "lxgui/gui_strata.hpp"
#include "lxgui/gui_exception.hpp"

#include <lxgui/xml.hpp>

#include <lxgui/utils_maths.hpp>
#include <lxgui/utils_observer.hpp>

#include <array>
#include <unordered_map>
#include <vector>
#include <optional>

#include <sol/object.hpp>

namespace sol
{
    class state;
}

namespace lxgui {
namespace gui
{
    struct addon;
    class manager;

    class frame;
    class frame_renderer;

    /// The base class of all elements in the GUI.
    /** Objects of this class offers core functionalities needed by every element
    *   of the interface. They have a name, and a corresponding variable created
    *   in Lua to access them. They can have a parent #lxgui::gui::frame. They can be
    *   placed on the screen at an absolute position, or relative to other uiobjects.
    *   They can be shown or hidden.
    *
    *   Apart form this, a uiobject does not contain anything, nor can it display
    *   anything on the screen. Any functionality beyond the list above is implemented
    *   in specialized subclasses (see the full list below).
    *
    *   __Interaction between C++, Lua, and XML.__ When a uiobject is created,
    *   it must be given a name, for example `"PlayerHealthBar"`. For as long as the
    *   object lives, this name will be used to refer to it. In particular, as soon
    *   as the object is created, regardless of whether this was done in C++, XML, or
    *   Lua, a new variable will be created in the Lua state with the exact same name,
    *   `PlayerHealthBar`. This variable is a reference to the uiobject, and can
    *   be used to interact with it dynamically. Because of this, each object must have
    *   a unique name, otherwise it could not be accessible from Lua.
    *
    *   Note: Although you can destroy this Lua variable by setting it to nil, this is
    *   not recommended: the object will _not_ be destroyed (nor garbage-collected)
    *   because it still exists in the C++ memory space. The only way to truly destroy
    *   an object from Lua is to call `delete_frame` (for frames only). Destroying and
    *   creating objects has a cost however. If the object is likely to reappear later
    *   with the same content, simply hide it and show it again later on. If the
    *   content may change, you can also recycle the object, i.e., keep it alive and
    *   simply change its content when it later reappears.
    *
    *   Deleting an object from C++ is done using uiobject::destroy.
    *   This will automatically delete all references to this object in Lua as well.
    *
    *   Finally, note that objects do not need to be explicitly destroyed: they will
    *   automatically be destroyed when their parent is itself destroyed (see below).
    *   Only use explicit destruction when absolutely necessary.
    *
    *   __Parent-child relationship.__ Parents of uiobjects are frames. See
    *   the #lxgui::gui::frame class documentation for more information. One important
    *   aspect of the parent-child relationship is related to the object name. If a
    *   uiobject has a parent, it can be given a name starting with `"$parent"`.
    *   The name of the parent will automatically replace the `"$parent"` string.
    *   For example, if an object is named `"$parentButton"` and its parent is named
    *   `"ErrorMessage"`, the final name of the object will be `"ErrorMessageButton"`.
    *   It can be accessed from the Lua state as `ErrorMessageButton`, or as
    *   `ErrorMessage.Button`. Note that this is totally dynamic: if you later change
    *   the parent of this button to be another frame, for example `"ExitDialog"`
    *   its name will naturally change to `"ExitDialogButton"`, and it can be accessed
    *   from Lua as `ExitDialogButton`, or as `ExitDialog.Button`. This is particularly
    *   powerful for writing generic code which does not rely on the full names of
    *   objects, only on their child-parent relationship.
    *
    *   __Positioning.__ uiobjects have a position on the screen, but this is
    *   not parametrized as a simple pair of X and Y coordinates. Instead, objects
    *   are positioned based on a list of "anchors". Anchors are links between
    *   objects, which force one edge or one corner of a given object to match with
    *   the edge or corner of another object. For example, given two objects A and B,
    *   you can create an anchor that links the top-left corner of A to the top-left
    *   corner of B. The position of A will automatically be linked to the position of
    *   B, hence if B moves, A will follow. To further refine this positioning, you
    *   can specify anchor offsets: for example, you may want A's top-left corner to
    *   be shifted from B's top-left corner by two pixels in the X direction, and
    *   five in the Y direction. This offset can be defined either as an absolute
    *   number of pixels, or as a relative fraction of the size of the object being
    *   anchored to. For example, you can specify that A's top-left corner links to
    *   B's top-left corner, with an horizontal offset equal to 30% of B's width.
    *   Read the "Anchors" section below for more information.
    *
    *   An object which has no anchor will be considered "invalid" and will not be
    *   displayed.
    *
    *   __Sizing.__ There are two ways to specify the size of a uiobject. The
    *   first and most straightforward approach is to directly set its width and/or
    *   height. This must be specified as an absolute number of pixels. The second
    *   and more versatile method is to use more than one anchor for opposite sides
    *   of the object, for example an anchor for the "left" and another for the
    *   "right" edge. This will implicitly give a width to the object, depending on
    *   the position of the other objects to which it is anchored. Anchors will always
    *   override the absolute width and height of an object if they provide any
    *   constraint on the extents of the object in a given dimension.
    *
    *   An object which has neither a fixed absolute size, nor has it size implicitly
    *   constrained by anchors, is considered "invalid" and will not be displayed.
    *
    *   __Anchors.__ There are nine available anchor points:
    *
    *   - `TOPLEFT`: constrains the max Y and min X.
    *   - `TOPRIGHT`: constrains the max Y and max X.
    *   - `BOTTOMLEFT`: constrains the min Y and min X.
    *   - `BOTTOMRIGH`: constrains the min Y and max X.
    *   - `LEFT`: constrains the min X and the midpoint in Y.
    *   - `RIGHT`: constrains the max X and the midpoint in Y.
    *   - `TOP`: constrains the max Y and the midpoint in X.
    *   - `BOTTOM`: constrains the min Y and the midpoint in X.
    *   - `CENTER`: constrains the midpoint in X and Y.
    *
    *   If you specify two constraints on the same point (for example: `TOPLEFT`
    *   and `BOTTOMLEFT` both constrain the min X coordinate), the most stringent
    *   constraint always wins. Constraints on the midpoints are more subtle however,
    *   as they will always be discarded when both the min and max are constrained.
    *   For example, consider an object `A` of fixed size 30x30 and some other object
    *   `B` of fixed size 40x40. If we anchor the `RIGHT` of `A` to the `LEFT` of `B`,
    *   `A`'s _vertical_ center will be automatically aligned with `B`'s vertical center.
    *   This is the effect of the midpoint constraint. Now, if we further anchor the
    *   `TOP` of `A` to the `TOP` of `B`, we have more than one anchor constraining
    *   the vertical extents of `A` (see "Sizing" above), therefore `A`'s fixed height
    *   of 30 pixels will be ignored from now on. It will shrink to a height of 20
    *   pixels, i.e., the distance between `B`'s top edge and its vertical center.
    *   Finally, if we further anchor the `BOTTOM` of `A` to the `BOTTOM` of `B`, the
    *   constraint on `A`'s midpoint will be ignored: `A` will be enlarged to a height
    *   of 40 pixels, i.e., the distance between `B`'s top and bottom edges.
    */
    class uiobject : public utils::enable_observer_from_this<uiobject>
    {
    friend manager;
    public :

        /// Contructor.
        explicit uiobject(manager& mManager);

        /// Destructor.
        virtual ~uiobject();

        /// Non-copiable
        uiobject(const uiobject&) = delete;

        /// Non-movable
        uiobject(uiobject&&) = delete;

        /// Non-copiable
        uiobject& operator=(const uiobject&) = delete;

        /// Non-movable
        uiobject& operator=(uiobject&&) = delete;

        /// Renders this widget on the current render target.
        virtual void render() = 0;

        /// Updates this widget's logic.
        /** \param fDelta Time spent since last update
        */
        virtual void update(float fDelta);

        /// Prints all relevant information about this widget in a string.
        /** \param sTab The offset to give to all lines
        *   \return All relevant information about this widget
        */
        virtual std::string serialize(const std::string& sTab) const;

        /// Copies an uiobject's parameters into this uiobject (inheritance).
        /** \param pObj The uiobject to copy
        */
        virtual void copy_from(const uiobject& mObj);

        /// Tells this widget that its borders need updating.
        virtual void notify_borders_need_update() const;

        /// Tells this widget that the global interface scaling factor has changed.
        virtual void notify_scaling_factor_updated();

        /// Returns this widget's name.
        /** \return This widget's name
        */
        const std::string& get_name() const;

        /// Returns this widget's Lua name.
        /** \return This widget's Lua name
        */
        const std::string& get_lua_name() const;

        /// Returns this widget's raw name.
        /** \return This widget's raw name
        *   \note This is the name of the widget before "$parent"
        *         has been replaced by its parent's name.
        */
        const std::string& get_raw_name() const;

        /// Sets this widget's name.
        /** \param sName This widget's name
        *   \note Can only be called once. If you need to set both the name and the parent
        *         at the same time (typically, at creation), use set_name_and_parent().
        */
        void set_name(const std::string& sName);

        /// Changes this widget's parent.
        /** \param pParent The new parent
        *   \note Default is nullptr.
        */
        void set_parent(utils::observer_ptr<frame> pParent);

        /// Returns this widget's parent.
        /** \return This widget's parent
        */
        utils::observer_ptr<const frame> get_parent() const { return pParent_; }

        /// Returns this widget's parent.
        /** \return This widget's parent
        */
        const utils::observer_ptr<frame>& get_parent() { return pParent_; }

        /// Sets this widget's name and parent at once.
        /** \param sName This widget's name
        *   \param pParent The new parent
        *   \note The name can only be set once. If you need to just change the
        *         parent, call set_parent().
        */
        void set_name_and_parent(const std::string& sName, utils::observer_ptr<frame> pParent);

        /// Removes this widget from its parent and return an owning pointer.
        /** \return An owning pointer to this widget
        */
        virtual utils::owner_ptr<uiobject> release_from_parent();

        /// Forcefully removes this widget from the GUI.
        /** \warning After calling this function, any pointer to the object is invalidated!
        *            Only call this function if you need the object to be destroyed early,
        *            before its parent (if any) would itself be destroyed.
        */
        void destroy();

        /// Changes this widget's alpha (opacity).
        /** \param fAlpha The new alpha value
        *   \note Default is 1.0f.
        */
        void set_alpha(float fAlpha);

        /// Returns this widget's alpha (opacity).
        /** \return This widget's alpha (opacity).
        */
        float get_alpha() const;

        /// Returns this widget's effective alpha (opacity).
        /** \return This widget's effective alpha (opacity).
        *   \note This includes the widget's parent alpha.
        */
        float get_effective_alpha() const;

        /// shows this widget.
        /** \note Its parent must be shown for it to appear on
        *         the screen.
        */
        virtual void show();

        /// hides this widget.
        /** \note All its children won't be visible on the screen
        *         anymore, even if they are still marked as shown.
        */
        virtual void hide();

        /// shows/hides this widget.
        /** \param bIsShown 'true' if you want to show this widget
        *   \note See show() and hide() for more infos.
        *   \note Contrary to show() and hide(), this function doesn't
        *         trigger any event ("OnShow" or "OnHide"). It should
        *         only be used to set the initial state of the widget.
        */
        virtual void set_shown(bool bIsShown);

        /// Checks if this widget is shown.
        /** \return 'true' if this widget is shown
        */
        bool is_shown() const;

        /// Checks if this widget can be seen on the screen.
        /** \return 'true' if this widget can be seen on the screen
        */
        virtual bool is_visible() const;

        /// Changes this widget's absolute dimensions (in pixels).
        /** \param mDimensions The new dimensions
        */
        virtual void set_dimensions(const vector2f& mDimensions);

        /// Changes this widget's absolute width (in pixels).
        /** \param fAbsWidth The new width
        */
        virtual void set_width(float fAbsWidth);

        /// Changes this widget's absolute height (in pixels).
        /** \param fAbsHeight The new height
        */
        virtual void set_height(float fAbsHeight);

        /// Changes this widget's dimensions (relative to its parent).
        /** \param mDimensions The new dimensions (relative)
        */
        void set_relative_dimensions(const vector2f& mDimensions);

        /// Changes this widget's width (relative to its parent).
        /** \param fRelWidth The new width
        */
        void set_relative_width(float fRelWidth);

        /// Changes this widget's height (relative to its parent).
        /** \param fRelHeight The new height
        */
        void set_relative_height(float fRelHeight);

        /// Returns this widget's explicitly-defined width and height (in pixels).
        /** \return This widget's explicitly-defined width and height (in pixels)
        *   \note If you need to get the actual size of a widget on the screen,
        *         use get_apparent_dimensions(), as some widgets may not have
        *         their dimensions explicitly defined, and instead get their
        *         extents from anchors. If a dimension is not explicitly defined,
        *         it will be returned as zero.
        */
        const vector2f& get_dimensions() const;

        /// Returns this widget's appearent width and height (in pixels).
        /** \return This widget's appearent width and height (in pixels)
        *   \note If you need to get the actual size of a widget on the screen,
        *         use this function instead of get_dimensions(), as some widgets
        *         may not have their dimensions explicitly defined, and instead
        *         get their extents from anchors.
        */
        vector2f get_apparent_dimensions() const;

        /// Checks if this widget's apparent width is defined.
        /** \return 'true' if defined, 'false' otherwise
        *   \note The apparent width is defined if either the widget's absolute
        *         or relative width is explicitly specified (from set_width(),
        *         set_relative_width(), set_dimensions(), or set_relative_dimensions()),
        *         or if its left and right borders are anchored. A widget with an undefined
        *         apparent width will not be rendered on the screen until its width is defined.
        */
        bool is_apparent_width_defined() const;

        /// Checks if this widget's apparent height is defined.
        /** \return 'true' if defined, 'false' otherwise
        *   \note The apparent height is defined if either the widget's absolute
        *         or relative height is explicitly specified (from set_height(),
        *         set_relative_height(), set_dimensions(), or set_relative_dimensions()),
        *         or if its left and right borders are anchored. A widget with an undefined
        *         apparent height will not be rendered on the screen until its height is defined.
        */
        bool is_apparent_height_defined() const;

        /// Returns the type of this widget.
        /** \return The type of this widget
        */
        const std::string& get_object_type() const;

        /// Checks if this widget is of the provided type.
        /** \param sType The type to test
        *   \return 'true' if this widget is of the provided type
        */
        bool is_object_type(const std::string& sType) const;

        /// Checks if this widget is of the provided type.
        /** \return 'true' if this widget is of the provided type
        */
        template<typename ObjectType>
        bool is_object_type() const
        {
            return is_object_type(ObjectType::CLASS_NAME);
        }

        /// Returns an array containing all the types of this widget.
        /** \return An array containing all the types of this widget
        */
        const std::vector<std::string>& get_object_type_list() const;

        /// Returns the vertical position of this widget's bottom border.
        /** \return The vertical position of this widget's bottom border
        */
        float get_bottom() const;

        /// Returns the position of this widget's center.
        /** \return The position of this widget's center
        */
        vector2f get_center() const;

        /// Returns the horizontal position of this widget's left border.
        /** \return The horizontal position of this widget's left border
        */
        float get_left() const;

        /// Returns the horizontal position of this widget's right border.
        /** \return The horizontal position of this widget's right border
        */
        float get_right() const;

        /// Returns the vertical position of this widget's top border.
        /** \return The vertical position of this widget's top border
        */
        float get_top() const;

        /// Returns this widget's borders.
        /** \return This widget's borders
        */
        const bounds2f& get_borders() const;

        /// Removes all anchors.
        /** \note This widget and its children won't be visible until you
        *         define at least one anchor.
        */
        void clear_all_points();

        /// Adjusts this widgets anchors to fit the provided widget.
        /** \param pObj A pointer to the object you want to wrap
        *   \note Removes all anchors and defines two new ones.
        */
        void set_all_points(const utils::observer_ptr<uiobject>& pObj);

        /// Adjusts this widgets anchors to fit the provided widget.
        /** \param sObjName The name of the object to fit to
        *   \note Removes all anchors and defines two new ones.<br>
        *         This version is to be used by virtual widgets to
        *         preserve the anchor hierarchy.
        */
        void set_all_points(const std::string& sObjName);

        /// Adds/replaces an anchor.
        /** \param mAnchor The anchor to add
        */
        void set_point(const anchor_data& mAnchor);

        /// Checks if this widget depends on another.
        /** \param mObj The widget to test
        *   \note Usefull to detect circular refences.
        */
        bool depends_on(const uiobject& mObj) const;

        /// Returns the number of defined anchors.
        /** \return The number of defined anchors
        */
        uint get_num_point() const;

        /// Returns one of this widget's anchor to modify it.
        /** \param mPoint The anchor point
        *   \return A pointer to the anchor, nullptr if none
        */
        anchor& modify_point(anchor_point mPoint);

        /// Returns one of this widget's anchor.
        /** \param mPoint The anchor point
        *   \return A pointer to the anchor, nullptr if none
        */
        const anchor& get_point(anchor_point mPoint) const;

        /// Returns all of this widgets's anchors.
        /** \return All of this widgets's anchors
        */
        const std::array<std::optional<anchor>,9>& get_point_list() const;

        /// Round an absolute position on screen to the nearest physical pixel.
        /** \param fValue The input absolute position (can be fractional)
        *   \param mMethod   The rounding method
        *   \return The position of the nearest physical pixel
        */
        float round_to_pixel(float fValue,
            utils::rounding_method mMethod = utils::rounding_method::NEAREST) const;

        /// Round an absolute position on screen to the nearest physical pixel.
        /** \param mPosition The input absolute position (can be fractional)
        *   \param mMethod   The rounding method
        *   \return The position of the nearest physical pixel
        */
        vector2f round_to_pixel(const vector2f& mPosition,
            utils::rounding_method mMethod = utils::rounding_method::NEAREST) const;

        /// Notifies this widget that another one is anchored to it.
        /** \param pObj      The anchored widget
        *   \param bAnchored 'true' if it is anchored, 'false' if it's no longer the case
        */
        void notify_anchored_object(utils::observer_ptr<uiobject> pObj, bool bAnchored) const;

        /// Checks if this uiobject is virtual.
        /** \return 'true' if this uiobject is virtual
        *   \note A virtual uiobject will not be displayed on the screen, but can serve as a
        *         template to create new GUI elements (it is then "inherited", although note
        *         that this has no connection to C++ inheritance).
        */
        bool is_virtual() const;

        /// Makes this uiobject virtual.
        /** \note See is_virtual().
        */
        void set_virtual();

        /// Flags this object as "special".
        /** \note Special objects are not automatically copied
        *         in the frame inheritance process. They must be
        *         explicitely copied by the derived class
        *         (example : Button will have to copy its button
        *         textures itself).
        */
        void set_special();

        /// Checks if this object is special.
        /** \return 'true' if this objet is special
        *   \note For more informations, see set_special().
        */
        bool is_special() const;

        /// Flags this object as newly created.
        /** \note Newly created objects aren't rendered.
        *         They unflag themselves after the first update() call.
        *   \note This function is only called on objects created in Lua.
        */
        void set_newly_created();

        /// Checks if this object has been newly created.
        /** \return 'true' if this object has been newly created
        *   \note For more informations, see set_newly_created().
        */
        bool is_newly_created() const;

        /// Returns the renderer of this object or its parents.
        /** \return The renderer of this object or its parents
        *   \note For more informations, see frame::set_renderer().
        */
        virtual utils::observer_ptr<const frame_renderer> get_top_level_renderer() const;

        /// Returns the renderer of this object or its parents, nullptr if none.
        /** \return The renderer of this object or its parents, nullptr if none
        *   \note For more informations, see set_renderer().
        */
        utils::observer_ptr<frame_renderer> get_top_level_renderer()
        {
            return utils::const_pointer_cast<frame_renderer>(
                const_cast<const uiobject*>(this)->get_top_level_renderer());
        }

        /// Notifies the renderer of this widget that it needs to be redrawn.
        /** \note Automatically called by any shape-changing function.
        */
        virtual void notify_renderer_need_redraw() const;

        /// Returns the list of all objects that are anchored to this one.
        /** \return The list of all objects that are anchored to this one
        */
        const std::vector<utils::observer_ptr<uiobject>>& get_anchored_objects() const;

        /// Notifies this widget that it has been fully loaded.
        virtual void notify_loaded();

        /// Notifies this widget that it is now visible on screen.
        /** \param bTriggerEvents Set to false to disable OnShow/OnHide events
        *   \note Automatically called by show()/hide().
        */
        virtual void notify_visible(bool bTriggerEvents = true);

        /// Notifies this widget that it is no longer visible on screen.
        /** \param bTriggerEvents Set to false to disable OnShow/OnHide events
        *   \note Automatically called by show()/hide().
        */
        virtual void notify_invisible(bool bTriggerEvents = true);

        /// Returns this widget's manager.
        /** \return This widget's manager
        */
        manager& get_manager() { return mManager_; }

        /// Returns this widget's manager.
        /** \return This widget's manager
        */
        const manager& get_manager() const { return mManager_; }

        /// Creates the associated Lua glue.
        /** \note This method is pure virtual : it must be overriden.
        */
        virtual void create_glue() = 0;

        /// Removes the Lua glue.
        void remove_glue();

        /// Parses data from an xml::block.
        /** \param pBlock The uiobject's xml::block
        */
        virtual void parse_block(xml::block* pBlock) = 0;

        /// Registers this widget class to the provided Lua state
        static void register_on_lua(sol::state& mLua);

        template<typename ObjectType>
        friend const ObjectType* down_cast(const uiobject* pSelf);

        template<typename ObjectType>
        friend ObjectType* down_cast(uiobject* pSelf);

        static constexpr const char* CLASS_NAME = "UIObject";

    protected :

        // XML parsing
        virtual void parse_size_block_(xml::block* pBlock);
        virtual void parse_anchor_block_(xml::block* pBlock);
        color        parse_color_block_(xml::block* pBlock);

        void read_anchors_(float& fLeft, float& fRight, float& fTop,
                           float& fBottom, float& fXCenter, float& fYCenter) const;

        void make_borders_(float& fMin, float& fMax, float fCenter, float fSize) const;

        virtual void update_borders_() const;
        virtual void update_anchors_();

        sol::state&  get_lua_();

        template<typename T>
        void create_glue_(T* pSelf);

        void set_lua_member_(std::string sKey, sol::stack_object mValue);
        sol::object get_lua_member_(const std::string& sKey) const;

        manager& mManager_;

        std::string sName_;
        std::string sRawName_;
        std::string sLuaName_;
        uint        uiID_ = uint(-1);

        utils::observer_ptr<frame> pParent_ = nullptr;

        bool         bSpecial_ = false;
        bool         bNewlyCreated_ = false;
        bool         bInherits_ = false;
        bool         bVirtual_ = false;
        bool         bLoaded_ = false;
        mutable bool bReady_ = true;

        std::vector<std::string> lType_;

        std::array<std::optional<anchor>,9> lAnchorList_;
        std::vector<utils::observer_ptr<const uiobject>> lPreviousAnchorParentList_;
        bounds2<bool>                       lDefinedBorderList_;
        mutable bounds2f                    lBorderList_;

        float fAlpha_ = 1.0f;
        bool  bIsShown_ = true;
        bool  bIsVisible_ = true;

        mutable vector2f mDimensions_;

        mutable bool bUpdateBorders_ = true;

        mutable std::vector<utils::observer_ptr<uiobject>> lAnchoredObjectList_;

        std::unordered_map<std::string, sol::object> lLuaMembers_;
    };

    /// Obtain a pointer to a derived class.
    /** \param pSelf The pointer to down cast
    *   \return A pointer to a derived class
    *   \note Like dynamic_cast(), this will return nullptr if this widget
    *         is not of the requested type. However, it will throw if the cast
    *         failed because the derived class destructor has already been
    *         called. This indicates a programming error.
    */
    template<typename ObjectType>
    const ObjectType* down_cast(const uiobject* pSelf)
    {
        const ObjectType* pObject = dynamic_cast<const ObjectType*>(pSelf);
        if (pSelf && !pObject && pSelf->is_object_type(ObjectType::CLASS_NAME))
        {
            throw gui::exception(pSelf->lType_.back(),
                "cannot use down_cast() to "+std::string(ObjectType::CLASS_NAME)+
                " as object is being destroyed");
        }
        return pObject;
    }

    /// Obtain a pointer to a derived class.
    /** \param pSelf The pointer to down cast
    *   \return A pointer to a derived class
    *   \note Like dynamic_cast(), this will return nullptr if this widget
    *         is not of the requested type. However, it will throw if the cast
    *         failed because the derived class destructor has already been
    *         called. This indicates a programming error.
    */
    template<typename ObjectType>
    ObjectType* down_cast(uiobject* pSelf)
    {
        return const_cast<ObjectType*>(
            down_cast<ObjectType>(const_cast<const uiobject*>(pSelf)));
    }

    /// Perform a down cast on an owning pointer.
    /** \param pObject The owning pointer to down cast
    *   \return The down casted pointer.
    *   \note See down_cast(const uiobject*) for more information.
    */
    template<typename ObjectType>
    utils::owner_ptr<ObjectType> down_cast(utils::owner_ptr<uiobject>&& pObject)
    {
        return utils::owner_ptr<ObjectType>(std::move(pObject),
            down_cast<ObjectType>(pObject.get()));
    }

    /// Perform a down cast on an observer pointer.
    /** \param pObject The observer pointer to down cast
    *   \return The down casted pointer.
    *   \note See down_cast(const uiobject*) for more information.
    */
    template<typename ObjectType>
    utils::observer_ptr<ObjectType> down_cast(const utils::observer_ptr<uiobject>& pObject)
    {
        return utils::observer_ptr<ObjectType>(pObject, down_cast<ObjectType>(pObject.get()));
    }

    /// Perform a down cast on an observer pointer.
    /** \param pObject The observer pointer to down cast
    *   \return The down casted pointer.
    *   \note See down_cast(const uiobject*) for more information.
    */
    template<typename ObjectType>
    utils::observer_ptr<ObjectType> down_cast(utils::observer_ptr<uiobject>&& pObject)
    {
        return utils::observer_ptr<ObjectType>(std::move(pObject),
            down_cast<ObjectType>(pObject.get()));
    }

    /// Obtain an observer pointer from a raw pointer (typically 'this')
    /** \param pSelf The raw pointer to get an observer from
    *   \return The observer pointer.
    *   \note This returns the same things as pSelf->observer_from_this(),
    *         but returning a pointer to the most-derived type known form the
    *         input pointer.
    */
    template<typename ObjectType>
    utils::observer_ptr<ObjectType> observer_from(ObjectType* pSelf)
    {
        if (pSelf)
            return utils::static_pointer_cast<ObjectType>(pSelf->uiobject::observer_from_this());
        else
            return nullptr;
    }
}
}

#endif
