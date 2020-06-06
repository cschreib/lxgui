#ifndef SORTED_VECTOR_HPP
#define SORTED_VECTOR_HPP

#include <vector>
#include <algorithm>
#include <functional>
#include <memory>

namespace lxgui {
namespace utils
{
    /// Sorted std::vector wrapper.
    /** This class is a light alternative to std::set.
        Inspired from: [1] www.lafstern.org/matt/col1.pdf

        The sorted vector achieves the same O(log(N)) look up complexity as std::set, but with a
        lower constant of proportionality thanks to the binary search algorithm (twice lower
        according to [1]). The fact that this container uses an std::vector internally also implies
        that it has the lowest possible memory usage.

        On the other hand, it has worse insertion complexity (O(N), compared to the O(log(N)) of
        std::set). This drawback can be completely ignored if either:

         - one does much more look-ups than insertions,
         - all the data are available when the vector is created, so that the insertions are all
           performed in one go,
         - and, even better, if one builds an already sorted std::vector and creates a sorted_vector
           out of it.

        The elements of this container are sorted according to the Cmp template argument, which
        defaults to std::less<T> (i.e. the elements are sorted using operator<()). One can provide
        a custom comparison functor if std::less is not desirable. Using a custom comparison functor
        can also allow to find elements by keys instead of their own value. For example, if T is

        \code{.cpp}
            struct test { int id; };
        \endcode

        then one can use the following comparison functor:

        \code{.cpp}
            struct comp {
                bool operator() (const test& n1, const test& n2) const {
                    return n1.id < n2.id;
                }
                bool operator() (const test& n1, int i) const {
                    return n1.id < i;
                }
                bool operator() (int i, const test& n2) const {
                    return i < n2.id;
                }
            };
        \endcode

        and use an integer as the key to find elements in the
        container

        \code{.cpp}
            using vec_t = sorted_vector<test, comp>;
            vec_t vec;
            // ... fill vec with some data ...
            // Then find the element that has the 'id' equal to 5
            vec_t::iterator it = vec.find(5);
        \endcode
    **/
    template<typename T, typename Cmp = std::less<T>>
    class sorted_vector : private std::vector<T> {
        using base = std::vector<T>;
        Cmp compare;

    public :
        using iterator = typename base::iterator;
        using const_iterator = typename base::const_iterator;
        using reverse_iterator = typename base::reverse_iterator;
        using const_reverse_iterator = typename base::const_reverse_iterator;

        /// Default constructor.
        /** Creates an empty vector.
        **/
        sorted_vector() = default;

        /// Copy another vector into this one.
        sorted_vector(const sorted_vector& s) = default;

        /// Move another sorted_vector into this one.
        /** The other vector is left empty in the process, and all its content is transfered into
            this new one.
        **/
        sorted_vector(sorted_vector&& s) = default;

        /// Copy another sorted_vector into this one.
        sorted_vector& operator = (const sorted_vector& s) = default;

        /// Move another vector into this one.
        /** The other vector is left empty in the process, and all its content is transfered into
            this new one.
        **/
        sorted_vector& operator = (sorted_vector&& s) = default;

        /// Copy a pre-built vector into this one.
        /** The provided vector must be sorted, and shall not contain any duplicate value.
        **/
        explicit sorted_vector(const base& s) : base(s) {}

        /// Move a pre-built vector into this one.
        /** The provided vector must be sorted, and shall not contain any duplicate value. It is
            left empty in the process, and all its content is transfered into this new
            sorted_vector.
        **/
        explicit sorted_vector(base&& s) : base(std::move(s)) {}

        /// Default constructor, with comparator.
        /** Creates an empty vector and sets the comparator function.
        **/
        explicit sorted_vector(const Cmp& c) : compare(c) {}

        /// Write an initializer list into this vector.
        /** The list need not be sorted.
        **/
        sorted_vector(std::initializer_list<T> l) {
            for (auto& e : l) {
                insert(e);
            }
        }

        /// Insert a copy of the provided object in the vector.
        /** If an object already exists with the same key, it is destroyed and replaced by this
            copy.
        **/
        iterator insert(const T& t) {
            return insert(T(t));
        }

        /// Insert the provided object in the vector.
        /** If an object already exists with the same key, it is destroyed and replaced by this one.
        **/
        iterator insert(T&& t) {
            if (empty()) {
                base::push_back(std::move(t));
                return base::begin();
            } else {
                auto iter = std::lower_bound(begin(), end(), t, compare);
                if (iter != end() && !compare(t, *iter)) {
                    *iter = std::move(t);
                } else {
                    iter = base::insert(iter, std::move(t));
                }
                return iter;
            }
        }

        /// Erase an element from this vector.
        iterator erase(iterator iter) {
            return base::erase(iter);
        }

        /// Erase a range from this vector.
        iterator erase(iterator first, iterator last) {
            return base::erase(first, last);
        }

        /// Erase an element from this vector by its key.
        /** The key can be a copy of the element itself, or any other object that is supported by
            the chosen comparison function. If no object is found with that key, this function does
            nothing.
        **/
        template<typename Key>
        iterator erase(const Key& k) {
            auto iter = find(k);
            if (iter != end()) {
                return base::erase(iter);
            } else {
                return end();
            }
        }

        /// Find an object in this vector by its key.
        /** The key can be a copy of the element itself, or any other object that is supported by
            the chosen comparison function. If no element is found, this function returns end().
        **/
        template<typename Key>
        iterator find(const Key& k) {
            if (!empty()) {
                auto iter = std::lower_bound(begin(), end(), k, compare);
                if (iter != end() && !compare(k, *iter)) {
                    return iter;
                }
            }
            return end();
        }

        /// Find an object in this vector by its key.
        /** The key can be a copy of the element itself, or any other object that is supported by
            the chosen comparison function. If no element is found, this function returns end().
        **/
        template<typename Key>
        const_iterator find(const Key& k) const {
            if (!empty()) {
                auto iter = std::lower_bound(begin(), end(), k, compare);
                if (iter != end() && !compare(k, *iter)) {
                    return iter;
                }
            }
            return end();
        }

        using base::front;
        using base::back;
        using base::size;
        using base::max_size;
        using base::capacity;
        using base::reserve;
        using base::empty;
        using base::clear;
        using base::pop_back;

        using base::begin;
        using base::rbegin;
        using base::end;
        using base::rend;
    };
}
}

#endif
