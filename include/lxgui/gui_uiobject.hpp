#ifndef LXGUI_GUI_UIOBJECT_HPP
#define LXGUI_GUI_UIOBJECT_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils.hpp>
#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_vector2.hpp"
#include "lxgui/gui_quad2.hpp"
#include "lxgui/gui_color.hpp"
#include "lxgui/gui_strata.hpp"
#include "lxgui/gui_exception.hpp"

#include <lxgui/xml.hpp>
#include <lxgui/luapp_lua_fwd.hpp>

#include <array>
#include <optional>

namespace sol
{
    class state;
}

namespace lxgui {
namespace lua
{
    class state;
}

namespace gui
{
    struct addon;
    class manager;

    /** \cond NOT_REMOVE_FROM_DOC
    */

    // Generic Lua glue
    class lua_glue
    {
    public :

        explicit lua_glue(lua_State* luaVM);
        virtual ~lua_glue();

        virtual void notify_deleted() = 0;

        int get_data_table(lua_State *L);

    protected :

        lua_State* pLua_;
        int        iRef_;
    };

    // Virtual widget Lua glue
    class lua_virtual_glue : public lua_glue
    {
    public :

        explicit lua_virtual_glue(lua_State* luaVM);
        virtual ~lua_virtual_glue();

        virtual void notify_deleted();

        int _mark_for_copy(lua_State*);
        int _get_base(lua_State*);
        int _get_name(lua_State*);

        static const char className[];
        static const char* classList[];
        static lua::lunar_binding<lua_virtual_glue> methods[];

    protected :

        uint      uiID_ = 0u;
        uiobject* pObject_ = nullptr;
    };

    /** \endcond
    */

    enum class layer_type
    {
        BACKGROUND,
        BORDER,
        ARTWORK,
        OVERLAY,
        HIGHLIGHT,
        SPECIALHIGH
    };

    class frame;
    class renderer;

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
    *   Deleting an object from C++ is done using uiobject::release_from_parent.
    *   This will automatically delete all references to this object in Lua as well.
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
    class uiobject
    {
    friend manager;
    public :

        /// Comparator for sorting objects by ID
        template<typename ObjectType>
        struct id_comparator
        {
            bool operator() (const std::unique_ptr<ObjectType>& pObject1, const std::unique_ptr<ObjectType>& pObject2) const
            {
                return pObject1->get_id() < pObject2->get_id();
            }
            bool operator() (const std::unique_ptr<ObjectType>& pObject1, uint uiID) const
            {
                return pObject1->get_id() < uiID;
            }
            bool operator() (uint uiID, const std::unique_ptr<ObjectType>& pObject2) const
            {
                return uiID < pObject2->get_id();
            }
        };

        /// Contructor.
        explicit uiobject(manager* pManager);

        /// Destructor.
        virtual ~uiobject();

        /// Renders this widget on the current render target.
        virtual void render() = 0;

        /// updates this widget's anchors.
        /** \note Must be called on *all* widgets before update().
        */
        virtual void update_anchors();

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
        virtual void copy_from(uiobject* pObj);

        /// Tells this widget to update its borders.
        virtual void fire_update_borders() const;

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
        *   \note Can only be called once.
        */
        void set_name(const std::string& sName);

        /// Changes this widget's parent.
        /** \param pParent The new parent
        *   \note Default is nullptr.
        */
        virtual void set_parent(frame* pParent);

        /// Returns this widget's parent.
        /** \return This widget's parent
        */
        const frame* get_parent() const;

        /// Returns this widget's parent.
        /** \return This widget's parent
        */
        frame* get_parent();

        /// Removes this widget from its parent and return an owning pointer.
        /** \return An owning pointer to this widget
        */
        virtual std::unique_ptr<uiobject> release_from_parent();

        /// Returns the widget this one inherits from.
        /** \return The widget this one inherits from
        */
        uiobject* get_base();

        /// Changes this widget's alpha (opacity).
        /** \param fAlpha The new alpha value
        *   \note Default is 1.0f.
        */
        void set_alpha(float fAlpha);

