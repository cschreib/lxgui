#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_anchor.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_layeredregion.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"

#include <lxgui/utils_string.hpp>
#include <lxgui/luapp_function.hpp>

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
    manager* pGUIMgr = manager::get_manager(lua::state::get_state(pLua));
    pParent_ = pGUIMgr->get_uiobject(uiID_);

    if (!pParent_)
        throw exception("lua_virtual_glue", "Glue missing its parent (\""+utils::to_string(uiID_)+"\") !");
}

lua_virtual_glue::~lua_virtual_glue()
{
}

void lua_virtual_glue::notify_deleted()
{
    pParent_ = nullptr;
}

int lua_virtual_glue::_mark_for_copy(lua_State* pLua)
{
    lua::function mFunc("VirtualGlue:mark_for_copy", pLua, 1);
    mFunc.add(0, "variable", lua::TYPE_STRING);
    if (mFunc.check())
        pParent_->mark_for_copy(mFunc.get(0)->get_string());

    return mFunc.on_return();
}

int lua_virtual_glue::_get_name(lua_State* pLua)
{
    lua::function mFunc("VirtualGlue:get_name", pLua, 1);

    mFunc.push(pParent_->get_lua_name());

    return mFunc.on_return();
}

int lua_virtual_glue::_get_base(lua_State* pLua)
{
    lua::function mFunc("VirtualGlue:get_base", pLua, 1);

    if (pParent_->get_base())
    {
        pParent_->get_base()->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

lua_uiobject::lua_uiobject(lua_State* pLua) : lua_glue(pLua)
{
    sName_ = lua_tostring(pLua, -1);
    manager* pGUIMgr = manager::get_manager(lua::state::get_state(pLua));
    pParent_ = pGUIMgr->get_uiobject_by_name(sName_);

    if (!pParent_)
        throw exception("lua_uiobject", "Glue missing its parent (\""+sName_+"\") !");
}

lua_uiobject::~lua_uiobject()
{
}

void lua_uiobject::notify_deleted()
{
    pParent_ = nullptr;
}

uiobject* lua_uiobject::get_parent()
{
    return pParent_;
}

bool lua_uiobject::check_parent_()
{
    if (!pParent_)
    {
        gui::out << gui::warning << sName_ << " : This widget has been deleted and can no longer be used." << std::endl;
        return false;
    }

    return true;
}

int lua_glue::get_data_table(lua_State * pLua)
{
    lua_rawgeti(pLua, LUA_REGISTRYINDEX, iRef_);
    return 1;
}

int lua_uiobject::_get_alpha(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:get_alpha", pLua, 1);

    mFunc.push(pParent_->get_alpha());

    return mFunc.on_return();
}

int lua_uiobject::_get_name(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:get_name", pLua, 1);

    mFunc.push(pParent_->get_name());

    return mFunc.on_return();
}

int lua_uiobject::_get_object_type(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:get_object_type", pLua, 1);

    mFunc.push(pParent_->get_object_type());

    return mFunc.on_return();
}

int lua_uiobject::_is_object_type(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:is_object_type", pLua, 1);
    mFunc.add(0, "object type", lua::TYPE_STRING);
    if (mFunc.check())
    {
        mFunc.push(pParent_->is_object_type(mFunc.get(0)->get_string()));
    }

    return mFunc.on_return();
}

int lua_uiobject::_set_alpha(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:set_alpha", pLua);
    mFunc.add(0, "alpha", lua::TYPE_NUMBER);
    if (mFunc.check())
    {
        pParent_->set_alpha(mFunc.get(0)->get_number());
    }

    return mFunc.on_return();
}

int lua_uiobject::_clear_all_points(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:clear_all_points", pLua);

    pParent_->clear_all_points();

    return mFunc.on_return();
}

int lua_uiobject::_get_bottom(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:get_bottom", pLua, 1);

    mFunc.push(pParent_->get_bottom());

    return mFunc.on_return();
}

int lua_uiobject::_get_center(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:get_center", pLua, 2);

    vector2<int> mP = pParent_->get_center();
    mFunc.push(mP.x);
    mFunc.push(mP.y);

    return mFunc.on_return();
}

int lua_uiobject::_get_height(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:get_height", pLua, 1);

    mFunc.push(pParent_->get_abs_height());

    return mFunc.on_return();
}

int lua_uiobject::_get_left(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:get_left", pLua, 1);

    mFunc.push(pParent_->get_left());

    return mFunc.on_return();
}

int lua_uiobject::_get_num_point(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:get_num_point", pLua, 1);

    mFunc.push(pParent_->get_num_point());

    return mFunc.on_return();
}

int lua_uiobject::_get_parent(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:get_parent", pLua, 1);

    if (pParent_->get_parent())
    {
        pParent_->get_parent()->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

int lua_uiobject::_get_base(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:get_base", pLua, 1);

    if (pParent_->get_base())
    {
        pParent_->get_base()->push_on_lua(mFunc.get_state());
        mFunc.notify_pushed();
    }
    else
        mFunc.push_nil();

    return mFunc.on_return();
}

int lua_uiobject::_get_point(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:get_point", pLua, 5);
    mFunc.add(0, "point ID", lua::TYPE_NUMBER, true);
    if (mFunc.check())
    {
        const std::map<anchor_point, anchor>& lanchorList = pParent_->get_point_list();
        if (!lanchorList.empty())
        {
            uint uianchorID = 1;
            if (mFunc.is_provided(0))
                uianchorID = uint(mFunc.get(0)->get_number());

            const anchor* pAnchor = nullptr;
            uint uiCounter = 1;
            std::map<anchor_point, anchor>::const_iterator iter;
            foreach (iter, lanchorList)
            {
                pAnchor = &iter->second;
                if (uiCounter == uianchorID)
                    break;
                else
                    ++uiCounter;
            }

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

int lua_uiobject::_get_right(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:get_right", pLua, 1);

    mFunc.push(pParent_->get_right());

    return mFunc.on_return();
}

int lua_uiobject::_get_top(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:get_top", pLua, 1);

    mFunc.push(pParent_->get_top());

    return mFunc.on_return();
}

int lua_uiobject::_get_width(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:get_width", pLua, 1);

    mFunc.push(pParent_->get_abs_width());

    return mFunc.on_return();
}

int lua_uiobject::_hide(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:hide", pLua);

    pParent_->hide();

    return mFunc.on_return();
}

int lua_uiobject::_is_shown(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:is_shown", pLua, 1);

    mFunc.push(pParent_->is_shown());

    return mFunc.on_return();
}

int lua_uiobject::_is_visible(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:is_visible", pLua, 1);

    mFunc.push(pParent_->is_visible());

    return mFunc.on_return();
}

int lua_uiobject::_set_all_points(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:set_all_points", pLua);
    mFunc.add(0, "Frame name", lua::TYPE_STRING, true);
    mFunc.add(0, "Frame", lua::TYPE_USERDATA, true);
    if (mFunc.check())
    {
        utils::wptr<lua::argument> pArg = mFunc.get(0);
        if (pArg->is_provided())
        {
            uiobject* pFrame = nullptr;
            if (pArg->get_type() == lua::TYPE_STRING)
                pFrame = pParent_->get_manager()->get_uiobject_by_name(pArg->get_string());
            else
            {
                lua_uiobject* pObj = pArg->get<lua_uiobject>();
                if (pObj)
                    pFrame = pObj->get_parent();
            }
            pParent_->set_all_points(pFrame);
        }
        else
            pParent_->set_all_points(nullptr);
    }

    return mFunc.on_return();
}

int lua_uiobject::_set_height(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:set_height", pLua);
    mFunc.add(0, "height", lua::TYPE_NUMBER);
    if (mFunc.check())
        pParent_->set_abs_height(uint(mFunc.get(0)->get_number()));

    return mFunc.on_return();
}

int lua_uiobject::_set_parent(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:set_parent", pLua);
    mFunc.add(0, "parent name", lua::TYPE_STRING, true);
    mFunc.add(0, "parent", lua::TYPE_USERDATA, true);
    if (mFunc.check())
    {
        utils::wptr<lua::argument> pArg = mFunc.get(0);
        uiobject* pParent = nullptr;

        if (pArg->is_provided())
        {
            if (pArg->get_type() == lua::TYPE_STRING)
            {
                pParent = pParent_->get_manager()->get_uiobject_by_name(pArg->get_string());
                if (!pParent)
                    return mFunc.on_return();
            }
            else
            {
                lua_uiobject* pObj = pArg->get<lua_uiobject>();
                if (pObj)
                    pParent = pObj->get_parent();
                else
                    return mFunc.on_return();
            }
        }

        frame* pFrame = dynamic_cast<frame*>(pParent);
        if (pFrame)
        {
            frame* pOldParent = dynamic_cast<frame*>(pParent_->get_parent());
            pParent_->set_parent(pParent);

            if (pParent_->is_object_type("Frame"))
            {
                frame* pThisFrame = dynamic_cast<frame*>(pParent_);
                if (pOldParent)
                    pOldParent->remove_child(pThisFrame);
                pFrame->add_child(pThisFrame);
            }
            else
            {
                layered_region* pThisRegion = dynamic_cast<layered_region*>(pParent_);
                if (pOldParent)
                    pOldParent->remove_region(pThisRegion);
                pFrame->add_region(pThisRegion);
            }
        }
        else
            gui::out << gui::error << mFunc.get_name() << " : Argument 1 must be a frame." << std::endl;
    }

    return mFunc.on_return();
}

int lua_uiobject::_set_point(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:set_point", pLua);
    mFunc.add(0, "point", lua::TYPE_STRING);
    mFunc.add(1, "parent name", lua::TYPE_STRING, true);
    mFunc.add(1, "parent", lua::TYPE_USERDATA, true);
    mFunc.add(2, "relative point", lua::TYPE_STRING, true);
    mFunc.add(3, "x offset", lua::TYPE_NUMBER, true);
    mFunc.add(4, "y offset", lua::TYPE_NUMBER, true);
    if (mFunc.check())
    {
        // point
        anchor_point mPoint = anchor::get_anchor_point(mFunc.get(0)->get_string());

        // parent
        utils::wptr<lua::argument> pArg = mFunc.get(1);
        uiobject* pParent = nullptr;
        if (pArg->is_provided())
        {
            if (pArg->get_type() == lua::TYPE_STRING)
            {
                std::string sParent = pArg->get_string();
                if (!utils::has_no_content(sParent))
                    pParent = pParent_->get_manager()->get_uiobject_by_name(sParent);
            }
            else
            {
                lua_uiobject* pLuaObj = pArg->get<lua_uiobject>();
                if (pLuaObj)
                    pParent = pLuaObj->get_parent();
            }
        }
        else
            pParent = pParent_->get_parent();

        // relativePoint
        anchor_point mParentPoint = mPoint;
        if (mFunc.is_provided(2))
            mParentPoint = anchor::get_anchor_point(mFunc.get(2)->get_string());

        // x
        int iAbsX = 0;
        if (mFunc.is_provided(3))
            iAbsX = int(mFunc.get(3)->get_number());

        // y
        int iAbsY = 0;
        if (mFunc.is_provided(4))
            iAbsY = int(mFunc.get(4)->get_number());

        pParent_->set_abs_point(mPoint, pParent ? pParent->get_name() : "", mParentPoint, iAbsX, iAbsY);
    }

    return mFunc.on_return();
}

int lua_uiobject::_set_rel_point(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:set_rel_point", pLua);
    mFunc.add(0, "point", lua::TYPE_STRING);
    mFunc.add(1, "parent name", lua::TYPE_STRING, true);
    mFunc.add(1, "parent", lua::TYPE_USERDATA, true);
    mFunc.add(2, "relative point", lua::TYPE_STRING, true);
    mFunc.add(3, "x offset", lua::TYPE_NUMBER, true);
    mFunc.add(4, "y offset", lua::TYPE_NUMBER, true);
    if (mFunc.check())
    {
        // point
        anchor_point mPoint = anchor::get_anchor_point(mFunc.get(0)->get_string());

        // parent
        utils::wptr<lua::argument> pArg = mFunc.get(1);
        uiobject* pParent = nullptr;
        if (pArg->is_provided())
        {
            if (pArg->get_type() == lua::TYPE_STRING)
            {
                std::string sParent = pArg->get_string();
                if (!utils::has_no_content(sParent))
                    pParent = pParent_->get_manager()->get_uiobject_by_name(sParent);
            }
            else
            {
                lua_uiobject* pLuaObj = pArg->get<lua_uiobject>();
                if (pLuaObj)
                    pParent = pLuaObj->get_parent();
            }
        }
        else
            pParent = pParent_->get_parent();

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

        pParent_->set_rel_point(mPoint, pParent ? pParent->get_name() : "", mParentPoint, fRelX, fRelY);
    }

    return mFunc.on_return();
}

int lua_uiobject::_set_width(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:set_width", pLua);
    mFunc.add(0, "width", lua::TYPE_NUMBER);
    if (mFunc.check())
        pParent_->set_abs_width(mFunc.get(0)->get_number());

    return mFunc.on_return();
}

int lua_uiobject::_show(lua_State* pLua)
{
    if (!check_parent_())
        return 0;

    lua::function mFunc("UIObject:show", pLua);

    pParent_->show();

    return mFunc.on_return();
}
}
