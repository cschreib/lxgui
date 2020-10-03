#include "lxgui/luapp_state.hpp"
#include "lxgui/luapp_function.hpp"
#include <lxgui/utils_string.hpp>

namespace lxgui {
namespace lua
{
argument::argument(const std::string& sName, type mLuaType, function* pParent) :
    pParent_(pParent), mLua_(pParent_->get_state())
{
    lData_.push_back(data(sName, mLuaType, this));
    pData_ = &lData_[0];
}

void argument::add(const std::string& sName, type mLuaType)
{
    lData_.push_back(data(sName, mLuaType, this));
    if (lData_.size() == 1)
        pData_ = &lData_[0];
}

float argument::get_number() const
{
    return float(utils::get<double>(pData_->get_value()));
}

int argument::get_int() const
{
    return int(utils::get<double>(pData_->get_value()));
}

bool argument::get_bool() const
{
    return utils::get<bool>(pData_->get_value());
}

std::string argument::get_string() const
{
    return utils::get<std::string>(pData_->get_value());
}

int argument::get_index() const
{
    return utils::get<int>(pData_->get_value());
}

type argument::get_type() const
{
    return pData_->get_type();
}

bool argument::is_provided() const
{
    return bSet_;
}

bool argument::test(state& mLua, int iIndex, bool bPrintError)
{
    bool bSeveralChoices = (lData_.size() > 1);

    type mType = mLua.get_type(iIndex);
    for (auto& mData : lData_)
    {
        if (mType != mData.get_type())
        {
            if (bPrintError && !bSeveralChoices)
            {
                mLua.print_error(
                    "argument "+utils::to_string(iIndex)+" of \""+pParent_->get_name()+"\" "
                    "must be a "+mLua.get_type_name(mData.get_type())+" "
                    "("+mData.get_name()+") (got a "+mLua.get_type_name(mType)+")."
                );
            }
        }
        else
        {
            mData.set(mLua, iIndex);
            break;
        }
    }

    if (!bSet_)
    {
        if (bPrintError && bSeveralChoices)
        {
            std::string sEnum = "";
            uint i = 0;
            for (const auto& mData : lData_)
            {
                if (i == 0)
                    sEnum += "a ";
                else
                {
                    if (i == lData_.size()-1)
                        sEnum += ", or a ";
                    else
                        sEnum += ", a ";
                }

                sEnum += mLua.get_type_name(mData.get_type())+" ("+mData.get_name()+")";
                ++i;
            }
            mLua.print_error(
                "argument "+utils::to_string(iIndex)+" of \""+pParent_->get_name()+"\" "
                "must be "+sEnum+" (got a "+mLua.get_type_name(mType)+")."
            );
        }
    }

    return bSet_;
}

void argument::set_data(data* pData)
{
    bSet_ = true;
    pData_ = pData;
}

data* argument::get_data() const
{
    return pData_;
}
}
}