        /// Returns this widget's alpha (opacity).
        /** \return This widget's alpha (opacity).
        */
        float get_alpha() const;

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
        /** \param uiAbsWidth  The new width
        *   \param uiAbsHeight The new height
        */
        virtual void set_abs_dimensions(uint uiAbsWidth, uint uiAbsHeight);

        /// Changes this widget's absolute width (in pixels).
        /** \param uiAbsWidth The new width
        */
        virtual void set_abs_width(uint uiAbsWidth);

        /// Changes this widget's absolute height (in pixels).
        /** \param uiAbsHeight The new height
        */
        virtual void set_abs_height(uint uiAbsHeight);

        /// Changes this widget's dimensions (relative to its parent).
        /** \param fRelWidth  The new width
        *   \param fRelHeight The new height
        */
        void set_rel_dimensions(float fRelWidth, float fRelHeight);

        /// Changes this widget's width (relative to its parent).
        /** \param fRelWidth The new width
        */
        void set_rel_width(float fRelWidth);

        /// Changes this widget's height (relative to its parent).
        /** \param fRelHeight The new height
        */
        void set_rel_height(float fRelHeight);

        /// Returns this widget's absolute width (in pixels).
        /** \return This widget's absolute width (in pixels)
        *   \note If you need to get the size of a widget on the screen,
        *         use get_apparent_width() instead, because some
        *         widgets can have an infinite or undefined width.
        */
        uint get_abs_width() const;

        /// Returns this widget's appearend width (in pixels).
        /** \return This widget's appearend width (in pixels)
        *   \note If you need to get the size of a widget on the screen,
        *         use this function instead of get_abs_width(), because
        *         some widgets can have an infinite or undefined width.
        */
        uint get_apparent_width() const;

        /// Returns this widget's absolute height (in pixels).
        /** \return This widget's absolute height (in pixels)
        *   \note If you need to get the size of a widget on the screen,
        *         use get_apparent_height() instead, because some
        *         widgets can have an infinite or undefined height.
        */
        uint get_abs_height() const;

        /// Returns this widget's appearend height (in pixels).
        /** \return This widget's appearend height (in pixels)
        *   \note If you need to get the size of a widget on the screen,
        *         use this function instead of get_abs_height(), because
        *         some widgets can have an infinite or undefined height.
        */
        uint get_apparent_height() const;

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
        int get_bottom() const;

        /// Returns the position of this widget's center.
        /** \return The position of this widget's center
        */
        vector2<int> get_center() const;

        /// Returns the horizontal position of this widget's left border.
        /** \return The horizontal position of this widget's left border
        */
        int get_left() const;

        /// Returns the horizontal position of this widget's right border.
        /** \return The horizontal position of this widget's right border
        */
        int get_right() const;

        /// Returns the vertical position of this widget's top border.
        /** \return The vertical position of this widget's top border
        */
        int get_top() const;

        /// Returns this widget's borders.
        /** \return This widget's borders
        */
        const quad2i& get_borders() const;

        /// Removes all anchors.
        /** \note This widget and its children won't be visible until you
        *         define at least one anchor.
        */
        void clear_all_points();

        /// Adjusts this widgets anchors to fit the provided widget.
        /** \param pObj A pointer to the object you want to wrap
        *   \note Removes all anchors and defines two new ones.
        */
        void set_all_points(uiobject* pObj);

        /// Adjusts this widgets anchors to fit the provided widget.
        /** \param sObjName The name of the object to fit to
        *   \note Removes all anchors and defines two new ones.<br>
        *         This version is to be used by virtual widgets to
        *         preserve the anchor hierarchy.
        */
        void set_all_points(const std::string& sObjName);

        /// create_s/modifies an anchor.
        /** \param mPoint         The anchor point for this object
        *   \param sParentName    The anchor's parent
        *   \param mRelativePoint The anchor point for the parent
        *   \param iX             The horizontal offset
        *   \param iY             The vertical offset
        */
        void set_abs_point(anchor_point mPoint, const std::string& sParentName,
            anchor_point mRelativePoint, int iX = 0, int iY = 0);

