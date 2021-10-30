#ifndef LXGUI_UTILS_OBSERVER_HPP
#define LXGUI_UTILS_OBSERVER_HPP

#include <lxgui/lxgui.hpp>
#include "lxgui/utils.hpp"

#include <memory>

namespace lxgui {
namespace utils
{

/// std::unique_ptr that can be observed by std::weak_ptr
/** This smart pointer mimics the interface of std::unique_ptr, in that
*   it is movable but not copiable. The smart pointer holds exclusive
*   (unique) ownership of the pointed object. The main difference with
*   std::unique_ptr is that it allows std::weak_ptr instances to be
*   constructed, to observe the lifetime of the pointed object, as one
*   would do with std::shared_ptr. The price to pay, compared to a standard
*   std::unique_ptr, is the additional heap allocation of the
*   reference-counting control block, which utils::make_observable_unique()
*   will optimise as a single heap allocation (as std::make_shared).
*/
template<typename T>
class observable_unique_ptr : private std::shared_ptr<T>
{
private:
    explicit observable_unique_ptr(std::shared_ptr<T>&& pValue) : std::shared_ptr<T>(std::move(pValue)) {}

    template<typename U>
    friend class observable_unique_ptr;

public:
    using std::shared_ptr<T>::reset;
    using std::shared_ptr<T>::swap;
    using std::shared_ptr<T>::get;
    using std::shared_ptr<T>::operator*;
    using std::shared_ptr<T>::operator->;
    using std::shared_ptr<T>::operator bool;

    /// Default constructor (null pointer).
    observable_unique_ptr() noexcept = default;

    /// Construct a null pointer.
    observable_unique_ptr(std::nullptr_t) {}

    /// Explicit ownership capture of a raw pointer.
    /** \param pValue The raw pointer to take ownership of
    *   \note Do *not* manually delete this raw pointer after
    *         the observable_unique_ptr is created.
    */
    explicit observable_unique_ptr(T* pValue) : std::shared_ptr<T>(pValue) {}

    /// Transfer ownership by implicit casting
    /** \param pValue The pointer to take ownership from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U>
    observable_unique_ptr(observable_unique_ptr<U>&& pValue) :
        std::shared_ptr<T>(std::move(static_cast<std::shared_ptr<U>&>(pValue))) {}

    /// Transfer ownership by explicit casting
    /** \param pManager The smart pointer to take ownership from
    *   \param pValue The casted pointer value to take ownership of
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U>
    observable_unique_ptr(observable_unique_ptr<U>&& pManager, T* pValue) :
        std::shared_ptr<T>(pManager, pValue)
    {
        // Resets source to nullptr
        observable_unique_ptr<U> pReseter = std::move(pManager);
    }

    /// Transfer ownership by implicit casting
    /** \param pValue The pointer to take ownership from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U>
    observable_unique_ptr& operator=(observable_unique_ptr<U>&& pValue)
    {
        std::shared_ptr<T>::operator=(std::move(static_cast<std::shared_ptr<U>&>(pValue)));
        return *this;
    }

    // Movable
    observable_unique_ptr(observable_unique_ptr&&) noexcept = default;
    observable_unique_ptr& operator=(observable_unique_ptr&&) noexcept = default;

    // Non-copyable
    observable_unique_ptr(const observable_unique_ptr&) = delete;
    observable_unique_ptr& operator=(const observable_unique_ptr&) = delete;

    template<typename U, typename ... Args>
    friend observable_unique_ptr<U> make_observable_unique(Args&& ... mArgs);
};

/// Create a new observable_unique_ptr with a newly constructed object.
/** \param mArgs Arguments to construt the new object
*   \return The new observable_unique_ptr
*/
template<typename T, typename ... Args>
observable_unique_ptr<T> make_observable_unique(Args&& ... mArgs)
{
    return observable_unique_ptr<T>(std::make_shared<T>(std::forward<Args>(mArgs)...));
}

template<typename T>
bool operator== (const observable_unique_ptr<T>& pValue, std::nullptr_t)
{
    return pValue.get() == nullptr;
}

template<typename T>
bool operator== (std::nullptr_t, const observable_unique_ptr<T>& pValue)
{
    return pValue.get() == nullptr;
}

template<typename T>
bool operator!= (const observable_unique_ptr<T>& pValue, std::nullptr_t)
{
    return pValue.get() != nullptr;
}

template<typename T>
bool operator!= (std::nullptr_t, const observable_unique_ptr<T>& pValue)
{
    return pValue.get() != nullptr;
}

template<typename T, typename U>
bool operator== (const observable_unique_ptr<T>& pFirst, const observable_unique_ptr<U>& pSecond)
{
    return pFirst.get() == pSecond.get();
}

template<typename T, typename U>
bool operator!= (const observable_unique_ptr<T>& pFirst, const observable_unique_ptr<U>& pSecond)
{
    return pFirst.get() != pSecond.get();
}

}
}

#endif
