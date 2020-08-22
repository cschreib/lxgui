#ifndef LXGUI_UTILS_VIEW_HPP
#define LXGUI_UTILS_VIEW_HPP

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

    /// Convert unique_ptr to standard pointer
    template<typename BaseIterator>
    struct unique_ptr_dereferencer
    {
        using data_type = decltype(std::declval<BaseIterator>()->get());
        static data_type dereference(const BaseIterator& mIter) { return mIter->get(); }
    };

    /// De-reference an iterator twice
    template<typename BaseIterator>
    struct ptr_dereferencer
    {
        using data_type = decltype(**std::declval<BaseIterator>());
        static data_type dereference(const BaseIterator& mIter) { return **mIter; }
    };

    /// Allow iterating over a container without access to the container itself
    template<typename ContainerType, template<typename> class Dereferencer>
    class adaptor
    {
    public:

        using base_iterator = typename ContainerType::const_iterator;
        using dereferencer = Dereferencer<base_iterator>;
        using data_type = typename dereferencer::data_type;

        explicit adaptor(const ContainerType& mCollection) : mCollection_(mCollection) {}

        adaptor(const adaptor& mOther) : mCollection_(mOther.mCollection_) {}
        adaptor(adaptor&& mOther) : mCollection_(mOther.mCollection_) {}
        adaptor& operator=(const adaptor& mOther) { mCollection_ = mOther.mCollection_; return *this; }
        adaptor& operator=(adaptor&& mOther) { mCollection_ = mOther.mCollection_; return *this; }

        class iterator
        {
        public:
            explicit iterator(base_iterator mIter) : mIter_(mIter) {}

            iterator& operator++() { ++mIter_; return *this; }
            iterator operator++(int) { return iterator(mIter_++); }

            bool operator == (const iterator& mOther) const { return mIter_ == mOther.mIter_; }
            bool operator != (const iterator& mOther) const { return mIter_ != mOther.mIter_; }

            data_type operator* () const { return dereferencer::dereference(mIter_); }
            data_type operator-> () const { return dereferencer::dereference(mIter_); }

        private:

            base_iterator mIter_;
        };

        iterator begin() const
        {
            return iterator(mCollection_.begin());
        }

        iterator end() const
        {
            return iterator(mCollection_.end());
        }

    private:

        const ContainerType& mCollection_;
    };
}
}
}

#endif
