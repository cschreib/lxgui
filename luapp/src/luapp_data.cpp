#include "lxgui/luapp_state.hpp"
#include "lxgui/luapp_function.hpp"

namespace lxgui {
namespace lua
{
data::data(const std::string& name, type mLuaType, argument* pParent) :
    sName_(name), mLuaType_(mLuaType), pParent_(pParent)
{
}

void data::set(state& mLua, int iIndex)
{
    if (mLuaType_ == type::BOOLEAN)
        mValue_ = mLua.get_bool(iIndex);
    else if (mLuaType_ == type::NUMBER)
        mValue_ = mLua.get_number(iIndex);
    else if (mLuaType_ == type::STRING)
        mValue_ = mLua.get_string(iIndex);
    else if (mLuaType_ == type::TABLE)
        mValue_ = iIndex;
    else if (mLuaType_ == type::FUNCTION)
        mValue_ = iIndex;
    else if (mLuaType_ == type::USERDATA)
        mValue_ = iIndex;
    else if (mLuaType_ == type::NIL)
        mValue_ = utils::any();

    pParent_->set_data(this);
}

const std::string& data::get_name() const
{
    return sName_;
}

const utils::any& data::get_value() const
{
    return mValue_;
}

type data::get_type() const
{
    return mLuaType_;
}
}
}
