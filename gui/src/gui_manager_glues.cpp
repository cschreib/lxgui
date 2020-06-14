#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/input.hpp"

#include <lxgui/luapp_function.hpp>

namespace lxgui {
namespace gui
{
/** \cond NOT_REMOVE_FROM_DOC
*/

class lua_manager
{
public :

    explicit lua_manager(lua_State* luaVM);
    virtual ~lua_manager();

    void set_manager(manager* pMgr);
    manager* get_manager();

    int get_data_table(lua_State *L);

    static const char className[];
    static const char* classList[];
    static lua::Lunar<lua_manager>::RegType methods[];

protected :

    lua_State* pLua_;
    int        iRef_;

    manager* pMgr_;
};

/** \endcond
*/

void manager::register_lua_manager_()
{
    pLua_->reg<lua_manager>();
    lua_manager* pLuaMgr = pLua_->push_new<lua_manager>();
    pLuaMgr->set_manager(this);
    pLua_->set_global("_MGR");
}

manager* manager::get_manager(lua::state* pState)
{
    pState->get_global("_MGR");
    manager* pMgr = pState->get<lua_manager>()->get_manager();
    pState->pop();
    return pMgr;
}

lua_manager::lua_manager(lua_State* pLua)
{
    lua_newtable(pLua);
    iRef_ = luaL_ref(pLua, LUA_REGISTRYINDEX);
    pLua_ = pLua;
}

lua_manager::~lua_manager()
{
    luaL_unref(pLua_, LUA_REGISTRYINDEX, iRef_);
}

void lua_manager::set_manager(manager* pMgr)
{
    pMgr_ = pMgr;
}

manager* lua_manager::get_manager()
{
    return pMgr_;
}

int lua_manager::get_data_table(lua_State* pLua)
{
    lua_rawgeti(pLua, LUA_REGISTRYINDEX, iRef_);
    return 1;
}

const char lua_manager::className[] = "manager";
const char* lua_manager::classList[] = {"manager", 0};
lua::Lunar<lua_manager>::RegType lua_manager::methods[] = {
    {"dt", &lua_manager::get_data_table},
    {0,0}
};

int l_set_key_binding(lua_State* pLua)
{
    lua::function mFunc("set_key_binding", pLua);
    mFunc.add(0, "key", lua::type::NUMBER);
    mFunc.add(1, "code", lua::type::STRING);
    mFunc.add(1, "nil", lua::type::NIL);
    mFunc.new_param_set();
    mFunc.add(0, "key", lua::type::NUMBER);
    mFunc.add(1, "modifier", lua::type::NUMBER);
    mFunc.add(2, "code", lua::type::STRING);
    mFunc.add(2, "nil", lua::type::NIL);
    mFunc.new_param_set();
    mFunc.add(0, "key", lua::type::NUMBER);
    mFunc.add(1, "modifier1", lua::type::NUMBER);
    mFunc.add(2, "modifier2", lua::type::NUMBER);
    mFunc.add(3, "code", lua::type::STRING);
    mFunc.add(3, "nil", lua::type::NIL);

    if (mFunc.check())
    {
        lua::state* pState = mFunc.get_state();
        pState->get_global("_MGR");
        manager* pGUIMgr = pState->get<lua_manager>()->get_manager();
        pState->pop();

        input::key uiKey = static_cast<input::key>(mFunc.get(0)->get_int());

        if (mFunc.get_param_set_rank() == 0)
        {
            if (mFunc.is_provided(1) && mFunc.get(1)->get_type() == lua::type::STRING)
                pGUIMgr->set_key_binding(uiKey, mFunc.get(1)->get_string());
            else
                pGUIMgr->remove_key_binding(uiKey);
        }
        else if (mFunc.get_param_set_rank() == 1)
        {
            input::key uiModifier = static_cast<input::key>(mFunc.get(1)->get_int());

            if (mFunc.is_provided(2) && mFunc.get(2)->get_type() == lua::type::STRING)
                pGUIMgr->set_key_binding(uiKey, uiModifier, mFunc.get(2)->get_string());
            else
                pGUIMgr->remove_key_binding(uiKey, uiModifier);
        }
        else
        {
            input::key uiModifier1 = static_cast<input::key>(mFunc.get(1)->get_int());
            input::key uiModifier2 = static_cast<input::key>(mFunc.get(2)->get_int());

            if (mFunc.is_provided(3) && mFunc.get(3)->get_type() == lua::type::STRING)
                pGUIMgr->set_key_binding(uiKey, uiModifier1, uiModifier2, mFunc.get(3)->get_string());
            else
                pGUIMgr->remove_key_binding(uiKey, uiModifier1, uiModifier2);
        }
    }

    return mFunc.on_return();
}

int l_create_frame(lua_State* pLua)
{
    lua::function mFunc("create_frame", pLua, 1);
    mFunc.add(0, "type", lua::type::STRING);
    mFunc.add(1, "name", lua::type::STRING);
    mFunc.add(2, "parent", lua::type::USERDATA, true);
    mFunc.add(2, "parent", lua::type::NIL, true);
    mFunc.add(3, "inherits", lua::type::STRING, true);

    if (mFunc.check())
    {
        lua::state* pState = mFunc.get_state();
        std::string sType = mFunc.get(0)->get_string();
        std::string sName = mFunc.get(1)->get_string();

        pState->get_global("_MGR");
        manager* pGUIMgr = pState->get<lua_manager>()->get_manager();
        pState->pop();

        frame* pParent = nullptr;
        if (mFunc.is_provided(2) && mFunc.get(2)->get_type() == lua::type::USERDATA)
        {
            lua_frame* pFrameObj = mFunc.get(2)->get<lua_frame>();
            if (pFrameObj)
            {
                pParent = pFrameObj->get_parent()->down_cast<frame>();
            }
            else
            {
                lua_uiobject* pObj = mFunc.get(2)->get<lua_uiobject>();
                if (pObj)
                {
                    gui::out << gui::error << mFunc.get_name()
                        << "The second argument of " << mFunc.get_name() << " must be a frame "
                        << "(got a " << pObj->get_parent()->get_object_type() << ")." << std::endl;
                }
                else
                {
                    gui::out << gui::error << mFunc.get_name()
                        << "The second argument of " << mFunc.get_name() << " must be a frame."
                        << std::endl;

                }

                return mFunc.on_return();
            }
        }

        std::string sInheritance;
        if (mFunc.get(3)->is_provided())
            sInheritance = mFunc.get(3)->get_string();

        frame* pNewFrame = nullptr;
        if (pParent)
            pNewFrame = pParent->create_child(sType, sName, sInheritance);
        else
        {
            std::unique_ptr<frame> pFrame = pGUIMgr->create_frame(sType, sName, sInheritance);
            pNewFrame = pFrame.get();
            pGUIMgr->add_root_uiobject(std::move(pFrame));
        }

        if (pNewFrame)
        {
            pNewFrame->push_on_lua(pState);
            mFunc.notify_pushed();
        }
        else
            mFunc.push_nil();
    }

    return mFunc.on_return();
}

int l_delete_frame(lua_State* pLua)
{
    lua::function mFunc("delete_frame", pLua);
    mFunc.add(0, "frame", lua::type::USERDATA);

    if (mFunc.check())
    {
        lua_frame* pFrameObj = mFunc.get(0)->get<lua_frame>();
        if (!pFrameObj)
        {
            lua_uiobject* pObj = mFunc.get(0)->get<lua_uiobject>();
            if (pObj)
            {
                gui::out << gui::error << mFunc.get_name()
                    << "The first argument of " << mFunc.get_name() << " must be a frame "
                    << "(got a " << pObj->get_parent()->get_object_type() << ")." << std::endl;
            }
            else
            {
                gui::out << gui::error << mFunc.get_name()
                    << "The first argument of " << mFunc.get_name() << " must be a frame."
                    << std::endl;

            }

            return mFunc.on_return();
        }

        pFrameObj->get_parent()->down_cast<frame>()->release_from_parent();
    }

    return mFunc.on_return();
}

int l_get_locale(lua_State* pLua)
{
    lua::function mFunc("get_locale", pLua, 1);

    lua::state* pState = mFunc.get_state();
    pState->get_global("_MGR");
    manager* pGUIMgr = pState->get<lua_manager>()->get_manager();
    pState->pop();

    mFunc.push(pGUIMgr->get_locale());

    return mFunc.on_return();
}

int l_log(lua_State* pLua)
{
    lua::function mFunc("log", pLua);
    mFunc.add(0, "message", lua::type::STRING);

    if (mFunc.check())
        gui::out << mFunc.get(0)->get_string() << std::endl;

    return mFunc.on_return();
}
}
}
