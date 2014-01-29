#include "lxgui/luapp_var.hpp"
#include <lxgui/utils_string.hpp>

namespace lua
{
const var_type& var::VALUE_NONE    = typeid(void);
const var_type& var::VALUE_INT     = typeid(int);
const var_type& var::VALUE_UINT    = typeid(uint);
const var_type& var::VALUE_FLOAT   = typeid(float);
const var_type& var::VALUE_DOUBLE  = typeid(double);
const var_type& var::VALUE_BOOL    = typeid(bool);
const var_type& var::VALUE_STRING  = typeid(std::string);
const var_type& var::VALUE_POINTER = typeid(void*);

var::var() : pValue_(nullptr)
{
}

var::var(const var& mVar) : pValue_(mVar.pValue_ ? mVar.pValue_->clone() : nullptr)
{
}

var& var::operator = (const var& mVar)
{
    if (&mVar != this)
    {
        var mTemp(mVar);
        swap(mTemp);
    }
    return *this;
}

bool var::operator == (const var& mVar) const
{
    if (get_type() == mVar.get_type())
    {
        if (is_of_type<std::string>())
        {
            std::string s1 = get<std::string>();
            std::string s2 = mVar.get<std::string>();
            return s1 == s2;
        }
        else if (is_of_type<float>())
        {
            float f1 = get<float>();
            float f2 = mVar.get<float>();
            return f1 == f2;
        }
        else if (is_of_type<double>())
        {
            double d1 = get<double>();
            double d2 = mVar.get<double>();
            return d1 == d2;
        }
        else if (is_of_type<int>())
        {
            int i1 = get<int>();
            int i2 = mVar.get<int>();
            return i1 == i2;
        }
        else if (is_of_type<uint>())
        {
            uint ui1 = get<uint>();
            uint ui2 = mVar.get<uint>();
            return ui1 == ui2;
        }
        else if (is_of_type<bool>())
        {
            bool b1 = get<bool>();
            bool b2 = mVar.get<bool>();
            return b1 == b2;
        }
        else if (is_of_type<void>())
            return true;
    }

    return false;
}

bool var::operator != (const var& mVar) const
{
    return !(operator == (mVar));
}

void var::swap(var& mVar)
{
    std::swap(pValue_, mVar.pValue_);
}

bool var::is_empty() const
{
    return (pValue_ == nullptr);
}


const var_type& var::get_type() const
{
    if (pValue_)
    {
        return pValue_->get_type();
    }
    else
    {
        return typeid(void);
    }
}

std::string var::to_string() const
{
    const var_type& mType = get_type();
    if (mType == VALUE_INT) return utils::to_string(get<int>());
    else if (mType == VALUE_UINT) return utils::to_string(get<uint>())+"u";
    else if (mType == VALUE_FLOAT) return utils::to_string(get<float>())+"f";
    else if (mType == VALUE_DOUBLE) return utils::to_string(get<double>());
    else if (mType == VALUE_BOOL) return utils::to_string(get<bool>());
    else if (mType == VALUE_STRING) return "\""+get<std::string>()+"\"";
    else if (mType == VALUE_POINTER) return utils::to_string(get<void*>());
    else return "<none>";
}
}
