#ifndef GUI_UIOBJECT_HPP
#define GUI_UIOBJECT_HPP

#include <lxgui/utils.hpp>
#include <lxgui/utils_optional.hpp>
#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_manager.hpp"
#include <lxgui/luapp_state.hpp>
#include "lxgui/gui_vector2.hpp"
#include "lxgui/gui_quad2.hpp"
#include <lxgui/xml.hpp>

#include <array>

namespace lxgui {
namespace gui
{
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
        static lua::Lunar<lua_virtual_glue>::RegType methods[];

    protected :

        uint      uiID_ = 0u;
        uiobject* pParent_ = nullptr;
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

    /// The base of the GUI
    /** This widget (GUI element) is a virtual base.
    *   It doesn't display anything on its own and must
    *   be inherited to allow new features. It provides
    *   several virtual functions : update, render...
    *   They should be overriden and contain all the
    *   required logic for it to work.<br>
    *   Look at the provided widgets (Frame, Slider, ...)
    *   to see how it's done.
    */
    class uiobject
    {
    friend manager;
    public :

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

        /// updates this widget's logic.
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

        /// Tells this widget to update its dimensions.
        void fire_update_dimensions() const;

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
        virtual void set_parent(uiobject* pParent);

        /// Returns this widget's parent.
        /** \return This widget's parent
        */
        const uiobject* get_parent() const;

        /// Returns this widget's parent.
        /** \return This widget's parent
        */
        uiobject* get_parent();

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

        /// Checks if this widget's width has been defined as absolute.
        /** \return 'true' if this widget's width has been defined as absolute
        */
        bool is_width_absolute() const;

        /// Checks if this widget's height has been defined as absolute.
        /** \return 'true' if this widget's height has been defined as absolute
        */
        bool is_height_absolute() const;

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

        /// Returns this widget's width (relative to its parent).
        /** \return This widget's width (relative to its parent)
        */
        float get_rel_width() const;

        /// Returns this widget's height (relative to its parent).
        /** \return This widget's height (relative to its parent)
        */
        float get_rel_height() const;

        /// Returns the type of this widget.
        /** \return The type of this widget
        */
        const std::string& get_object_type() const;

        /// Checks if this widget is of the provided type.
        /** \param sType The type to test
        *   \return 'true' if this widget is of the provided type
        */
        bool is_object_type(const std::string& sType) const;

        /// Returns an array containing all the types of this widget.
        /** \return  An array containing all the types of this widget
        */
        const std::vector<std::string>& get_object_typeList() const;

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
        const std::array<utils::optional<anchor>,9>& get_point_list() const;

        /// Notifies this widget that another one is anchored to it.
        /** \param pObj      The anchored widget
        *   \param bAnchored 'true' if it is anchored, 'false' if it's no longer the case
        */
        void notify_anchored_object(uiobject* pObj, bool bAnchored) const;

        /// Checks if this uiobject is virtual.
        /** \return 'true' if this uiobject is virtual
        *   \note A virtual uiobject can be inherited.
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

        /// Flags this object as "manually rendered".
        /** \param bManuallyRendered 'true' to flag it as manually rendered
        *   \param pRenderer         The uiobject that will take care of
        *                            rendering this widget
        *   \note Manually rendered objects are not automatically rendered
        *         by their parent (for layered_regions) or the manager
        *         (for frames). They also don't receive input automatically.
        */
        virtual void set_manually_rendered(bool bManuallyRendered, uiobject* pRenderer = nullptr);

        /// Checks if this object is manually rendered.
        /** \return 'true' if this object is manually rendered
        *   \note For more informations, see set_manually_rendered().
        */
        bool is_manually_rendered() const;

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

        /// Notifies the renderer of this widget that it needs to be redrawn.
        /** \note Automatically called by any shape changing function.
        */
        virtual void notify_renderer_need_redraw() const;

        /// Tells this widget that a manually rendered widget requires redraw.
        /** \note This function does nothing by default.
        */
        virtual void fire_redraw() const;

