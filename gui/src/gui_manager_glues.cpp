#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_uiobject.hpp"
#include "lxgui/gui_frame.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/input.hpp"

#include <lxgui/luapp_state.hpp>
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
    static lua::lunar_binding<lua_manager> methods[];

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

manager* manager::get_manager(lua::state& mState)
{
    mState.get_global("_MGR");
    manager* pMgr = mState.get<lua_manager>()->get_manager();
    mState.pop();
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
lua::lunar_binding<lua_manager> lua_manager::methods[] = {
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
        lua::state& mState = mFunc.get_state();
        mState.get_global("_MGR");
        manager* pGUIMgr = mState.get<lua_manager>()->get_manager();
        mState.pop();

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
        lua::state& mState = mFunc.get_state();
        std::string sType = mFunc.get(0)->get_string();
        std::string sName = mFunc.get(1)->get_string();

        mState.get_global("_MGR");
        manager* pGUIMgr = mState.get<lua_manager>()->get_manager();
        mState.pop();

        frame* pParent = nullptr;
        if (mFunc.is_provided(2) && mFunc.get(2)->get_type() == lua::type::USERDATA)
        {
            lua_frame* pFrameObj = mFunc.get(2)->get<lua_frame>();
            if (pFrameObj)
            {
                pParent = pFrameObj->get_object();
            }
            else
            {
                lua_uiobject* pObj = mFunc.get(2)->get<lua_uiobject>();
                if (pObj)
                {
                    gui::out << gui::error << mFunc.get_name()
                        << " : The second argument of " << mFunc.get_name() << " must be a frame "
                        << "(got a " << pObj->get_object()->get_object_type() << ")." << std::endl;
                }
                else
                {
                    gui::out << gui::error << mFunc.get_name()
                        << " : The second argument of " << mFunc.get_name() << " must be a frame."
                        << std::endl;

                }

                return mFunc.on_return();
            }
        }

        std::string sInheritance;
        if (mFunc.get(3)->is_provided())
            sInheritance = mFunc.get(3)->get_string();

        auto lInheritance = pGUIMgr->get_virtual_uiobject_list(sInheritance);

        frame* pNewFrame = nullptr;
        if (pParent)
            pNewFrame = pParent->create_child(sType, sName, lInheritance);
        else
            pNewFrame = pGUIMgr->create_root_frame(sType, sName, lInheritance);

        if (pNewFrame)
        {
            pNewFrame->push_on_lua(mState);
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
                    << " : The first argument of " << mFunc.get_name() << " must be a frame "
                    << "(got a " << pObj->get_object()->get_object_type() << ")." << std::endl;
            }
            else
            {
                gui::out << gui::error << mFunc.get_name()
                    << " : The first argument of " << mFunc.get_name() << " must be a frame."
                    << std::endl;

            }

            return mFunc.on_return();
        }

        frame* pFrame = pFrameObj->get_object();
        if (!pFrame)
        {
            gui::out << gui::error << mFunc.get_name()
                << " : Frame '" << pFrameObj->get_name() << "' is already deleted." << std::endl;

            return mFunc.on_return();
        }


        lua::state& mState = mFunc.get_state();
        mState.get_global("_MGR");
        manager* pGUIMgr = mState.get<lua_manager>()->get_manager();
        mState.pop();

        pGUIMgr->delayed_delete_frame(down_cast<frame>(pFrame->release_from_parent()));
        pFrameObj->clear_object();
    }

    return mFunc.on_return();
}

int l_get_locale(lua_State* pLua)
{
    lua::function mFunc("get_locale", pLua, 1);

    lua::state& mState = mFunc.get_state();
    mState.get_global("_MGR");
    manager* pGUIMgr = mState.get<lua_manager>()->get_manager();
    mState.pop();

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
