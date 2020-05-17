#ifndef UTILS_WPTR_HPP
#define UTILS_WPTR_HPP

#include "lxgui/utils.hpp"

namespace lxgui {
namespace utils
{
template<class T>
class refptr;

/// Weak pointer
/** This is a companion of the reference counter pointer.<br>
*   Use the lock() function to get a valid pointer, or a nullptr
*   if it has been deleted.
*/
template<class T>
class wptr
{
public :

    template<class N> friend class wptr;

    /// Default constructor.
    /** \note Initializes the pointer to nullptr.
    */
    wptr() = default;

    /// Copy constructor.
    wptr(const wptr& pPtr) noexcept
    {
        pValue_    = pPtr.pValue_;
        pCounter_  = pPtr.pCounter_;
        pWCounter_ = pPtr.pWCounter_;

        increment_();
    }

    /// Move constructor.
    wptr(wptr&& pPtr) noexcept
    {
        pValue_    = pPtr.pValue_;
        pCounter_  = pPtr.pCounter_;
        pWCounter_ = pPtr.pWCounter_;

        pPtr.pValue_    = nullptr;
        pPtr.pCounter_  = nullptr;
        pPtr.pWCounter_ = nullptr;
    }

    /// Conversion from nullptr.
    wptr(const std::nullptr_t&) noexcept
    {
        pValue_    = nullptr;
        pCounter_  = nullptr;
        pWCounter_ = nullptr;
    }

    /// Destructor.
    /** \note This function will <b>never</b> delete the pointed object (if any).
    */
    ~wptr() noexcept
    {
        decrement_();
    }

    template<class N>
    /// refptr conversion.
    wptr(const refptr<N>& pRefPtr) noexcept
    {
        pValue_    = pRefPtr.pValue_;
        pCounter_  = pRefPtr.pCounter_;
        pWCounter_ = pRefPtr.pWCounter_;

        increment_();
    }

    template<class N>
    /// wptr conversion.
    wptr(const wptr<N>& pPtr) noexcept
    {
        pValue_    = pPtr.pValue_;
        pCounter_  = pPtr.pCounter_;
        pWCounter_ = pPtr.pWCounter_;

        increment_();
    }

    template<class N>
    /// wptr conversion move.
    wptr(wptr<N>&& pPtr) noexcept
    {
        pValue_    = pPtr.pValue_;
        pCounter_  = pPtr.pCounter_;
        pWCounter_ = pPtr.pWCounter_;

        pPtr.pValue_    = nullptr;
        pPtr.pCounter_  = nullptr;
        pPtr.pWCounter_ = nullptr;
    }

    /// Checks if this pointer points to a valid object.
    /** \note The pointer can be invalid if the object has
    *         been deleted somewhere else.
    */
    bool is_valid() const noexcept
    {
        return (pCounter_ && *pCounter_ != 0u);
    }

    /// Returns the number of refptr pointing to the object.
    /** \return The number of refptr pointing to the object
    *   \note If this function returns 0, then the object has
    *         been deleted and this pointer is invalid, or it
    *         is simply nullptr.
    */
    uint get_count() const noexcept
    {
        if (pCounter_)
            return *pCounter_;
        else
            return 0u;
    }

    /// Returns the number of wptr pointing to the object.
    /** \return The number of wptr pointing to the object
    *   \note This function returns 0 if the pointer is nullptr.
    */
    uint get_weak_count() const noexcept
    {
        if (pWCounter_)
            return *pWCounter_;
        else
            return 0u;
    }

    /// Sets this pointer to nullptr.
    /** \note This function will <b>never</b> delete the pointed object (if any).
    */
    void set_null() noexcept
    {
        decrement_();

        pValue_    = nullptr;
        pCounter_  = nullptr;
        pWCounter_ = nullptr;
    }

    /// Creates a refptr pointing to the object (if any).
    /** \returns A refptr pointing to the object (if any)
    *   \note If this pointer is invalid, this function returns an empty
    *         refptr.
    */
    refptr<const T> lock() const noexcept
    {
        if (is_valid())
            return refptr<const T>(pValue_, pCounter_, pWCounter_);
        else
            return refptr<const T>();
    }

    /// Creates a refptr pointing to the object (if any).
    /** \returns A refptr pointing to the object (if any)
    *   \note If this pointer is invalid, this function returns an empty
    *         refptr.
    */
    refptr<T> lock() noexcept
    {
        if (is_valid())
            return refptr<T>(pValue_, pCounter_, pWCounter_);
        else
            return refptr<T>();
    }

