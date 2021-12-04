#ifndef LXGUI_UTILS_VIEW_HPP
#define LXGUI_UTILS_VIEW_HPP

#include <lxgui/lxgui.hpp>

#include <utility>

namespace lxgui {
namespace utils {
namespace view
{
    /// De-reference an iterator normally
    template<typename BaseIterator>
    struct standard_dereferencer
    {
        using data_type = decltype(*std::declval<BaseIterator>());
        static data_type dereference(const BaseIterator& mIter) { return *mIter; }
    };

    /// Convert unique_ptr or shared_ptr to standard pointer
    template<typename BaseIterator>
    struct smart_ptr_dereferencer
    {
        using data_type = decltype(*std::declval<BaseIterator>()->get());
        static data_type dereference(const BaseIterator& mIter) { return *mIter->get(); }
    };

    /// De-reference an iterator twice
    template<typename BaseIterator>
    struct ptr_dereferencer
    {
        using data_type = decltype(**std::declval<BaseIterator>());
        static data_type dereference(const BaseIterator& mIter) { return **mIter; }
    };

    /// No filtering
    template<typename BaseIterator>
    struct no_filter
    {
        static bool is_included(const BaseIterator&) { return true; }
    };

    /// Filter non-null
    template<typename BaseIterator>
    struct non_null_filter
    {
        static bool is_included(const BaseIterator& mIter) { return *mIter != nullptr; }
    };

    /// Allow iterating over a container without access to the container itself
    template<typename ContainerType, template<typename> class Dereferencer, template<typename> class Filter>
    class adaptor
    {
    public:

        using base_iterator = std::conditional_t<std::is_const_v<ContainerType>,
            typename ContainerType::const_iterator,
            typename ContainerType::iterator>;
        using dereferencer = Dereferencer<base_iterator>;
        using filter = Filter<base_iterator>;
        using data_type = typename dereferencer::data_type;

        explicit adaptor(ContainerType& mCollection) : mCollection_(mCollection) {}

        adaptor(const adaptor& mOther) : mCollection_(mOther.mCollection_) {}
        adaptor(adaptor&& mOther) : mCollection_(mOther.mCollection_) {}
        adaptor& operator=(const adaptor& mOther) { mCollection_ = mOther.mCollection_; return *this; }
        adaptor& operator=(adaptor&& mOther) { mCollection_ = mOther.mCollection_; return *this; }

        class iterator
        {
        public:
            explicit iterator(ContainerType& mCollection, base_iterator mIter) :
                mCollection_(mCollection), mIter_(mIter) {}

            iterator& operator++()
            {
                do
                {
                    ++mIter_;
                } while (mIter_ != mCollection_.end() && !filter::is_included(mIter_));

                return *this;
            }

            iterator operator++(int)
            {
                iterator mOld = *this;
                this->operator++();
                return mOld;
            }

            bool operator == (const iterator& mOther) const { return mIter_ == mOther.mIter_; }
            bool operator != (const iterator& mOther) const { return mIter_ != mOther.mIter_; }

            data_type operator* () const { return dereferencer::dereference(mIter_); }
            data_type operator-> () const { return dereferencer::dereference(mIter_); }

        private:

            ContainerType& mCollection_;
            base_iterator mIter_;
        };

        iterator begin() const
        {
            iterator mIter(mCollection_, mCollection_.begin());
            if (mCollection_.begin() != mCollection_.end() &&
                !filter::is_included(mCollection_.begin()))
            {
                ++mIter;
            }
            return mIter;
        }

        iterator end() const
        {
            return iterator(mCollection_, mCollection_.end());
        }

    private:

        ContainerType& mCollection_;
    };
}
}
}

#endif