        /// create_s/modifies an anchor.
        /** \param mPoint         The anchor point for this object
        *   \param sParentName    The anchor's parent
        *   \param mRelativePoint The anchor point for the parent
        *   \param mOffset        The offset
        */
        void set_abs_point(anchor_point mPoint, const std::string& sParentName,
            anchor_point mRelativePoint, const vector2i& mOffset);

        /// create_s/modifies an anchor.
        /** \param mPoint         The anchor point for this object
        *   \param sParentName    The anchor's parent
        *   \param mRelativePoint The anchor point for the parent
        *   \param fX             The horizontal offset
        *   \param fY             The vertical offset
        */
        void set_rel_point(anchor_point mPoint, const std::string& sParentName,
            anchor_point mRelativePoint, float fX = 0, float fY = 0);

        /// create_s/modifies an anchor.
        /** \param mPoint         The anchor point for this object
        *   \param sParentName    The anchor's parent
        *   \param mRelativePoint The anchor point for the parent
        *   \param mOffset        The offset
        */
        void set_rel_point(anchor_point mPoint, const std::string& sParentName,
            anchor_point mRelativePoint, const vector2f& mOffset);

        /// Adds/replaces an anchor.
        /** \param mAnchor The anchor to add
        */
        void set_point(const anchor& mAnchor);

        /// Checks if this widget depends on another.
        /** \param pObj The widget to test
        *   \note Usefull to detect circular refences.
        */
        bool depends_on(uiobject* pObj) const;

        /// Returns the number of defined anchors.
        /** \return The number of defined anchors
        */
        uint get_num_point() const;

        /// Returns one of this widget's anchor to modify it.
        /** \param mPoint The anchor point
        *   \return A pointer to the anchor, nullptr if none
        */
        anchor* modify_point(anchor_point mPoint);

        /// Returns one of this widget's anchor.
        /** \param mPoint The anchor point
        *   \return A pointer to the anchor, nullptr if none
        */
        const anchor* get_point(anchor_point mPoint) const;

        /// Returns all of this widgets's anchors.
        /** \return All of this widgets's anchors
        */
        const std::array<std::optional<anchor>,9>& get_point_list() const;

        /// Notifies this widget that another one is anchored to it.
        /** \param pObj      The anchored widget
        *   \param bAnchored 'true' if it is anchored, 'false' if it's no longer the case
        */
        void notify_anchored_object(uiobject* pObj, bool bAnchored) const;

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

        /// Returns this widget's unique ID.
        /** \return This widget's unique ID
        */
        uint get_id() const;

        /// Sets this widget's unique ID.
        /** \param uiID The ID
        *   \note Can only be called once.
        */
        void set_id(uint uiID);

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
        virtual renderer* get_top_level_renderer();

        /// Returns the renderer of this object or its parents.
        /** \return The renderer of this object or its parents
        *   \note For more informations, see frame::set_renderer().
        */
        virtual const renderer* get_top_level_renderer() const;

        /// Notifies the renderer of this widget that it needs to be redrawn.
        /** \note Automatically called by any shape-changing function.
        */
        virtual void notify_renderer_need_redraw() const;

        /// Adds a Lua variable to copy when inheriting.
        /** \param sVariable The name of the variable
        *   \note The variable must be an element of the widget's Lua glue.
        *         If you have a virtual widget called "Test", and in some lua code you do :<br>
        *         Test.someVariable = 2;<br>
        *         ... then you can call this function with "someVariable".
        */
        void mark_for_copy(const std::string& sVariable);

        /// Returns the list of all objects that are anchored to this one.
        /** \return The list of all objects that are anchored to this one
        */
        const std::vector<uiobject*>& get_anchored_objects() const;

        /// Notifies this widget that it has been fully loaded.
        virtual void notify_loaded();

        /// Returns this widget's manager.
        /** \return This widget's manager
        */
        manager* get_manager();

        /// Returns this widget's manager.
        /** \return This widget's manager
        */
        const manager* get_manager() const;

        /// Creates the associated Lua glue.
        /** \note This method is pure virtual : it must be overriden.
        */
        virtual void create_glue() = 0;

        /// Removes the Lua glue.
        void remove_glue();