    /// Dereferences the pointer.
    /** \return The contained pointer
    *   \note You should test for the pointer's validity before using
    *         this operator, unless you're sure the object is still
    *         alive.
    */
    const T* operator -> () const noexcept
    {
        return pValue_;
    }

    /// Dereferences the pointer.
    /** \return The contained pointer
    *   \note You should test for the pointer's validity before using
    *         this operator, unless you're sure the object is still
    *         alive.
    */
    T* operator -> () noexcept
    {
        return pValue_;
    }

    /// Returns a reference to the contained value.
    /** \return A reference to the contained value
    */
    const T& operator * () const noexcept
    {
        return *pValue_;
    }

    /// Returns a reference to the contained value.
    /** \return A reference to the contained value
    */
    T& operator * () noexcept
    {
        return *pValue_;
    }

    /// nullptr assignation operator.
    /** \param pPtr The value to copy
    */
    wptr& operator = (const std::nullptr_t& pPtr) noexcept
    {
        decrement_();

        pValue_ = nullptr;

        return *this;
    }

    /// Copy operator.
    /** \param pPtr The value to copy
    */
    wptr& operator = (const wptr& pPtr) noexcept
    {
        decrement_();

        pValue_ = pPtr.pValue_;
        pCounter_ = pPtr.pCounter_;
        pWCounter_ = pPtr.pWCounter_;

        increment_();

        return *this;
    }

    template<class N>
    /// Copy operator.
    /** \param pPtr The value to copy
    */
    wptr& operator = (const wptr<N>& pPtr) noexcept
    {
        decrement_();

        pValue_ = pPtr.pValue_;
        pCounter_ = pPtr.pCounter_;
        pWCounter_ = pPtr.pWCounter_;

        increment_();

        return *this;
    }

    /// Move operator.
    /** \param pPtr The value to move
    */
    wptr& operator = (wptr&& pPtr) noexcept
    {
        decrement_();

        pValue_ = pPtr.pValue_;
        pCounter_ = pPtr.pCounter_;
        pWCounter_ = pPtr.pWCounter_;

        pPtr.pValue_    = nullptr;
        pPtr.pCounter_  = nullptr;
        pPtr.pWCounter_ = nullptr;

        return *this;
    }

    template<class N>
    /// Move operator.
    /** \param pPtr The value to move
    */
    wptr& operator = (wptr<N>&& pPtr) noexcept
    {
        decrement_();

        pValue_ = pPtr.pValue_;
        pCounter_ = pPtr.pCounter_;
        pWCounter_ = pPtr.pWCounter_;

        pPtr.pValue_    = nullptr;
        pPtr.pCounter_  = nullptr;
        pPtr.pWCounter_ = nullptr;

        return *this;
    }

    /// Checks if this pointer equals another
    /** \param pValue The pointer to test
    */
    template<class N>
    bool operator == (const wptr<N>& pValue) const noexcept
    {
        return (pValue_ == pValue.pValue_);
    }

    /// Checks if this pointer equals another
    /** \param pValue The pointer to test
    */
    template<class N>
    bool operator == (const refptr<N>& pValue) const noexcept
    {
        return (pValue_ == pValue.pValue_);
    }

    /// Checks if this pointer equals another
    /** \param pValue The pointer to test
    */
    template<class N>
    bool operator == (N* pValue) const noexcept
    {
        return (pValue_ == pValue);
    }

    /// Checks if this pointer is null
    /** \param pValue The null pointer
    *   \return 'true' if this pointer is null
    */
    bool operator == (std::nullptr_t pValue) const noexcept
    {
        return (pValue_ == 0);
    }

    /// Checks if this pointer is different from another
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer is different from another
    */
    template<class N>
    bool operator != (const wptr<N>& pValue) const noexcept
    {
        return (pValue_ != pValue.pValue_);
    }

    /// Checks if this pointer is different from another
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer is different from another
    */
    template<class N>
    bool operator != (const refptr<N>& pValue) const noexcept
    {
        return (pValue_ != pValue.pValue_);
    }

    /// Checks if this pointer is different from another
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer is different from another
    */
    template<class N>
    bool operator != (N* pValue) const noexcept
    {
        return (pValue_ != pValue);
    }

    /// Checks if this pointer is not null
    /** \param pValue The null pointer
    *   \return 'true' if this pointer is not null
    */
    bool operator != (std::nullptr_t pValue) const noexcept
    {
        return (pValue_ != 0);
    }

