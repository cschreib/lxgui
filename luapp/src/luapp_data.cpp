#include "lxgui/luapp_state.hpp"
#include "lxgui/luapp_function.hpp"

namespace lua
{
data::data() : sName_(""), mLuaType_(TYPE_NIL), pParent_(nullptr)
{
}

data::data(const std::string& name, type mLuaType, argument* pParent) :
    sName_(name), mLuaType_(mLuaType), pParent_(pParent)
{
}

void data::set(state* pLua, int iIndex)
{
    if (mLuaType_ == TYPE_BOOLEAN)
        mValue_ = pLua->get_bool(iIndex);
    else if (mLuaType_ == TYPE_NUMBER)
        mValue_ = pLua->get_number(iIndex);
    else if (mLuaType_ == TYPE_STRING)
        mValue_ = pLua->get_string(iIndex);
    else if (mLuaType_ == TYPE_TABLE)
        mValue_ = iIndex;
    else if (mLuaType_ == TYPE_FUNCTION)
        mValue_ = iIndex;
    else if (mLuaType_ == TYPE_USERDATA)
        mValue_ = iIndex;
    else if (mLuaType_ == TYPE_NIL)
        mValue_ = var();

    pParent_->set_data(this);
}

const std::string& data::get_name() const
{
    return sName_;
}

const var& data::get_value() const
{
    return mValue_;
}

type data::get_type() const
{
    return mLuaType_;
}
}
