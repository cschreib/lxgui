#ifndef LXGUI_UTILS_RANGE_HPP
#define LXGUI_UTILS_RANGE_HPP

#include <lxgui/lxgui.hpp>

#include <type_traits>

namespace lxgui {
namespace utils
{
    namespace range
    {
        /** \cond INCLUDE_INTERNALS_IN_DOC
        */
        namespace range_impl
        {
            template<typename T>
            struct reverse_range
            {
                using base = typename std::decay<T>::type;
                using iterator = typename std::conditional<
                    std::is_const<T>::value, typename base::const_reverse_iterator, typename base::reverse_iterator>::type;

                T& lContainer;

                explicit reverse_range(T& c) : lContainer(c) {}

                iterator begin() { return iterator(lContainer.rbegin()); }
                iterator end() { return iterator(lContainer.rend()); }
            };
        }
        /** \endcond
        */

        /// Reverse traversal
        /** \param lContainer The container to traverse
        *   \note Example usage:
        *         \code{.cpp}
        *         std::vector<int> v = { ... };
        *         for (int& i : utils::range::reverse(v)) { ... }
        *         // Equivalent to:
        *         for (auto iter = v.rbegin(); iter != v.rend(); ++iter) { int& i = *iter; ... }
        *         \endcode
        */
        template<typename T> range_impl::reverse_range<T> reverse(T& lContainer)
        {
            return range_impl::reverse_range<T>(lContainer);
        }

        /** \cond INCLUDE_INTERNALS_IN_DOC
        */
        namespace range_impl
        {
            template<typename I>
            struct iterator_adapter
            {
                I iter;
                explicit iterator_adapter(I i) : iter(i) {}
                I operator * () { return iter; }
                iterator_adapter& operator ++ () { ++iter; return *this; }
                iterator_adapter operator ++ (int) { return iter++; }
                bool operator != (const iterator_adapter& o) { return iter != o.iter; }
            };

            template<typename T>
            struct iterator_range
            {
                using base = typename std::decay<T>::type;
                using base_iterator = typename std::conditional<
                    std::is_const<T>::value, typename base::const_iterator, typename base::iterator>::type;
                using iterator = iterator_adapter<base_iterator>;

                T& lContainer;

                explicit iterator_range(T& c) : lContainer(c) {}

                iterator begin() { return iterator(lContainer.begin()); }
                iterator end() { return iterator(lContainer.end()); }
            };
        }
        /** \endcond
        */

        /// Expose the iterator rather than the element
        /** \param lContainer The container to traverse
        *   \note Example usage:
        *         \code{.cpp}
        *         std::vector<int> v = { ... };
        *         for (auto iter : utils::range::iterator(v)) { ... }
        *         // Equivalent to:
        *         for (auto iter = v.begin(); iter != v.end(); ++iter) { ... }
        *         \endcode
        */
        template<typename T> range_impl::iterator_range<T> iterator(T& lContainer)
        {
            return range_impl::iterator_range<T>(lContainer);
        }

        /** \cond INCLUDE_INTERNALS_IN_DOC
        */
        namespace range_impl
        {
            template<typename T>
            struct reverse_iterator_range
            {
                using base = typename std::decay<T>::type;
                using base_iterator = typename std::conditional<
                    std::is_const<T>::value, typename base::const_reverse_iterator, typename base::reverse_iterator>::type;
                using iterator = iterator_adapter<base_iterator>;

                T& lContainer;

                explicit reverse_iterator_range(T& c) : lContainer(c) {}

                iterator begin() { return iterator(lContainer.rbegin()); }
                iterator end() { return iterator(lContainer.rend()); }
            };
        }
        /** \endcond
        */

        /// Expose the iterator rather than the element, with reverse traversal
        /** \param lContainer The container to traverse
        *   \note Example usage:
        *         \code{.cpp}
        *         std::vector<int> v = { ... };
        *         for (auto iter : utils::range::reverse_iterator(v)) { ... }
        *         // Equivalent to:
        *         for (auto iter = v.rbegin(); iter != v.rend(); ++iter) { ... }
        *         \endcode
        */
        template<typename T> range_impl::reverse_iterator_range<T> reverse_iterator(T& lContainer)
        {
            return range_impl::reverse_iterator_range<T>(lContainer);
        }

        /** \cond INCLUDE_INTERNALS_IN_DOC
        */
        namespace range_impl
        {
            template<typename I, typename V>
            struct value_iterator_adapter
            {
                I iter;
                explicit value_iterator_adapter(I i) : iter(i) {}
                V& operator * () { return iter->second; }
                value_iterator_adapter& operator ++ () { ++iter; return *this; }
                value_iterator_adapter operator ++ (int) { return iter++; }
                bool operator != (const value_iterator_adapter& o) { return iter != o.iter; }
            };

            template<typename T>
            struct value_range
            {
                using base = typename std::decay<T>::type;
                using base_iterator = typename std::conditional<
                    std::is_const<T>::value, typename base::const_iterator, typename base::iterator>::type;
                using value_type = typename std::conditional<
                    std::is_const<T>::value, const typename base::mapped_type, typename base::mapped_type>::type;
                using iterator = value_iterator_adapter<base_iterator, value_type>;

                T& lContainer;