    /// Checks if this pointer's value is lower than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is lower than the other
    */
    template<class N>
    bool operator < (const wptr<N>& pValue) const noexcept
    {
        return (pValue_ < pValue.pValue_);
    }

    /// Checks if this pointer's value is lower than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is lower than the other
    */
    template<class N>
    bool operator < (const refptr<N>& pValue) const noexcept
    {
        return (pValue_ < pValue.pValue_);
    }

    /// Checks if this pointer's value is lower than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is lower than the other
    */
    template<class N>
    bool operator < (N* pValue) const noexcept
    {
        return (pValue_ < pValue);
    }

    /// Checks if this pointer's value is lower or equal than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is lower or equal than the other
    */
    template<class N>
    bool operator <= (const wptr<N>& pValue) const noexcept
    {
        return (pValue_ <= pValue.pValue_);
    }

    /// Checks if this pointer's value is lower or equal than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is lower or equal than the other
    */
    template<class N>
    bool operator <= (const refptr<N>& pValue) const noexcept
    {
        return (pValue_ <= pValue.pValue_);
    }

    /// Checks if this pointer's value is lower or equal than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is lower or equal than the other
    */
    template<class N>
    bool operator <= (N* pValue) const noexcept
    {
        return (pValue_ <= pValue);
    }

    /// Checks if this pointer's value is greater than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is greater than the other
    */
    template<class N>
    bool operator > (const wptr<N>& pValue) const noexcept
    {
        return (pValue_ > pValue.pValue_);
    }

    /// Checks if this pointer's value is greater than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is greater than the other
    */
    template<class N>
    bool operator > (const refptr<N>& pValue) const noexcept
    {
        return (pValue_ > pValue.pValue_);
    }

    /// Checks if this pointer's value is greater than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is greater than the other
    */
    template<class N>
    bool operator > (N* pValue) const noexcept
    {
        return (pValue_ > pValue);
    }

    /// Checks if this pointer's value is greater or equal than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is greater or equal than the other
    */
    template<class N>
    bool operator >= (const wptr<N>& pValue) const noexcept
    {
        return (pValue_ >= pValue.pValue_);
    }

    /// Checks if this pointer's value is greater or equal than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is greater or equal than the other
    */
    template<class N>
    bool operator >= (const refptr<N>& pValue) const noexcept
    {
        return (pValue_ >= pValue.pValue_);
    }

    /// Checks if this pointer's value is greater or equal than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is greater or equal than the other
    */
    template<class N>
    bool operator >= (N* pValue) const noexcept
    {
        return (pValue_ >= pValue);
    }

    /// Allows : "if (!pPointer)".
    bool operator ! () const noexcept
    {
        return (pCounter_ == 0 ? true : *pCounter_ == 0u);
    }

    /// Allows : "if (pPointer)".
    explicit operator bool() const noexcept
    {
        return (pCounter_ && *pCounter_ != 0u);
    }

    /// Casts the provided pointer to this one's type.
    /** \param pValue The pointer to cast
    *   \return The new casted pointer
    */
    template<class N>
    static wptr<T> cast(const wptr<N>& pValue) noexcept
    {
        return wptr<T>(static_cast<T*>(pValue.pValue_), pValue.pCounter_, pValue.pWCounter_);
    }

    /// Tries to dynamic cast the provided pointer to this one's type.
    /** \param pValue The pointer to cast
    *   \return The new casted pointer
    *   \note Dynamic cast can fail, and in this case, will result in
    *         a nullptr pointer.
    */
    template<class N>
    static wptr<T> dyn_cast(const wptr<N>& pValue) noexcept
    {
        T* pTemp = dynamic_cast<T*>(pValue.pValue_);
        if (pTemp)
            return wptr<T>(pTemp, pValue.pCounter_, pValue.pWCounter_);
        else
            return wptr<T>();
    }

private :

    wptr(T* pValue, uint* pCounter, uint* pWCounter) noexcept :
        pValue_(pValue), pCounter_(pCounter), pWCounter_(pWCounter)
    {
        increment_();
    }

    void increment_() noexcept
    {
        if (pWCounter_)
            ++(*pWCounter_);
    }

    void decrement_() noexcept
    {
        if (pWCounter_)
        {
            --(*pWCounter_);
            if (*pWCounter_ == 0u && *pCounter_ == 0u)
            {
                delete pCounter_;
                pCounter_ = nullptr;
                delete pWCounter_;
                pWCounter_ = nullptr;
            }
        }
    }

    T*    pValue_ = nullptr;
    uint* pCounter_ = nullptr;
    uint* pWCounter_ = nullptr;
};
}
}


#endif
