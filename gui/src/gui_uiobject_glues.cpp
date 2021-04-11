#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/luapp_state.hpp>
#include <lxgui/utils_string.hpp>
#include <lxgui/luapp_function.hpp>

/** The base class of all elements in the GUI.
*   Objects of this class offers core functionalities needed by every element
*   of the interface. They have a name, and a corresponding variable created
*   in Lua to access them. They can have a parent @{Frame}. They can be placed
*   on the screen at an absolute position, or relative to other @{UIObject}s.
*   They can be shown or hidden.
*
*   Apart form this, a @{UIObject} does not contain anything, nor can it display
*   anything on the screen. Any functionality beyond the list above is implemented
*   in specialized subclasses (see the full list below).
*
*   __Interaction between C++, Lua, and XML.__ When a @{UIObject} is created,
*   it must be given a name, for example `"PlayerHealthBar"`. For as long as the
*   object lives, this name will be used to refer to it. In particular, as soon
*   as the object is created, regardless of whether this was done in C++, XML, or
*   Lua, a new variable will be created in the Lua state with the exact same name,
*   `PlayerHealthBar`. This variable is a reference to the @{UIObject}, and can
*   be used to interact with it dynamically. Because of this, each object must have
*   a unique name, otherwise it could not be accessible from Lua.
*
*   Note: Although you can destroy this Lua variable by setting it to nil, this is
*   not recommended: the object will _not_ be destroyed (nor garbage-collected)
*   because it still exists in the C++ memory space. The only way to truly destroy
*   an object is to call @{Manager.delete_frame} (for @{Frame}s only). Destroying and
*   creating objects has a cost however. If the object is likely to reappear later
*   with the same content, simply hide it and show it again later on. If the
*   content may change, you can also recycle the object, i.e., keep it alive and
*   simply change its content when it later reappears.
*
*   __Parent-child relationship.__ Parents of @{UIObject}s are @{Frame}s. See
*   the @{Frame} class documentation for more information. One important aspect
*   of the parent-child relationship is related to the object name. If a
*   @{UIObject} has a parent, it can be given a name starting with `"$parent"`.
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
*   __Positioning.__ @{UIObject}s have a position on the screen, but this is
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
*   __Sizing.__ There are two ways to specify the size of a @{UIObject}. The
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
*
*   Inherits all methods from: none.
*
*   Child classes: @{Frame}, @{LayeredRegion}, @{FontString}, @{Texture},
*   @{Button}, @{CheckButton}, @{FocusFrame}, @{EditBox}, @{ScrollFrame},
*   @{Slider}, @{StatusBar}.
*   @classmod UIObject
*/

