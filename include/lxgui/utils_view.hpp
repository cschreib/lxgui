#ifndef LXGUI_UTILS_VIEW_HPP
#define LXGUI_UTILS_VIEW_HPP

#include "lxgui/lxgui.hpp"

#include <utility>

namespace lxgui::utils::view {

/// De-reference an iterator normally
template<typename BaseIterator>
struct standard_dereferencer {
    using data_type = decltype(*std::declval<BaseIterator>());
    static data_type dereference(const BaseIterator& iter) {
        return *iter;
    }
};

/// Convert unique_ptr or shared_ptr to standard pointer
template<typename BaseIterator>
struct smart_ptr_dereferencer {
    using data_type = decltype(*std::declval<BaseIterator>()->get());
    static data_type dereference(const BaseIterator& iter) {
        return *iter->get();
    }
};

/// De-reference an iterator twice
template<typename BaseIterator>
struct ptr_dereferencer {
    using data_type = decltype(**std::declval<BaseIterator>());
    static data_type dereference(const BaseIterator& iter) {
        return **iter;
    }
};

/// No filtering
template<typename BaseIterator>
struct no_filter {
    static bool is_included(const BaseIterator&) {
        return true;
    }
};

/// Filter non-null
template<typename BaseIterator>
struct non_null_filter {
    static bool is_included(const BaseIterator& iter) {
        return *iter != nullptr;
    }
};

/// Allow iterating over a container without access to the container itself
template<
    typename ContainerType,
    template<typename>
    class Dereferencer,
    template<typename>
    class Filter>
class adaptor {
public:
    using base_iterator = std::conditional_t<
        std::is_const_v<ContainerType>,
        typename ContainerType::const_iterator,
        typename ContainerType::iterator>;
    using dereferencer = Dereferencer<base_iterator>;
    using filter       = Filter<base_iterator>;
    using data_type    = typename dereferencer::data_type;

    explicit adaptor(ContainerType& collection) : collection_(collection) {}
    explicit adaptor(ContainerType& collection, dereferencer&& deref, filter&& filter) :
        collection_(collection), deref_(std::move(deref)), filter_(std::move(filter)) {}

    adaptor(const adaptor& other) : collection_(other.collection_) {}
    adaptor(adaptor&& other) : collection_(other.collection_) {}
    adaptor& operator=(const adaptor& other) {
        collection_ = other.collection_;
        return *this;
    }
    adaptor& operator=(adaptor&& other) {
        collection_ = other.collection_;
        return *this;
    }

    class iterator {
    public:
        explicit iterator(const adaptor& adaptor, base_iterator iter) :
            adaptor_(adaptor), iter_(iter) {}

        iterator& operator++() {
            do {
                ++iter_;
            } while (iter_ != adaptor_.collection_.end() && !adaptor_.filter_.is_included(iter_));

            return *this;
        }

        iterator operator++(int) {
            iterator old = *this;
            this->   operator++();
            return old;
        }

        bool operator==(const iterator& other) const {
            return iter_ == other.iter_;
        }
        bool operator!=(const iterator& other) const {
            return iter_ != other.iter_;
        }

        data_type operator*() const {
            return adaptor_.deref_.dereference(iter_);
        }
        data_type operator->() const {
            return adaptor_.deref_.dereference(iter_);
        }

    private:
        const adaptor& adaptor_;
        base_iterator  iter_;
    };

    iterator begin() const {
        iterator iter(*this, collection_.begin());
        if (collection_.begin() != collection_.end() && !filter_.is_included(collection_.begin())) {
            ++iter;
        }
        return iter;
    }

    iterator end() const {
        return iterator(*this, collection_.end());
    }

private:
    ContainerType& collection_;
    dereferencer   deref_;
    filter         filter_;
};

} // namespace lxgui::utils::view

#endif
