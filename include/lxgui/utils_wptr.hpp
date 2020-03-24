#ifndef UTILS_WPTR_HPP
#define UTILS_WPTR_HPP

#include "lxgui/utils.hpp"

typedef decltype(nullptr) nullptr_t;

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
    /** \note Initializes the pointer to NULL.
    */
    wptr()
    {
        pValue_    = nullptr;
        pCounter_  = nullptr;
        pWCounter_ = nullptr;
    }

    /// Copy constructor.
    wptr(const wptr& pPtr)
    {
        pValue_    = pPtr.pValue_;
        pCounter_  = pPtr.pCounter_;
        pWCounter_ = pPtr.pWCounter_;

        increment_();
    }

    /// Move constructor.
    wptr(wptr&& pPtr)
    {
        pValue_    = pPtr.pValue_;
        pCounter_  = pPtr.pCounter_;
        pWCounter_ = pPtr.pWCounter_;

        pPtr.pValue_    = nullptr;
        pPtr.pCounter_  = nullptr;
        pPtr.pWCounter_ = nullptr;
    }

    /// Conversion from nullptr.
    wptr(const nullptr_t&)
    {
        pValue_    = nullptr;
        pCounter_  = nullptr;
        pWCounter_ = nullptr;
    }

    /// Destructor.
    /** \note This function will <b>never</b> delete the pointed object (if any).
    */
    ~wptr()
    {
        decrement_();
    }

    template<class N>
    /// refptr conversion.
    wptr(const refptr<N>& pRefPtr)
    {
        pValue_    = pRefPtr.pValue_;
        pCounter_  = pRefPtr.pCounter_;
        pWCounter_ = pRefPtr.pWCounter_;

        increment_();
    }

    template<class N>
    /// wptr conversion.
    wptr(const wptr<N>& pPtr)
    {
        pValue_    = pPtr.pValue_;
        pCounter_  = pPtr.pCounter_;
        pWCounter_ = pPtr.pWCounter_;

        increment_();
    }

    template<class N>
    /// wptr conversion move.
    wptr(wptr<N>&& pPtr)
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
    bool is_valid() const
    {
        return (pCounter_ && *pCounter_ != 0u);
    }

    /// Returns the number of refptr pointing to the object.
    /** \return The number of refptr pointing to the object
    *   \note If this function returns 0, then the object has
    *         been deleted and this pointer is invalid, or it
    *         is simply NULL.
    */
    uint get_count() const
    {
        if (pCounter_)
            return *pCounter_;
        else
            return 0u;
    }

    /// Returns the number of wptr pointing to the object.
    /** \return The number of wptr pointing to the object
    *   \note This function returns 0 if the pointer is NULL.
    */
    uint get_weak_count() const
    {
        if (pWCounter_)
            return *pWCounter_;
        else
            return 0u;
    }

    /// Sets this pointer to NULL.
    /** \note This function will <b>never</b> delete the pointed object (if any).
    */
    void set_null()
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
    refptr<const T> lock() const
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
    refptr<T> lock()
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
    const T* operator -> () const
    {
        return pValue_;
    }

    /// Dereferences the pointer.
    /** \return The contained pointer
    *   \note You should test for the pointer's validity before using
    *         this operator, unless you're sure the object is still
    *         alive.
    */
    T* operator -> ()
    {
        return pValue_;
    }

    /// Returns a reference to the contained value.
    /** \return A reference to the contained value
    */
    const T& operator * () const
    {
        return *pValue_;
    }

    /// Returns a reference to the contained value.
    /** \return A reference to the contained value
    */
    T& operator * ()
    {
        return *pValue_;
    }

    /// nullptr assignation operator.
    /** \param pPtr The value to copy
    */
    wptr& operator = (const nullptr_t& pPtr)
    {
        decrement_();

        pValue_ = nullptr;

        return *this;
    }

    /// Copy operator.
    /** \param pPtr The value to copy
    */
    wptr& operator = (const wptr& pPtr)
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
    wptr& operator = (const wptr<N>& pPtr)
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
    wptr& operator = (wptr&& pPtr)
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
    wptr& operator = (wptr<N>&& pPtr)
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
    bool operator == (const wptr<N>& pValue) const
    {
        return (pValue_ == pValue.pValue_);
    }

    /// Checks if this pointer equals another
    /** \param pValue The pointer to test
    */
    template<class N>
    bool operator == (const refptr<N>& pValue) const
    {
        return (pValue_ == pValue.pValue_);
    }

    /// Checks if this pointer equals another
    /** \param pValue The pointer to test
    */
    template<class N>
    bool operator == (N* pValue) const
    {
        return (pValue_ == pValue);
    }

    /// Checks if this pointer is null
    /** \param pValue The null pointer
    *   \return 'true' if this pointer is null
    */
    bool operator == (nullptr_t pValue) const
    {
        return (pValue_ == 0);
    }

    /// Checks if this pointer is different from another
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer is different from another
    */
    template<class N>
    bool operator != (const wptr<N>& pValue) const
    {
        return (pValue_ != pValue.pValue_);
    }

    /// Checks if this pointer is different from another
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer is different from another
    */
    template<class N>
    bool operator != (const refptr<N>& pValue) const
    {
        return (pValue_ != pValue.pValue_);
    }

    /// Checks if this pointer is different from another
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer is different from another
    */
    template<class N>
    bool operator != (N* pValue) const
    {
        return (pValue_ != pValue);
    }

    /// Checks if this pointer is not null
    /** \param pValue The null pointer
    *   \return 'true' if this pointer is not null
    */
    bool operator != (nullptr_t pValue) const
    {
        return (pValue_ != 0);
    }

    /// Checks if this pointer's value is lower than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is lower than the other
    */
    template<class N>
    bool operator < (const wptr<N>& pValue) const
    {
        return (pValue_ < pValue.pValue_);
    }

    /// Checks if this pointer's value is lower than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is lower than the other
    */
    template<class N>
    bool operator < (const refptr<N>& pValue) const
    {
        return (pValue_ < pValue.pValue_);
    }

    /// Checks if this pointer's value is lower than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is lower than the other
    */
    template<class N>
    bool operator < (N* pValue) const
    {
        return (pValue_ < pValue);
    }

    /// Checks if this pointer's value is lower or equal than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is lower or equal than the other
    */
    template<class N>
    bool operator <= (const wptr<N>& pValue) const
    {
        return (pValue_ <= pValue.pValue_);
    }

    /// Checks if this pointer's value is lower or equal than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is lower or equal than the other
    */
    template<class N>
    bool operator <= (const refptr<N>& pValue) const
    {
        return (pValue_ <= pValue.pValue_);
    }

    /// Checks if this pointer's value is lower or equal than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is lower or equal than the other
    */
    template<class N>
    bool operator <= (N* pValue) const
    {
        return (pValue_ <= pValue);
    }

    /// Checks if this pointer's value is greater than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is greater than the other
    */
    template<class N>
    bool operator > (const wptr<N>& pValue) const
    {
        return (pValue_ > pValue.pValue_);
    }

    /// Checks if this pointer's value is greater than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is greater than the other
    */
    template<class N>
    bool operator > (const refptr<N>& pValue) const
    {
        return (pValue_ > pValue.pValue_);
    }

    /// Checks if this pointer's value is greater than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is greater than the other
    */
    template<class N>
    bool operator > (N* pValue) const
    {
        return (pValue_ > pValue);
    }

    /// Checks if this pointer's value is greater or equal than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is greater or equal than the other
    */
    template<class N>
    bool operator >= (const wptr<N>& pValue) const
    {
        return (pValue_ >= pValue.pValue_);
    }

    /// Checks if this pointer's value is greater or equal than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is greater or equal than the other
    */
    template<class N>
    bool operator >= (const refptr<N>& pValue) const
    {
        return (pValue_ >= pValue.pValue_);
    }

    /// Checks if this pointer's value is greater or equal than the other
    /** \param pValue The pointer to test
    *   \return 'true' if this pointer's value is greater or equal than the other
    */
    template<class N>
    bool operator >= (N* pValue) const
    {
        return (pValue_ >= pValue);
    }

    /// Allows : "if (!pPointer)".
    bool operator ! () const
    {
        return (pCounter_ == 0 ? true : *pCounter_ == 0u);
    }

    /// Allows : "if (pPointer)".
    explicit operator bool() const
    {
        return (pCounter_ && *pCounter_ != 0u);
    }

    /// Casts the provided pointer to this one's type.
    /** \param pValue The pointer to cast
    *   \return The new casted pointer
    */
    template<class N>
    static wptr<T> cast(const wptr<N>& pValue)
    {
        return wptr<T>(static_cast<T*>(pValue.pValue_), pValue.pCounter_, pValue.pWCounter_);
    }

    /// Tries to dynamic cast the provided pointer to this one's type.
    /** \param pValue The pointer to cast
    *   \return The new casted pointer
    *   \note Dynamic cast can fail, and in this case, will result in
    *         a NULL pointer.
    */
    template<class N>
    static wptr<T> dyn_cast(const wptr<N>& pValue)
    {
        T* pTemp = dynamic_cast<T*>(pValue.pValue_);
        if (pTemp)
            return wptr<T>(pTemp, pValue.pCounter_, pValue.pWCounter_);
        else
            return wptr<T>();
    }

private :

    wptr(T* pValue, uint* pCounter, uint* pWCounter) :
        pValue_(pValue), pCounter_(pCounter), pWCounter_(pWCounter)
    {
    }

    void increment_()
    {
        if (pWCounter_)
        {
            ++(*pWCounter_);
        }
    }

    void decrement_()
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

    T*    pValue_;
    uint* pCounter_;
    uint* pWCounter_;
};
}


#endif