namespace lxgui {
namespace gui
{
lua_glue::lua_glue(lua_State* pLua)
{
    lua_newtable(pLua);
    iRef_ = luaL_ref(pLua, LUA_REGISTRYINDEX);
    pLua_ = pLua;
}

lua_glue::~lua_glue()
{
    luaL_unref(pLua_, LUA_REGISTRYINDEX, iRef_);
}

lua_virtual_glue::lua_virtual_glue(lua_State* pLua) : lua_glue(pLua)
{
    uiID_ = lua_tonumber(pLua, -1);
    lua::state mState(pLua);
    manager* pGUIMgr = manager::get_manager(mState);
    pObject_ = pGUIMgr->get_uiobject(uiID_);

    if (!pObject_)
        throw exception("lua_virtual_glue", "Glue missing its parent (\""+utils::to_string(uiID_)+"\") !");
}

lua_virtual_glue::~lua_virtual_glue()
{
}

void lua_virtual_glue::notify_deleted()
{
    pObject_ = nullptr;
}

int lua_virtual_glue::_mark_for_copy(lua_State* pLua)
{
    lua::function mFunc("VirtualGlue:mark_for_copy", pLua, 1);
    mFunc.add(0, "variable", lua::type::STRING);
    if (mFunc.check())
        pObject_->mark_for_copy(mFunc.get(0)->get_string());

    return mFunc.on_return();
}

int lua_virtual_glue::_get_name(lua_State* pLua)
{
    lua::function mFunc("VirtualGlue:get_name", pLua, 1);

    mFunc.push(pObject_->get_lua_name());

    return mFunc.on_return();
}

int lua_virtual_glue::_get_base(lua_State* pLua)
{
    lua::function mFunc("VirtualGlue:get_base", pLua, 1);

    if (pObject_->get_base())
    {
        pObject_->get_base()->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

lua_uiobject::lua_uiobject(lua_State* pLua) : lua_glue(pLua)
{
    sName_ = lua_tostring(pLua, -1);
    lua::state mState(pLua);
    manager* pGUIMgr = manager::get_manager(mState);
    pObject_ = pGUIMgr->get_uiobject_by_name(sName_);

    if (!pObject_)
        throw exception("lua_uiobject", "Glue missing its object (\""+sName_+"\") !");
}

lua_uiobject::~lua_uiobject()
{
}

void lua_uiobject::notify_deleted()
{
    pObject_ = nullptr;
}

uiobject* lua_uiobject::get_object()
{
    return pObject_;
}

const std::string& lua_uiobject::get_name() const
{
    return sName_;
}

void lua_uiobject::clear_object()
{
    pObject_ = nullptr;
}

bool lua_uiobject::check_object_()
{
    if (!pObject_)
    {
        gui::out << gui::warning << " : " << sName_ << " : This widget has been deleted and can no longer be used." << std::endl;
        return false;
    }

    return true;
}

int lua_glue::get_data_table(lua_State * pLua)
{
    lua_rawgeti(pLua, LUA_REGISTRYINDEX, iRef_);
    return 1;
}

/** @function get_alpha
*/
int lua_uiobject::_get_alpha(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:get_alpha", pLua, 1);

    mFunc.push(pObject_->get_alpha());

    return mFunc.on_return();
}

/** @function get_name
*/
int lua_uiobject::_get_name(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:get_name", pLua, 1);

    mFunc.push(pObject_->get_name());

    return mFunc.on_return();
}

/** @function get_object_type
*/
int lua_uiobject::_get_object_type(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:get_object_type", pLua, 1);

    mFunc.push(pObject_->get_object_type());

    return mFunc.on_return();
}

/** @function is_object_type
*/
int lua_uiobject::_is_object_type(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:is_object_type", pLua, 1);
    mFunc.add(0, "object type", lua::type::STRING);
    if (mFunc.check())
    {
        mFunc.push(pObject_->is_object_type(mFunc.get(0)->get_string()));
    }

    return mFunc.on_return();
}

/** @function set_alpha
*/
int lua_uiobject::_set_alpha(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:set_alpha", pLua);
    mFunc.add(0, "alpha", lua::type::NUMBER);
    if (mFunc.check())
    {
        pObject_->set_alpha(mFunc.get(0)->get_number());
    }

    return mFunc.on_return();
}

/** @function clear_all_points
*/
int lua_uiobject::_clear_all_points(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:clear_all_points", pLua);

    pObject_->clear_all_points();

    return mFunc.on_return();
}

/** @function get_bottom
*/
int lua_uiobject::_get_bottom(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:get_bottom", pLua, 1);

    mFunc.push(pObject_->get_bottom());

    return mFunc.on_return();
}

/** @function get_center
*/
int lua_uiobject::_get_center(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:get_center", pLua, 2);

    vector2f mP = pObject_->get_center();
    mFunc.push(mP.x);
    mFunc.push(mP.y);

    return mFunc.on_return();
}

/** @function get_height
*/
int lua_uiobject::_get_height(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:get_height", pLua, 1);

    mFunc.push(pObject_->get_apparent_height());

    return mFunc.on_return();
}

/** @function get_left
*/
int lua_uiobject::_get_left(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:get_left", pLua, 1);

    mFunc.push(pObject_->get_left());

    return mFunc.on_return();
}

/** @function get_num_point
*/
int lua_uiobject::_get_num_point(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:get_num_point", pLua, 1);

    mFunc.push(pObject_->get_num_point());

    return mFunc.on_return();
}

/** @function get_parent
*/
int lua_uiobject::_get_parent(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:get_parent", pLua, 1);

    if (pObject_->get_parent())
    {
        pObject_->get_parent()->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

/** @function get_base
*/
int lua_uiobject::_get_base(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:get_base", pLua, 1);

    if (pObject_->get_base())
    {
        pObject_->get_base()->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

/** @function get_point
*/
int lua_uiobject::_get_point(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:get_point", pLua, 5);
    mFunc.add(0, "point ID", lua::type::NUMBER, true);
    if (mFunc.check())
    {
        anchor_point mPoint = anchor_point::TOPLEFT;
        if (mFunc.is_provided(0))
            mPoint = static_cast<anchor_point>(mFunc.get(0)->get_int());

        const anchor* pAnchor = pObject_->get_point(mPoint);
        if (pAnchor)
        {
            mFunc.push(anchor::get_string_point(pAnchor->get_point()));
            if (pAnchor->get_parent())
            {
                pAnchor->get_parent()->push_on_lua(mFunc.get_state());
                mFunc.notify_pushed();
            }
            else
                mFunc.push_nil();

            mFunc.push(anchor::get_string_point(pAnchor->get_parent_point()));
            mFunc.push(pAnchor->get_abs_offset_x());
            mFunc.push(pAnchor->get_abs_offset_y());
        }
    }

    return mFunc.on_return();
}

/** @function get_right
*/
int lua_uiobject::_get_right(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:get_right", pLua, 1);

    mFunc.push(pObject_->get_right());

    return mFunc.on_return();
}

/** @function get_top
*/
int lua_uiobject::_get_top(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:get_top", pLua, 1);

    mFunc.push(pObject_->get_top());

    return mFunc.on_return();
}

/** @function get_width
*/
int lua_uiobject::_get_width(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:get_width", pLua, 1);

    mFunc.push(pObject_->get_apparent_width());

    return mFunc.on_return();
}

/** @function hide
*/
int lua_uiobject::_hide(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:hide", pLua);

    pObject_->hide();

    return mFunc.on_return();
}

/** @function is_shown
*/
int lua_uiobject::_is_shown(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:is_shown", pLua, 1);

    mFunc.push(pObject_->is_shown());

    return mFunc.on_return();
}

/** @function is_visible
*/
int lua_uiobject::_is_visible(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:is_visible", pLua, 1);

    mFunc.push(pObject_->is_visible());

    return mFunc.on_return();
}

/** @function set_all_points
*/
int lua_uiobject::_set_all_points(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:set_all_points", pLua);
    mFunc.add(0, "Frame name", lua::type::STRING, true);
    mFunc.add(0, "Frame", lua::type::USERDATA, true);
    if (mFunc.check())
    {
        lua::argument* pArg = mFunc.get(0);
        if (pArg->is_provided())
        {
            uiobject* pFrame = nullptr;
            if (pArg->get_type() == lua::type::STRING)
                pFrame = pObject_->get_manager()->get_uiobject_by_name(pArg->get_string());
            else
            {
                lua_uiobject* pObj = pArg->get<lua_uiobject>();
                if (pObj)
                    pFrame = pObj->get_object();
            }
            pObject_->set_all_points(pFrame);
        }
        else
            pObject_->set_all_points(nullptr);
    }

    return mFunc.on_return();
}

/** @function set_height
*/
int lua_uiobject::_set_height(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:set_height", pLua);
    mFunc.add(0, "height", lua::type::NUMBER);
    if (mFunc.check())
        pObject_->set_abs_height(mFunc.get(0)->get_number());

    return mFunc.on_return();
}

/** @function set_parent
*/
int lua_uiobject::_set_parent(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:set_parent", pLua);
    mFunc.add(0, "parent name", lua::type::STRING, true);
    mFunc.add(0, "parent", lua::type::USERDATA, true);
    if (mFunc.check())
    {
        lua::argument* pArg = mFunc.get(0);
        frame* pNewParentFrame = nullptr;

        if (pArg->is_provided())
        {
            if (pArg->get_type() == lua::type::STRING)
            {
                uiobject* pNewParent = pObject_->get_manager()->get_uiobject_by_name(pArg->get_string());
                if (!pNewParent)
                {
                    gui::out << gui::error << mFunc.get_name() << " : \""+pArg->get_string()
                        +"\" does not exist." << std::endl;
                    return mFunc.on_return();
                }

                pNewParentFrame = down_cast<frame>(pNewParent);
                if (!pNewParentFrame)
                {
                    gui::out << gui::error << mFunc.get_name() << " : \""+pArg->get_string()
                        +"\" is not a frame." << std::endl;
                    return mFunc.on_return();
                }
            }
            else
            {
                lua_frame* pObj = pArg->get<lua_frame>();
                if (!pObj)
                {
                    gui::out << gui::error << mFunc.get_name() << " : Argument 1 must be a frame." << std::endl;
                    return mFunc.on_return();
                }

                pNewParentFrame = pObj->get_object();
            }
        }

        pObject_->set_parent(pNewParentFrame);

        if (pObject_->is_object_type<frame>())
        {
            pNewParentFrame->add_child(down_cast<frame>(pObject_->release_from_parent()));
        }
        else
        {
            pNewParentFrame->add_region(down_cast<layered_region>(pObject_->release_from_parent()));
        }
    }

    return mFunc.on_return();
}

/** @function set_point
*/
int lua_uiobject::_set_point(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:set_point", pLua);
    mFunc.add(0, "point", lua::type::STRING);
    mFunc.add(1, "parent name", lua::type::STRING, true);
    mFunc.add(1, "parent", lua::type::USERDATA, true);
    mFunc.add(2, "relative point", lua::type::STRING, true);
    mFunc.add(3, "x offset", lua::type::NUMBER, true);
    mFunc.add(4, "y offset", lua::type::NUMBER, true);
    if (mFunc.check())
    {
        // point
        anchor_point mPoint = anchor::get_anchor_point(mFunc.get(0)->get_string());

        // parent
        lua::argument* pArg = mFunc.get(1);
        uiobject* pParent = nullptr;
        if (pArg->is_provided())
        {
            if (pArg->get_type() == lua::type::STRING)
            {
                std::string sParent = pArg->get_string();
                if (!utils::has_no_content(sParent))
                    pParent = pObject_->get_manager()->get_uiobject_by_name(sParent);
            }
            else
            {
                lua_uiobject* pLuaObj = pArg->get<lua_uiobject>();
                if (pLuaObj)
                    pParent = pLuaObj->get_object();
            }
        }
        else
            pParent = pObject_->get_parent();

        if (pParent && pParent->depends_on(pObject_))
        {
            gui::out << gui::error << mFunc.get_name() << " : Cyclic anchor dependency ! "
                << "\"" << pObject_->get_name() << "\" and \"" << pParent->get_name() << "\" depend on "
                "eachothers (directly or indirectly).\n\""
                << anchor::get_string_point(mPoint) << "\" anchor not added." << std::endl;
            return mFunc.on_return();
        }

        // relativePoint
        anchor_point mParentPoint = mPoint;
        if (mFunc.is_provided(2))
            mParentPoint = anchor::get_anchor_point(mFunc.get(2)->get_string());

        // x
        float fAbsX = 0;
        if (mFunc.is_provided(3))
            fAbsX = mFunc.get(3)->get_number();

        // y
        float fAbsY = 0;
        if (mFunc.is_provided(4))
            fAbsY = mFunc.get(4)->get_number();

        pObject_->set_abs_point(mPoint, pParent ? pParent->get_name() : "", mParentPoint, fAbsX, fAbsY);
    }

    return mFunc.on_return();
}

/** @function set_rel_point
*/
int lua_uiobject::_set_rel_point(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:set_rel_point", pLua);
    mFunc.add(0, "point", lua::type::STRING);
    mFunc.add(1, "parent name", lua::type::STRING, true);
    mFunc.add(1, "parent", lua::type::USERDATA, true);
    mFunc.add(2, "relative point", lua::type::STRING, true);
    mFunc.add(3, "x offset", lua::type::NUMBER, true);
    mFunc.add(4, "y offset", lua::type::NUMBER, true);
    if (mFunc.check())
    {
        // point
        anchor_point mPoint = anchor::get_anchor_point(mFunc.get(0)->get_string());

        // parent
        lua::argument* pArg = mFunc.get(1);
        uiobject* pParent = nullptr;
        if (pArg->is_provided())
        {
            if (pArg->get_type() == lua::type::STRING)
            {
                std::string sParent = pArg->get_string();
                if (!utils::has_no_content(sParent))
                    pParent = pObject_->get_manager()->get_uiobject_by_name(sParent);
            }
            else
            {
                lua_uiobject* pLuaObj = pArg->get<lua_uiobject>();
                if (pLuaObj)
                    pParent = pLuaObj->get_object();
            }
        }
        else
            pParent = pObject_->get_parent();

        // relativePoint
        anchor_point mParentPoint = mPoint;
        if (mFunc.is_provided(2))
            mParentPoint = anchor::get_anchor_point(mFunc.get(2)->get_string());

        // x
        float fRelX = 0.0f;
        if (mFunc.is_provided(3))
            fRelX = mFunc.get(3)->get_number();

        // y
        float fRelY = 0.0f;
        if (mFunc.is_provided(4))
            fRelY = mFunc.get(4)->get_number();

        pObject_->set_rel_point(mPoint, pParent ? pParent->get_name() : "", mParentPoint, fRelX, fRelY);
    }

    return mFunc.on_return();
}

/** @function set_width
*/
int lua_uiobject::_set_width(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:set_width", pLua);
    mFunc.add(0, "width", lua::type::NUMBER);
    if (mFunc.check())
        pObject_->set_abs_width(mFunc.get(0)->get_number());

    return mFunc.on_return();
}

/** @function show
*/
int lua_uiobject::_show(lua_State* pLua)
{
    if (!check_object_())
        return 0;

    lua::function mFunc("UIObject:show", pLua);

    pObject_->show();

    return mFunc.on_return();
}
}
}
