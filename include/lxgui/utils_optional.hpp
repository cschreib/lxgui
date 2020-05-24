#ifndef OPTIONAL_HPP
#define OPTIONAL_HPP

#include <utility>
#include "utils_exception.hpp"

namespace lxgui {
namespace utils
{
    constexpr struct nullopt_t {} nullopt;
    constexpr struct in_place_t {} in_place;

    /// Implementation of std::optional from C++17
    template<typename T>
    class optional
    {
    public:

        /// Default constructor.
        /** Constructs an empty optional.
        */
        optional() = default;

        /// Generic constructor.
        /** Constructs a new object from the provided arguments.
        */
        template<typename ... Args>
        explicit optional(Args&& ... args)
        {
            allocate_(std::forward<Args>(args)...);
        }

        /// Generic constructor.
        /** Constructs a new object from the provided arguments.
            This version exists to enable creating a default-constructed
            object, among otherthings.
        */
        template<typename ... Args>
        explicit optional(in_place_t, Args&& ... args)
        {
            allocate_(std::forward<Args>(args)...);
        }

        /// Copy constructor.
        /** The object contained in mOpt, if any, will be duplicated.
        */
        optional(const optional& mOpt) : bValid_(mOpt.bValid_)
        {
            if (bValid_)
            {
                allocate_(mOpt.unchecked_value_());
            }
        }

        /// Move constructor.
        /** The object contained in mOpt, if any, will be moved. mOpt becomes empty.
        */
        optional(optional&& mOpt) : bValid_(mOpt.bValid_)
        {
            if (bValid_)
            {
                allocate_(mOpt.unchecked_value_());
            }
        }

        /// Null constructor.
        /** Constructs an empty optional.
        */
        optional(nullopt_t) noexcept {}

        /// Destructor.
        /** Any object contained is destroyed.
        */
        ~optional() noexcept
        {
            reset();
        }

        /// Copy assignment operator.
        /** The object contained in mOpt, if any, will be duplicated.
            Any object contained in this optional will be destroyed.
        */
        optional& operator=(const optional& mOpt)
        {
            reset();
            bValid_ = mOpt.bValid_;
            if (bValid_)
            {
                allocate_(mOpt.unchecked_value_());
            }
            return *this;
        }

        /// Copy assignment operator.
        /** The object contained in mOpt, if any, will be moved. mOpt becomes empty.
            Any object contained in this optional will be destroyed.
        */
        optional& operator=(optional&& mOpt)
        {
            reset();
            bValid_ = mOpt.bValid_;
            if (bValid_)
            {
                allocate_(mOpt.unchecked_value_());
            }
            return *this;
        }

        /// Null assignment operator.
        /** Any object contained in this optional will be destroyed. This
            optional becomes empty.
        */
        optional& operator=(nullopt_t) noexcept
        {
            reset();
            return *this;
        }

        /// Obtain the contained object.
        /** Will return the contained object, or throw an exception if
            the optional is empty. For unchecked access, use operator*.
        */
        T& value()
        {
            check_valid_();
            return unchecked_value_();
        }

        /// Obtain the contained object.
        /** Will return the contained object, or throw an exception if
            the optional is empty. For unchecked access, use operator*.
        */
        const T& value() const
        {
            check_valid_();
            return unchecked_value_();
        }

        /// Construct a new object in this optional.
        /** A new object is created from the provided arguments.
            Any object contained in this optional will be destroyed.
        */
        template<typename ... Args>
        T& emplace(Args&& ... args)
        {
            reset();
            allocate_(std::forward<Args>(args)...);
            return unchecked_value_();
        }

        /// Destroy the contained object, if any.
        /** Any object contained in this optional will be destroyed.
        */
        void reset() noexcept
        {
            if (bValid_)
            {
                unchecked_value_().T::~T();
            }

            bValid_ = false;
        }

        /// Check if this optional contains an object.
        bool has_value() const
        {
            return bValid_;
        }

        /// Check if this optional contains an object.
        explicit operator bool() const
        {
            return bValid_;
        }

        /// Check if this optional does not contain an object.
        bool operator!() const
        {
            return !bValid_;
        }

        /// Obtain the contained object.
        /** No check is performed to ensure an object exists.
            Accessing an empty optional this way is undefined behavior.
        */
        T& operator *()
        {
            return unchecked_value_();
        }

        /// Obtain the contained object.
        /** No check is performed to ensure an object exists.
            Accessing an empty optional this way is undefined behavior.
        */
        const T& operator *() const
        {
            return unchecked_value_();
        }

        /// Obtain the contained object.
        /** No check is performed to ensure an object exists.
            Accessing an empty optional this way is undefined behavior.
        */
        T* operator ->()
        {
            return &unchecked_value_();
        }

        /// Obtain the contained object.
        /** No check is performed to ensure an object exists.
            Accessing an empty optional this way is undefined behavior.
        */
        const T* operator ->() const
        {
            return &unchecked_value_();
        }

    private:

        template<typename ... Args>
        void allocate_(Args&& ... args)
        {
            new (lBuffer_) T(std::forward<Args>(args)...);
            bValid_ = true;
        }

        void check_valid_() const
        {
            if (!bValid_)
                throw utils::exception("optional", "access to invalid object");
        }

        T& unchecked_value_()
        {
            return *reinterpret_cast<T*>(lBuffer_);
        }

        const T& unchecked_value_() const
        {
            return *reinterpret_cast<const T*>(lBuffer_);
        }

        char lBuffer_[sizeof(T)];
        bool bValid_ = false;
    };

    /// Create a new optional containing an object.
    /** The new object is constructed in the optional with the
        provided arguments.
    */
    template<typename T, typename ... Args>
    optional<T> make_optional(Args&& ... args)
    {
        return optional<T>(in_place, std::forward<Args>(args)...);
    }
}
}

#endif