                explicit value_range(T& c) : lContainer(c) {}

                iterator begin() { return iterator(lContainer.begin()); }
                iterator end() { return iterator(lContainer.end()); }
            };
        }
        /** \endcond
        */

        /// Expose the value rather than the (key,value) pair
        /** \param lContainer The container to traverse
        *   \note Example usage:
        *         \code{.cpp}
        *         std::map<int,std::string> v = { ... };
        *         for (std::string& s : utils::range::value(v)) { ... }
        *         // Equivalent to:
        *         for (auto iter = v.begin(); iter != v.end(); ++iter) { std::string& s = iter->second; ... }
        *         \endcode
        */
        template<typename T> range_impl::value_range<T> value(T& lContainer)
        {
            return range_impl::value_range<T>(lContainer);
        }

        /** \cond INCLUDE_INTERNALS_IN_DOC
        */
        namespace range_impl
        {
            template<typename T>
            struct reverse_value_range
            {
                using base = typename std::decay<T>::type;
                using base_iterator = typename std::conditional<
                    std::is_const<T>::value, typename base::const_reverse_iterator, typename base::reverse_iterator>::type;
                using value_type = typename std::conditional<
                    std::is_const<T>::value, const typename base::mapped_type, typename base::mapped_type>::type;
                using iterator = value_iterator_adapter<base_iterator, value_type>;

                T& lContainer;

                explicit reverse_value_range(T& c) : lContainer(c) {}

                iterator begin() { return iterator(lContainer.rbegin()); }
                iterator end() { return iterator(lContainer.rend()); }
            };
        }
        /** \endcond
        */

        /// Expose the value rather than the (key,value) pair, with reverse traversal
        /** \param lContainer The container to traverse
        *   \note Example usage:
        *         \code{.cpp}
        *         std::map<int,std::string> v = { ... };
        *         for (std::string& s : utils::range::reverse_value(v)) { ... }
        *         // Equivalent to:
        *         for (auto iter = v.rbegin(); iter != v.rend(); ++iter) { std::string& s = iter->second; ... }
        *         \endcode
        */
        template<typename T> range_impl::reverse_value_range<T> reverse_value(T& lContainer)
        {
            return range_impl::reverse_value_range<T>(lContainer);
        }

        /** \cond INCLUDE_INTERNALS_IN_DOC
        */
        namespace range_impl
        {
            template<typename I, typename K>
            struct key_iterator_adapter
            {
                I iter;
                explicit key_iterator_adapter(I i) : iter(i) {}
                const K& operator * () { return iter->first; }
                key_iterator_adapter& operator ++ () { ++iter; return *this; }
                key_iterator_adapter operator ++ (int) { return iter++; }
                bool operator != (const key_iterator_adapter& o) { return iter != o.iter; }
            };

            template<typename T>
            struct key_range
            {
                using base = typename std::decay<T>::type;
                using base_iterator = typename std::conditional<
                    std::is_const<T>::value, typename base::const_iterator, typename base::iterator>::type;
                using key_type = typename base::key_type;
                using iterator = key_iterator_adapter<base_iterator, key_type>;

                T& lContainer;

                explicit key_range(T& c) : lContainer(c) {}

                iterator begin() { return iterator(lContainer.begin()); }
                iterator end() { return iterator(lContainer.end()); }
            };
        }
        /** \endcond
        */

        /// Expose the key rather than the (key,value) pair
        /** \param lContainer The container to traverse
        *   \note Example usage:
        *         \code{.cpp}
        *         std::map<int,std::string> v = { ... };
        *         for (int k : utils::range::key(v)) { ... }
        *         // Equivalent to:
        *         for (auto iter = v.begin(); iter != v.end(); ++iter) { int k = iter->first; ... }
        *         \endcode
        */
        template<typename T> range_impl::key_range<T> key(T& lContainer)
        {
            return range_impl::key_range<T>(lContainer);
        }

        /** \cond INCLUDE_INTERNALS_IN_DOC
        */
        namespace range_impl
        {
            template<typename T>
            struct reverse_key_range
            {
                using base = typename std::decay<T>::type;
                using base_iterator = typename std::conditional<
                    std::is_const<T>::value, typename base::const_reverse_iterator, typename base::reverse_iterator>::type;
                using key_type = typename base::key_type;
                using iterator = key_iterator_adapter<base_iterator, key_type>;

                T& lContainer;

                explicit reverse_key_range(T& c) : lContainer(c) {}

                iterator begin() { return iterator(lContainer.rbegin()); }
                iterator end() { return iterator(lContainer.rend()); }
            };
        }
        /** \endcond
        */

        /// Expose the key rather than the (key,value) pair, with reverse traversal
        /** \param lContainer The container to traverse
        *   \note Example usage:
        *         \code{.cpp}
        *         std::map<int,std::string> v = { ... };
        *         for (int k : utils::range::key(v)) { ... }
        *         // Equivalent to:
        *         for (auto iter = v.rbegin(); iter != v.rend(); ++iter) { int k = iter->first; ... }
        *         \endcode
        */
        template<typename T> range_impl::reverse_key_range<T> reverse_key(T& lContainer)
        {
            return range_impl::reverse_key_range<T>(lContainer);
        }
    }
}
}

#endif