        /// Pushes this uiobject on the provided lua state.
        /** \param mLua The lua state on which to push the glue
        */
        void push_on_lua(sol::state& mLua) const;

        /// Pushes this uiobject on the provided lua State.
        /** \param mLua The lua state on which to push the glue
        */
        void push_on_lua(lua::state& mLua) const;

        /// Parses data from an xml::block.
        /** \param pBlock The uiobject's xml::block
        */
        virtual void parse_block(xml::block* pBlock) = 0;

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

        void         read_anchors_(float& iLeft, float& iRight, float& iTop, float& iBottom, float& iXCenter, float& iYCenter) const;
        void         make_borders_(float& iMin, float& iMax, float iCenter, float iSize) const;
        virtual void update_borders_() const;

        sol::state&  get_lua_();
        lua::state&  get_luapp_();

        template<typename T>
        void create_glue_();

        manager* pManager_ = nullptr;

        std::string sName_;
        std::string sRawName_;
        std::string sLuaName_;
        uint        uiID_ = uint(-1);
        frame*      pParent_ = nullptr;
        uiobject*   pInheritance_ = nullptr;
        bool        bSpecial_ = false;
        bool        bNewlyCreated_ = false;
        bool        bInherits_ = false;

        bool         bVirtual_ = false;
        bool         bLoaded_ = false;
        mutable bool bReady_ = true;

        lua_glue*                lGlue_ = nullptr;
        std::vector<std::string> lCopyList_;

        std::vector<std::string> lType_;

        std::array<std::optional<anchor>,9> lAnchorList_;
        std::vector<const uiobject*>        lPreviousAnchorParentList_;
        quad2<bool>                         lDefinedBorderList_;
        mutable quad2i                      lBorderList_;

        float fAlpha_ = 1.0f;
        bool  bIsShown_ = true;
        bool  bIsVisible_ = true;

        mutable uint uiAbsWidth_ = 0u;
        mutable uint uiAbsHeight_ = 0u;

        mutable bool bUpdateAnchors_ = false;
        mutable bool bUpdateBorders_ = true;

        mutable std::vector<uiobject*> lAnchoredObjectList_;
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
    std::unique_ptr<ObjectType> down_cast(std::unique_ptr<uiobject> pObject)
    {
        ObjectType* pCasted = down_cast<ObjectType>(pObject.get());
        pObject.release();
        return std::unique_ptr<ObjectType>(pCasted);
    }

    /** \cond NOT_REMOVE_FROM_DOC
    */

    // uiobject Lua glue
    class lua_uiobject : public lua_glue
    {
    public :

        explicit lua_uiobject(lua_State* luaVM);
        virtual ~lua_uiobject();

        virtual void notify_deleted();

        uiobject* get_object();
        const std::string& get_name() const;
        void clear_object();

        // uiobject
        int _get_alpha(lua_State*);
        int _get_name(lua_State*);
        int _get_object_type(lua_State*);
        int _is_object_type(lua_State*);
        int _set_alpha(lua_State*);
        // region
        int _clear_all_points(lua_State*);
        int _get_base(lua_State*);
        int _get_bottom(lua_State*);
        int _get_center(lua_State*);
        int _get_height(lua_State*);
        int _get_left(lua_State*);
        int _get_num_point(lua_State*);
        int _get_parent(lua_State*);
        int _get_point(lua_State*);
        int _get_right(lua_State*);
        int _get_top(lua_State*);
        int _get_width(lua_State*);
        int _hide(lua_State*);
        int _is_shown(lua_State*);
        int _is_visible(lua_State*);
        int _set_all_points(lua_State*);
        int _set_height(lua_State*);
        int _set_parent(lua_State*);
        int _set_point(lua_State*);
        int _set_rel_point(lua_State*);
        int _set_width(lua_State*);
        int _show(lua_State*);

        static const char className[];
        static const char* classList[];
        static lua::lunar_binding<lua_uiobject> methods[];

    protected :

        bool check_object_();

        std::string sName_;
        uiobject*   pObject_;
    };

    /** \endcond
    */
}
}

#endif
