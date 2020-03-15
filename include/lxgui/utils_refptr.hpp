#ifndef UTILS_REFPTR_HPP
#define UTILS_REFPTR_HPP

#include "lxgui/utils.hpp"

typedef decltype(nullptr) nullptr_t;

namespace utils
{
template<class> class wptr;

/// Reference counting pointer
/** This is a pointer that automatically deletes
*   its content when not used anymore.<br>
*   Can be used in conjunction with wptr.
*   Uses reference counting.
*/
template<class T>
class refptr
{
public :

    template<class> friend class refptr;
    template<class> friend class wptr;

    /// Default constructor.
    /** \note Initializes the pointer to NULL.
    */
    refptr()
    {
        pValue_    = nullptr;
        pCounter_  = nullptr;
        pWCounter_ = nullptr;
    }

    /// Copy constructor.
    /** \param mValue the refptr to copy
    */
    refptr(const refptr& mValue)
    {
        pValue_    = mValue.pValue_;
        pCounter_  = mValue.pCounter_;
        pWCounter_ = mValue.pWCounter_;

        increment_();
    }

    /// Move constructor.
    /** \param mValue the refptr to copy
    */
    refptr(refptr&& mValue)
    {
        pValue_    = mValue.pValue_;
        pCounter_  = mValue.pCounter_;
        pWCounter_ = mValue.pWCounter_;

        mValue.pValue_    = nullptr;
        mValue.pCounter_  = nullptr;
        mValue.pWCounter_ = nullptr;
    }

    /// Conversion from nullptr.
    refptr(const nullptr_t&)
    {
        pValue_    = nullptr;
        pCounter_  = nullptr;
        pWCounter_ = nullptr;
    }

    /// Constructor.
    /** \param pValue The pointer to assign
    *   \note This is the only way to assign a classic
    *         pointer to a refptr.
    */
    explicit refptr(T* pValue)
    {
        pValue_    = pValue;
        pCounter_  = new uint(0);
        pWCounter_ = new uint(0);

        increment_();
    }

    template<class N>
    /// Conversion constructor.
    /** \param mValue the refptr to copy
    */
    explicit refptr(const refptr<N>& mValue)
    {
        pValue_    = mValue.pValue_;
        pCounter_  = mValue.pCounter_;
        pWCounter_ = mValue.pWCounter_;

        increment_();
    }

    template<class N>
    /// Conversion move constructor.
    /** \param mValue the refptr to move
    */
    explicit refptr(refptr<N>&& mValue)
    {
        pValue_    = mValue.pValue_;
        pCounter_  = mValue.pCounter_;
        pWCounter_ = mValue.pWCounter_;

        mValue.pValue_    = nullptr;
        mValue.pCounter_  = nullptr;
        mValue.pWCounter_ = nullptr;
    }

    /// Destructor.
    /** \note Can cause deletion of the contained
    *         pointer.
    */
    ~refptr()
    {
        decrement_();
    }

    /// Returns the contained pointer.
    /** \return The contained pointer
    */
    const T* get() const
    {
        return pValue_;
    }

    /// Returns the contained pointer.
    /** \return The contained pointer
    */
    T* get()
    {
        return pValue_;
    }

    /// Checks if this pointer is usable.
    /** \return 'true' is this pointer is usable
    */
    bool is_valid() const
    {
        return (pValue_ != nullptr);
    }

    /// Returns the number of refptr pointing to the object.
    /** \return The number of refptr pointing to the object
    *   \note This function returns 0 if the pointer is NULL.
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
    /** \note Can cause deletion of the contained
    *         pointer.
    */
    void set_null()
    {
        decrement_();

        pValue_    = nullptr;
        pCounter_  = nullptr;
        pWCounter_ = nullptr;
    }

