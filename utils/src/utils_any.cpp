#include "lxgui/utils_any.hpp"
#include "lxgui/utils_string.hpp"

namespace lxgui {
namespace utils
{
const any_type& any::VALUE_NONE    = typeid(void);
const any_type& any::VALUE_INT     = typeid(int);
const any_type& any::VALUE_UINT    = typeid(uint);
const any_type& any::VALUE_FLOAT   = typeid(float);
const any_type& any::VALUE_DOUBLE  = typeid(double);
const any_type& any::VALUE_BOOL    = typeid(bool);
const any_type& any::VALUE_STRING  = typeid(std::string);
const any_type& any::VALUE_POINTER = typeid(void*);

any::any(const any& mAny) : pValue_(mAny.pValue_ ? mAny.pValue_->clone() : nullptr)
{
}

any& any::operator = (const any& mAny)
{
    if (&mAny != this)
    {
        any mTemp(mAny);
        swap(mTemp);
    }

    return *this;
}

template<typename T>
bool compare(const any& mAny1, const any& mAny2)
{
    return mAny1.get<T>() == mAny2.get<T>();
}

bool any::operator == (const any& mAny) const
{
    if (get_type() != mAny.get_type())
        return false;

    if (is_of_type<std::string>())
    {
        return compare<std::string>(*this, mAny);
    }
    else if (is_of_type<float>())
    {
        return compare<float>(*this, mAny);
    }
    else if (is_of_type<double>())
    {
        return compare<double>(*this, mAny);
    }
    else if (is_of_type<int>())
    {
        return compare<int>(*this, mAny);
    }
    else if (is_of_type<uint>())
    {
        return compare<uint>(*this, mAny);
    }
    else if (is_of_type<bool>())
    {
        return compare<bool>(*this, mAny);
    }
    else if (is_of_type<void>())
        return true;
    else
        return false;
}

bool any::operator != (const any& mAny) const
{
    return !(operator == (mAny));
}

void any::swap(any& mAny)
{
    std::swap(pValue_, mAny.pValue_);
}

bool any::is_empty() const
{
    return (pValue_ == nullptr);
}

const any_type& any::get_type() const
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

std::string any::to_string() const
{
    const any_type& mType = get_type();

    if (mType == VALUE_INT)          return utils::to_string(get<int>());
    else if (mType == VALUE_UINT)    return utils::to_string(get<uint>())+"u";
    else if (mType == VALUE_FLOAT)   return utils::to_string(get<float>())+"f";
    else if (mType == VALUE_DOUBLE)  return utils::to_string(get<double>());
    else if (mType == VALUE_BOOL)    return utils::to_string(get<bool>());
    else if (mType == VALUE_STRING)  return "\""+get<std::string>()+"\"";
    else if (mType == VALUE_POINTER) return utils::to_string(get<void*>());
    else                             return "<none>";
}
}
}
