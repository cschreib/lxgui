#ifndef LXGUI_UTILS_VIEW_HPP
#define LXGUI_UTILS_VIEW_HPP

#include "lxgui/lxgui.hpp"

#include <utility>

namespace lxgui::utils::view {

/// De-reference an iterator normally
template<typename BaseIterator>
struct standard_dereferencer {
    using data_type = decltype(*std::declval<BaseIterator>());
    static data_type dereference(const BaseIterator& m_iter) {
        return *m_iter;
    }
};

/// Convert unique_ptr or shared_ptr to standard pointer
template<typename BaseIterator>
struct smart_ptr_dereferencer {
    using data_type = decltype(*std::declval<BaseIterator>()->get());
    static data_type dereference(const BaseIterator& m_iter) {
        return *m_iter->get();
    }
};

/// De-reference an iterator twice
template<typename BaseIterator>
struct ptr_dereferencer {
    using data_type = decltype(**std::declval<BaseIterator>());
    static data_type dereference(const BaseIterator& m_iter) {
        return **m_iter;
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
    static bool is_included(const BaseIterator& m_iter) {
        return *m_iter != nullptr;
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

    explicit adaptor(ContainerType& m_collection) : m_collection_(m_collection) {}
    explicit adaptor(ContainerType& m_collection, dereferencer&& m_deref, filter&& m_filter) :
        m_collection_(m_collection), m_deref_(std::move(m_deref)), m_filter_(std::move(m_filter)) {}

    adaptor(const adaptor& m_other) : m_collection_(m_other.m_collection_) {}
    adaptor(adaptor&& m_other) : m_collection_(m_other.m_collection_) {}
    adaptor& operator=(const adaptor& m_other) {
        m_collection_ = m_other.m_collection_;
        return *this;
    }
    adaptor& operator=(adaptor&& m_other) {
        m_collection_ = m_other.m_collection_;
        return *this;
    }

    class iterator {
    public:
        explicit iterator(const adaptor& m_adaptor, base_iterator m_iter) :
            m_adaptor_(m_adaptor), m_iter_(m_iter) {}

        iterator& operator++() {
            do {
                ++m_iter_;
            } while (m_iter_ != m_adaptor_.m_collection_.end() &&
                     !m_adaptor_.m_filter_.is_included(m_iter_));

            return *this;
        }

        iterator operator++(int) {
            iterator m_old = *this;
            this->   operator++();
            return m_old;
        }

        bool operator==(const iterator& m_other) const {
            return m_iter_ == m_other.m_iter_;
        }
        bool operator!=(const iterator& m_other) const {
            return m_iter_ != m_other.m_iter_;
        }

        data_type operator*() const {
            return m_adaptor_.m_deref_.dereference(m_iter_);
        }
        data_type operator->() const {
            return m_adaptor_.m_deref_.dereference(m_iter_);
        }

    private:
        const adaptor& m_adaptor_;
        base_iterator  m_iter_;
    };

    iterator begin() const {
        iterator m_iter(*this, m_collection_.begin());
        if (m_collection_.begin() != m_collection_.end() &&
            !m_filter_.is_included(m_collection_.begin())) {
            ++m_iter;
        }
        return m_iter;
    }

    iterator end() const {
        return iterator(*this, m_collection_.end());
    }

private:
    ContainerType& m_collection_;
    dereferencer   m_deref_;
    filter         m_filter_;
};

} // namespace lxgui::utils::view

#endif