    /// Creates a wptr pointing at the same object.
    wptr<T> create_weak() const
    {
        return wptr<T>(*this);
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

    /// Dereferences the pointer.
    /** \return The contained pointer
    */
    const T* operator -> () const
    {
        return pValue_;
    }

    /// Dereferences the pointer.
    /** \return The contained pointer
    */
    T* operator -> ()
    {
        return pValue_;
    }

    /// Copy operator.
    /** \param mValue The value to copy
    *   \note Can cause deletion of the contained
    *         pointer.
    */
    refptr& operator = (const refptr& mValue)
    {
        if (&mValue != this)
        {
            decrement_();

            pValue_    = mValue.pValue_;
            pCounter_  = mValue.pCounter_;
            pWCounter_ = mValue.pWCounter_;

            increment_();
        }

        return *this;
    }

    template<class N>
    /// Copy operator.
    /** \param mValue The value to copy
    *   \note Can cause deletion of the contained
    *         pointer.
    */
    refptr& operator = (const refptr<N>& mValue)
    {
        if (mValue.pValue_ != pValue_)
        {
            decrement_();

            pValue_    = mValue.pValue_;
            pCounter_  = mValue.pCounter_;
            pWCounter_ = mValue.pWCounter_;

            increment_();
        }

        return *this;
    }

    /// Move operator.
    /** \param mValue The value to move
    *   \note Can cause deletion of the contained
    *         pointer.
    */
    refptr& operator = (refptr&& mValue)
    {
        if (&mValue != this)
        {
            decrement_();

            pValue_    = mValue.pValue_;
            pCounter_  = mValue.pCounter_;
            pWCounter_ = mValue.pWCounter_;

            mValue.pValue_    = nullptr;
            mValue.pCounter_  = nullptr;
            mValue.pWCounter_ = nullptr;
        }

        return *this;
    }

    template<class N>
    /// Move operator.
    /** \param mValue The value to move
    *   \note Can cause deletion of the contained
    *         pointer.
    */
    refptr& operator = (refptr<N>&& mValue)
    {
        if (mValue.pValue_ != pValue_)
        {
            decrement_();

            pValue_    = mValue.pValue_;
            pCounter_  = mValue.pCounter_;
            pWCounter_ = mValue.pWCounter_;

            mValue.pValue_    = nullptr;
            mValue.pCounter_  = nullptr;
            mValue.pWCounter_ = nullptr;
        }

        return *this;
    }

    template<class N>
    /// Checks if this pointer equals another
    /** \param mValue The pointer to test
    */
    bool operator == (const refptr<N>& mValue)
    {
        return (pValue_ == mValue.pValue_);
    }

    /// Checks if this pointer equals another
    /** \param pValue The pointer to test
    */
    bool operator == (T* pValue) const
    {
        return (pValue_ == pValue);
    }

    /// Checks if this pointer is null
    /** \param pValue The null pointer
    *   \return 'true' if this pointer is null
    */
    bool operator == (nullptr_t pValue) const
    {
        return (pValue_ == nullptr);
    }

    template<class N>
    /// Checks if this pointer is different from another
    /** \param mValue The pointer to test
    */
    bool operator != (const refptr<N>& mValue)
    {
        return (pValue_ != mValue.pValue_);
    }

    /// Checks if this pointer is different from another
    /** \param pValue The pointer to test
    */
    bool operator != (T* pValue) const
    {
        return (pValue_ != pValue);
    }

    /// Checks if this pointer is not null
    /** \param pValue The null pointer
    *   \return 'true' if this pointer is not null
    */
    bool operator != (nullptr_t pValue) const
    {
        return (pValue_ != nullptr);
    }

    /// Allows : "if (!pPointer)".
    bool operator ! () const
    {
        return (pValue_ == nullptr);
    }

    /// Allows : "if (pPointer)".
    #ifndef NO_CPP11_EXPLICIT_CONV
    explicit operator bool() const
    {
        return (pValue_ != nullptr);
    }
    #else
    typedef void (refptr::*memfunptr)();
    operator memfunptr() const
    {
        if (pValue_ != nullptr)
            return &refptr::set_null;
        else
            return nullptr;
    }
    #endif

    /// Allows limited implicit inheritance conversion.
    template<class N>
    operator refptr<const N>() const
    {
        return refptr<N>(pValue_, pCounter_, pWCounter_);
    }

    /// Allows limited implicit inheritance conversion.
    template<class N>
    operator refptr<N>()
    {
        return refptr<N>(pValue_, pCounter_, pWCounter_);
    }

    /// Casts the provided pointer to this one's type.
    /** \param pValue The pointer to cast
    *   \return The new casted pointer
    */
    template<class N>
    static refptr<T> cast(const refptr<N>& pValue)
    {
        return refptr<T>(static_cast<T*>(pValue.pValue_), pValue.pCounter_, pValue.pWCounter_);
    }

    /// Tries to dynamic cast the provided pointer to this one's type.
    /** \param pValue The pointer to cast
    *   \return The new casted pointer
    *   \note Dynamic cast can fail, and in this case, will result in
    *         a NULL pointer.
    */
    template<class N>
    static refptr<T> dyn_cast(const refptr<N>& pValue)
    {
        T* pTemp = dynamic_cast<T*>(pValue.pValue_);
        if (pTemp)
            return refptr<T>(pTemp, pValue.pCounter_, pValue.pWCounter_);
        else
            return refptr<T>();
    }

protected :

    /// Constructor.
    /** \param pValue   The pointer to assign
    *   \param pCounter The counter to use
    *   \param pWCounter The weak ptr counter to use
    *   \note This function should not be called
    *         by anyone else than itself (and other
    *         template specializations).
    */
    explicit refptr(T* pValue, uint* pCounter, uint* pWCounter)
    {
        pValue_    = pValue;
        pCounter_  = pCounter;
        pWCounter_ = pWCounter;

        increment_();
    }

private :

    void increment_()
    {
        if (pCounter_)
            ++(*pCounter_);
    }

    void decrement_()
    {
        if (pCounter_)
        {
            --(*pCounter_);
            if (*pCounter_ == 0u)
            {
                if (*pWCounter_ == 0u)
                {
                    delete pCounter_;
                    pCounter_ = nullptr;
                    delete pWCounter_;
                    pWCounter_ = nullptr;
                }

                delete pValue_;
                pValue_ = nullptr;
            }
        }
    }

    T*    pValue_;
    uint* pCounter_;
    uint* pWCounter_;
};
}

#endif
