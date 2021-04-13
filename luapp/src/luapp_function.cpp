#include "lxgui/luapp_state.hpp"
#include "lxgui/luapp_function.hpp"

#include <lxgui/utils_string.hpp>
#include <lxgui/utils_std.hpp>
#include <lxgui/utils_range.hpp>

namespace lxgui {
namespace lua
{
function::function(const std::string& sName, lua_State* pLua, uint uiReturnNbr) :
    sName_(sName), mLua_(pLua), uiReturnNbr_(uiReturnNbr)
{
    new_param_set();
}

void function::add(uint uiIndex, const std::string& sName, type mLuaType, bool bOptional)
{
    if (bOptional)
    {
        if (pArgList_->lOptional_.find(uiIndex) != pArgList_->lOptional_.end())
            pArgList_->lOptional_[uiIndex]->add(sName, mLuaType);
        else
            pArgList_->lOptional_[uiIndex] = std::unique_ptr<argument>(new argument(sName, mLuaType, this));
    }
    else
    {
        if (pArgList_->lArg_.find(uiIndex) != pArgList_->lArg_.end())
            pArgList_->lArg_[uiIndex]->add(sName, mLuaType);
        else
            pArgList_->lArg_[uiIndex] = std::unique_ptr<argument>(new argument(sName, mLuaType, this));
    }
}

void function::new_param_set()
{
    lArgListStack_.push_back(argument_list());
    pArgList_ = &lArgListStack_.back();
    pArgList_->uiRank_ = lArgListStack_.size()-1;
}

uint function::get_param_set_rank()
{
    return pArgList_->uiRank_;
}

argument* function::get(uint uiIndex)
{
    if (pArgList_->lArg_.find(uiIndex) != pArgList_->lArg_.end())
        return pArgList_->lArg_[uiIndex].get();

    if (pArgList_->lOptional_.find(uiIndex) != pArgList_->lOptional_.end())
        return pArgList_->lOptional_[uiIndex].get();

    return nullptr;
}

bool function::is_provided(uint uiIndex) const
{
    auto iter = pArgList_->lArg_.find(uiIndex);
    if (iter != pArgList_->lArg_.end())
        return iter->second->is_provided();

    iter = pArgList_->lOptional_.find(uiIndex);
    if (iter != pArgList_->lOptional_.end())
        return iter->second->is_provided();

    return false;
}

uint function::get_argument_count() const
{
    return uiArgumentCount_;
}

bool function::check(bool bPrintError)
{
    uiArgumentCount_ = mLua_.get_top();

    // Check if that's enough
    std::vector<argument_list*> lValidArgList;
    for (auto& mArgList : lArgListStack_)
    {
        if (uiArgumentCount_ >= mArgList.lArg_.size())
            lValidArgList.push_back(&mArgList);
    }

    if (lValidArgList.empty())
    {
        if (bPrintError)
        {
            std::string sError;
            for (auto& mArgList : lArgListStack_)
            {
                std::string sArguments = "\n  - ["+utils::to_string(uint(mArgList.lArg_.size()))+"] : ";
                for (auto iterArg : utils::range::iterator(mArgList.lArg_))
                {
                    if (iterArg != mArgList.lArg_.begin())
                        sArguments += ", ";

                    sArguments += iterArg->second->get_data()->get_name();
                }

                if (!mArgList.lOptional_.empty())
                {
                    if (!sArguments.empty())
                        sArguments += ", ";

                    sArguments += "(+";

                    for (auto iterArg : utils::range::iterator(mArgList.lOptional_))
                    {
                        if (iterArg != mArgList.lOptional_.begin())
                            sArguments += ", ";
                        sArguments += iterArg->second->get_data()->get_name();
                    }
                    sArguments += ")";
                }

                sError += sArguments;
            }

            if (lArgListStack_.size() == 1)
                mLua_.print_error("Too few arguments in \""+sName_+"\". Expected :"+sError);
            else
                mLua_.print_error("Too few arguments in \""+sName_+"\". Expected either :"+sError);
        }

        // So if there isn't enough, just return false
        return false;
    }

    // We then check the value type
    int i = 0;
    if (lValidArgList.size() > 1)
    {
        pArgList_ = nullptr;
        for (auto* pArgList : lValidArgList)
        {
            i = 1;
            bool bValid = true;
            for (auto& mArg : pArgList->lArg_)
            {
                if (!mArg.second->test(mLua_, i, false))
                {
                    bValid = false;
                    break;
                }
                ++i;
            }

            if (bValid)
            {
                pArgList_ = pArgList;
                break;
            }
        }

        if (!pArgList_)
        {
            if (bPrintError)
            {
                std::string sError;
                for (auto& mArgList : lArgListStack_)
                {
                    std::string sArguments = "\n  - ["+utils::to_string(uint(mArgList.lArg_.size()))+"] : ";
                    for (auto iterArg : utils::range::iterator(mArgList.lArg_))
                    {
                        if (iterArg != mArgList.lArg_.begin())
                            sArguments += ", ";
                        sArguments += iterArg->second->get_data()->get_name();
                    }

                    if (!mArgList.lOptional_.empty())
                    {
                        if (!sArguments.empty())
                            sArguments += ", ";

                        sArguments += "(+";

                        for (auto iterArg : utils::range::iterator(mArgList.lOptional_))
                        {
                            if (iterArg != mArgList.lOptional_.begin())
                                sArguments += ", ";
                            sArguments += iterArg->second->get_data()->get_name();
                        }

                        sArguments += ")";
                    }

                    sError += sArguments;
                }

                mLua_.print_error(
                    "Wrong arguments provided to \""+sName_+"\". Expected either :"+sError
                );
            }
            return false;
        }
    }
    else
    {
        pArgList_ = lValidArgList[0];
        i = 1;
        bool bValid = true;
        for (auto& mArg : pArgList_->lArg_)
        {
            if (!mArg.second->test(mLua_, i, bPrintError))
                bValid = false;
            ++i;
        }

        if (!bValid)
            return false;
    }

    // We fill the stack with nil value until there are enough for optional arguments
    uint uiMaxArgs = pArgList_->lArg_.size() + pArgList_->lOptional_.size();
    if (uiArgumentCount_ < uiMaxArgs)
        mLua_.push_nil(uiMaxArgs - uiArgumentCount_);

    // And we check optional arguments
    bool bValid = true;
    for (auto& mArg : pArgList_->lOptional_)
    {
        if (mLua_.get_type(i) != type::NIL)
        {
            if (!mArg.second->test(mLua_, i, bPrintError))
                bValid = false;
        }
        ++i;
    }

    return bValid;
}

const std::string& function::get_name() const
{
    return sName_;
}

void function::push(const std::string& sValue)
{
    if (uiReturnCount_ == uiReturnNbr_)
        ++uiReturnNbr_;

    mLua_.push_string(sValue);

    ++uiReturnCount_;
}

void function::push(double dValue)
{
    if (uiReturnCount_ == uiReturnNbr_)
        ++uiReturnNbr_;

    mLua_.push_number(dValue);

    ++uiReturnCount_;
}

void function::push(float fValue)
{
    if (uiReturnCount_ == uiReturnNbr_)
        ++uiReturnNbr_;

    mLua_.push_number(fValue);

    ++uiReturnCount_;
}

void function::push(int iValue)
{
    if (uiReturnCount_ == uiReturnNbr_)
        ++uiReturnNbr_;

    mLua_.push_number(iValue);

    ++uiReturnCount_;
}

void function::push(uint uiValue)
{
    if (uiReturnCount_ == uiReturnNbr_)
        ++uiReturnNbr_;

    mLua_.push_number(uiValue);

    ++uiReturnCount_;
}

void function::push(bool bValue)
{
    if (uiReturnCount_ == uiReturnNbr_)
        ++uiReturnNbr_;

    mLua_.push_bool(bValue);

    ++uiReturnCount_;
}

void function::push_nil(uint uiNbr)
{
    for (uint ui = 0; ui < uiNbr; ++ui)
    {
        if (uiReturnCount_ == uiReturnNbr_)
            ++uiReturnNbr_;

        mLua_.push_nil();

        ++uiReturnCount_;
    }
}

void function::notify_pushed()
{
    if (uiReturnCount_ == uiReturnNbr_)
        ++uiReturnNbr_;

    ++uiReturnCount_;
}

int function::on_return()
{
    // Fill with nil value
    if (uiReturnCount_ < uiReturnNbr_)
        mLua_.push_nil(uiReturnNbr_ - uiReturnCount_);

    // Return the number of returned value
    return uiReturnNbr_;
}

state& function::get_state()
{
    return mLua_;
}
}
}