        /// Adds a Lua variable to copy when derivating.
        /** \param sVariable The name of the variable
        *   \note The variable must be an element of the widget's Lua glue.<br>
        *         If you have a virtual widget called "Test", and in some lua code you do :<br>
        *         Test.someVariable = 2;<br>
        *         ... then you can call this function with "someVariable".
        */
        void mark_for_copy(const std::string& sVariable);


        /// Removes all anchors that point to this widget and all other kind of links.
        /** \note Will be called by the destructor.
        */
        virtual void clear_links();

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

        /// Pushes this uiobject on the provided lua::State.
        /** \param pLua The lua::State on which to push the glue
        */
        virtual void push_on_lua(lua::state* pLua) const;

        /// Parses data from an xml::block.
        /** \param pBlock The uiobject's xml::block
        */
        virtual void parse_block(xml::block* pBlock) = 0;

        static constexpr const char* CLASS_NAME = "UIObject";

    protected :

        // XML parsing
        virtual void parse_size_block_(xml::block* pBlock);
        virtual void parse_anchor_block_(xml::block* pBlock);
        color        parse_color_block_(xml::block* pBlock);

        void         read_anchors_(float& iLeft, float& iRight, float& iTop, float& iBottom, float& iXCenter, float& iYCenter) const;
        void         make_borders_(float& iMin, float& iMax, float iCenter, float iSize) const;
        virtual void update_borders_() const;
        virtual void update_dimensions_() const;

        virtual void notify_manually_rendered_object_(uiobject* pObject, bool bManuallyRendered);

        template<typename T>
        void create_glue_()
        {
            if (lGlue_) return;

            lua::state* pLua = pManager_->get_lua();

            if (bVirtual_)
            {
                pLua->push_number(uiID_);
                lGlue_ = pLua->push_new<lua_virtual_glue>();
            }
            else
            {
                pLua->push_string(sLuaName_);
                lGlue_ = pLua->push_new<T>();
            }

            pLua->set_global(sLuaName_);
            pLua->pop();
        }

        manager* pManager_ = nullptr;

        std::string sName_;
        std::string sRawName_;
        std::string sLuaName_;
        uint        uiID_ = uint(-1);
        uiobject*   pParent_ = nullptr;
        uiobject*   pInheritance_ = nullptr;
        bool        bSpecial_ = false;
        bool        bManuallyRendered_ = false;
        bool        bNewlyCreated_ = false;
        uiobject*   pRenderer_ = nullptr;
        bool        bInherits_ = false;

        bool         bVirtual_ = false;
        bool         bLoaded_ = false;
        mutable bool bReady_ = true;

        lua_glue*                lGlue_ = nullptr;
        std::vector<std::string> lCopyList_;

        std::vector<std::string> lType_;

        std::array<utils::optional<anchor>,9> lAnchorList_;
        std::vector<const uiobject*>          lPreviousAnchorParentList_;
        quad2<bool>                           lDefinedBorderList_;
        mutable quad2i                        lBorderList_;

        float fAlpha_ = 1.0f;
        bool  bIsShown_ = true;
        bool  bIsVisible_ = true;

        mutable bool  bIsWidthAbs_ = true;
        mutable bool  bIsHeightAbs_ = true;
        mutable uint  uiAbsWidth_ = 0u;
        mutable uint  uiAbsHeight_ = 0u;
        mutable float fRelWidth_ = 0.0f;
        mutable float fRelHeight_ = 0.0f;

        mutable bool bUpdateAnchors_ = false;
        mutable bool bUpdateBorders_ = true;
        mutable bool bUpdateDimensions_ = false;

        mutable std::vector<uiobject*> lAnchoredObjectList_;
    };

    /** \cond NOT_REMOVE_FROM_DOC
    */

    // uiobject Lua glue
    class lua_uiobject : public lua_glue
    {
    public :

        explicit lua_uiobject(lua_State* luaVM);
        virtual ~lua_uiobject();

        virtual void notify_deleted();

        uiobject* get_parent();

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
        static lua::Lunar<lua_uiobject>::RegType methods[];

    protected :

        bool check_parent_();

        std::string sName_;
        uiobject*   pParent_;
    };

    /** \endcond
    */
}
}

#endif
