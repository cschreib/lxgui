#ifndef UTILS_HPP
#define UTILS_HPP

#include <cmath>
#include <memory>

typedef unsigned long ulong;
typedef unsigned int  uint;
typedef unsigned char uchar;

namespace math
{
    #ifdef MSVC
    template<typename T>
    inline bool isinf(T x)
    {
        return !_finite(x);
    }
    #else
    using std::isinf;
    #endif
}

namespace utils
{
    namespace range
    {
        template<typename T>
        struct reverse_range
        {
            using base = typename std::decay<T>::type;
            using iterator = typename std::conditional<
                std::is_const<T>::value, typename base::const_reverse_iterator, typename base::reverse_iterator>::type;

            T& container;

            explicit reverse_range(T& c) : container(c) {}

            iterator begin() { return iterator(container.rbegin()); }
            iterator end() { return iterator(container.rend()); }
        };

        template<typename T> reverse_range<T> reverse(T& container)
        {
            return reverse_range<T>(container);
        }

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

            T& container;

            explicit iterator_range(T& c) : container(c) {}

            iterator begin() { return iterator(container.begin()); }
            iterator end() { return iterator(container.end()); }
        };

        template<typename T> iterator_range<T> iterator(T& container)
        {
            return iterator_range<T>(container);
        }

        template<typename T>
        struct reverse_iterator_range
        {
            using base = typename std::decay<T>::type;
            using base_iterator = typename std::conditional<
                std::is_const<T>::value, typename base::const_reverse_iterator, typename base::reverse_iterator>::type;
            using iterator = iterator_adapter<base_iterator>;

            T& container;

            explicit reverse_iterator_range(T& c) : container(c) {}

            iterator begin() { return iterator(container.rbegin()); }
            iterator end() { return iterator(container.rend()); }
        };

        template<typename T> reverse_iterator_range<T> reverse_iterator(T& container)
        {
            return reverse_iterator_range<T>(container);
        }

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

            T& container;

            explicit value_range(T& c) : container(c) {}

            iterator begin() { return iterator(container.begin()); }
            iterator end() { return iterator(container.end()); }
        };

        template<typename T> value_range<T> value(T& container)
        {
            return value_range<T>(container);
        }

        template<typename T>
        struct reverse_value_range
        {
            using base = typename std::decay<T>::type;
            using base_iterator = typename std::conditional<
                std::is_const<T>::value, typename base::const_reverse_iterator, typename base::reverse_iterator>::type;
            using value_type = typename std::conditional<
                std::is_const<T>::value, const typename base::mapped_type, typename base::mapped_type>::type;
            using iterator = value_iterator_adapter<base_iterator, value_type>;

            T& container;

            explicit reverse_value_range(T& c) : container(c) {}

            iterator begin() { return iterator(container.rbegin()); }
            iterator end() { return iterator(container.rend()); }
        };

        template<typename T> reverse_value_range<T> reverse_value(T& container)
        {
            return reverse_value_range<T>(container);
        }

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

            T& container;

            explicit key_range(T& c) : container(c) {}

            iterator begin() { return iterator(container.begin()); }
            iterator end() { return iterator(container.end()); }
        };

        template<typename T> key_range<T> key(T& container)
        {
            return key_range<T>(container);
        }

        template<typename T>
        struct reverse_key_range
        {
            using base = typename std::decay<T>::type;
            using base_iterator = typename std::conditional<
                std::is_const<T>::value, typename base::const_reverse_iterator, typename base::reverse_iterator>::type;
            using key_type = typename base::key_type;
            using iterator = key_iterator_adapter<base_iterator, key_type>;

            T& container;

            explicit reverse_key_range(T& c) : container(c) {}

            iterator begin() { return iterator(container.rbegin()); }
            iterator end() { return iterator(container.rend()); }
        };

        template<typename T> reverse_key_range<T> reverse_key(T& container)
        {
            return reverse_key_range<T>(container);
        }
    }
}

#endif
