#ifndef LXGUI_UTILS_SIGNAL_HPP
#define LXGUI_UTILS_SIGNAL_HPP

#include "lxgui/utils.hpp"
#include "lxgui/utils_view.hpp"
#include "lxgui/utils_observer.hpp"

#include <functional>
#include <vector>
#include <memory>
#include <algorithm>

namespace lxgui {
namespace utils
{
    /** \cond INCLUDE_INTERNALS_IN_DOC
    */
    namespace signal_impl
    {
        struct slot_base
        {
            bool disconnected = false;
        };
    }
    /** \endcond
    */

    /// Object representing the connection between a slot and a signal.
    class connection
    {
        template<typename T>
        friend class signal;

        explicit connection(utils::observer_ptr<signal_impl::slot_base> slot) noexcept :
            slot_(std::move(slot)) {}

    public:
        /// Default constructor, no connection.
        connection() = default;

        // Copiable, movable.
        connection(const connection&) = default;
        connection(connection&&) = default;
        connection& operator=(const connection&) = default;
        connection& operator=(connection&&) = default;

        /// Disconnect the slot.
        void disconnect() noexcept
        {
            if (auto* raw = slot_.get())
            {
                raw->disconnected = true;
                slot_ = nullptr;
            }
        }

        /// Check if this slot is still connected.
        [[nodiscard]] bool connected() const noexcept
        {
            if (auto* raw = slot_.get())
                return !raw->disconnected;
            else
                return false;
        }

    private:
        utils::observer_ptr<signal_impl::slot_base> slot_;
    };

    /// A @ref connection that automatically disconnects when going out of scope.
    class scoped_connection : private connection
    {
    public:
        /// Default constructor, no connection.
        scoped_connection() = default;

        /// Conversion constructor from a raw connection.
        scoped_connection(connection mConnection) noexcept : connection(std::move(mConnection)) {}

        /// Destructor, disconnects.
        ~scoped_connection() noexcept
        {
            disconnect();
        }

        // Non-copiable, movable.
        scoped_connection(const scoped_connection&) = delete;
        scoped_connection(scoped_connection&&) = default;
        scoped_connection& operator=(const scoped_connection&) = delete;
        scoped_connection& operator=(scoped_connection&&) = default;

        /// Disconnect the slot.
        using connection::disconnect;

        /// Check if this slot is still connected.
        using connection::connected;
    };

    /// Generic class for observing and triggering events.
    /** The implementation guarantees that the following is safe:
    *    - Connecting or disconnecting a slot from inside any other slot (including self).
    *    - Destroying the signal from any slot.
    *    - Calling the signal recursively from any slot (up to standard stack exhaustion limits).
    *
    *   Other notable behaviors:
    *    - Slots will be called in order of connection.
    *    - Slots connected while the signal is being triggered will not be called until the
    *      next trigger of the signal.
    *
    *   Example:
    *
    *   \code{.cpp}
    *   // Create a signal. This can be called at any time,
    *   // and will just forward its argument to all registered slots.
    *   utils::signal<void(int)> sig;
    *
    *   // By default, no slot is registered, so calling does nothing.
    *   sig(42); // nothing happens
    *
    *   // Register one slot that prints the value.
    *   sig.connect([](int i) { std::cout << i << std::endl; });
    *   sig(42); // prints 42
    *
    *   // We can register more than one slot; here we print a modified value.
    *   sig.connect([](int i) { std::cout << 2*i << std::endl; });
    *   sig(42); // prints 42 then 84
    *
    *   // We can use a connection to control when a slot is no longer called.
    *   utils::connection c = sig.connect([](int i) { std::cout << 3*i << std::endl; });
    *   sig(42); // prints 42, then 84, then 126
    *
    *   // Disconnect the last slot.
    *   c.disconnect();
    *   sig(42); // prints 42, then 84
    *
    *   // The safest way to do this is to use a scoped connection
    *   int val = 0;
    *   {
    *       utils::scoped_connection sc = sig.connect([](int i) { val = i; });
    *       sig(42); // prints 42, then 84, and stores 42 in val
    *   }
    *
    *   // The last slot is automatically disconnected when out of scope.
    *   sig(43); // prints 43, then 86; val is not changed
    *   \endcode
    */
    template<typename T>
    class signal
    {
    public:
        /// Type of the callable function stored in a slot.
        /** Can use any function/delegate type here, as long as it
        *   has a matching call operator. The function type does not
        *   need to be owning; in this case, the slot must be manually
        *   disconnected whenever the pointed function is destroyed.
        */
        using function_type = std::function<T>;

    private:
        /// Type of a slot.
        /** This inherits from slot_base, which allows a slot to be disconnected
        *   using a simple boolean flag. This enable safe disconnection at any time.
        */
        struct slot : signal_impl::slot_base
        {
            explicit slot(function_type func) noexcept : callback(std::move(func)) {}

            function_type callback;
        };

        /// Type of the slot list (internal).
        /** Constraints on the choice container type:
        *    1. must allow back insertion
        *    2. must allow forward iteration
        *    3. ordering of elements is relevant
        *    4. most common use is iteration, not addition or removal
        *    5. back insertion must not invalidate iterators, OR random access is allowed
        *    6. references to elements may be invalidated on insertion
        *    7. iterators may be invalidated on removal
        *
        *   Criteria 1 to 5 are fundamental to the API. The others are linked to the
        *   choice of implementation, and could be changed.
        *
        *   There are multiple standard containers that satisfy these criteria:
        *    - std::vector
        *    - std::deque
        *    - std::list
        *    - std::forward_list
        *
        *   std::vector is the simplest and most efficient, hence is chosen here.
        */
        template<typename U>
        using container_type = std::vector<U>;

        /// Type of the slot list.
        /** Slots are stored as observable shared pointers, so that connection objects
        *   can be implemented as a simple observer pointer, without risk of accessing
        *   an already deleted slot. If manual disconnection is not required, this could
        *   be changed to store slots by value directly for better performance.
        */
        using slot_list = container_type<utils::owner_ptr<slot>>;

        /// De-reference an iterator from the slot list to get to the function.
        template<typename BaseIterator>
        struct slot_dereferencer
        {
            using data_type = const function_type&;
            static data_type dereference(const BaseIterator& iter) noexcept
            {
                return (*iter)->callback;
            }
        };

        /// Filter out the disconnected slots from the slot list.
        template<typename BaseIterator>
        struct non_disconnected_filter
        {
            static bool is_included(const BaseIterator& iter) noexcept
            {
                return !(*iter)->disconnected;
            }
        };

    public:

        /// Type of the view returned by slots().
        using slot_list_view = utils::view::adaptor<slot_list,
            slot_dereferencer,
            non_disconnected_filter>;

        /// Default constructor (no slot).
        signal() : impl_(std::make_shared<impl>())
        {
        }

        /// Destructor.
        ~signal()
        {
            // Mark all slots as disconnected, in case the destructor
            // is called midway through the signal being emitted.
            disconnect_all();
        }

        // Non-copiable, movable.
        signal(const signal&) = delete;
        signal(signal&&) = default;
        signal& operator=(const signal&) = delete;
        signal& operator=(signal&&) = default;

        /// Disconnects all slots.
        void disconnect_all() noexcept
        {
            if (impl_->recursion == 0u)
            {
                impl_->slots.clear();
                return;
            }

            // We are in the middle of an iteration; just mark slot as
            // disconnected. The slots will not be called, however they still
            // occupy a spot in the slot list. This is done to avoid invalidating
            // iterators while iterating over the slot list, if one of the
            // slot happens to disconnect itself. The memory will be cleared
            // up when the signal is no longer being iterated on.
            for (auto& slot : impl_->slots)
                slot->disconnected = true;
        }

        /// Check if this signal contains any slot.
        /** \return 'true' if at least one slot is connected.
        */
        [[nodiscard]] bool empty() const noexcept
        {
            const auto& slots = impl_->slots;
            return std::none_of(slots.begin(), slots.end(),
                [](const auto& slot) { return !slot->disconnected; });
        }

        /// Return a constant view onto the connected slots.
        /** \return A constant view onto the connected slots
        *   \warning Do not attempt to connect or disconnect slots while iterating over this view.
        */
        [[nodiscard]] slot_list_view slots() const noexcept
        {
            return slot_list_view(impl_->slots);
        }

        /// Connect a new slot to this signal.
        /** \param mFunction The function to store in the slot.
        *   \return A connection object, which can be used to disconnect the slot at any time.
        *   \note If the returned connection object is discarded, the slot will remain connected
        *         for the entire lifetime of the signal, or until @ref disconnect_all is called.
        *         If this is not desirable, store the returned connection, and disconnect the slot
        *         when it should no longer be called. The RAII helper class @ref scoped_connection
        *         can do this safely.
        */
        connection connect(function_type mFunction)
        {
            impl_->slots.push_back(utils::make_owned<slot>(std::move(mFunction)));
            return connection(impl_->slots.back());
        }

        /// Trigger the signal.
        /** \param args Arguments to forward to all the connected slots.
        */
        template<typename ... Args>
        void operator()(Args&& ... args)
        {
            // Make a shared-ownership copy of the slot list, so that the list
            // survives even if this signal is destroyed midway during a slot.
            // To support this, we must not use any member variable or member
            // function of the signal object, which may be destroyed at any time.
            [[maybe_unused]] const auto impl_copy_ptr = impl_;
            auto& impl = *impl_;
            const auto& slots = impl.slots;
            auto& recursion = impl.recursion;

            ++recursion;

            // Call the slots
            // NB: Cache the size here, so new slots connected will not trigger
            // NB: Use integer-based iteration since iterators may be invalidated on insertion
            const std::size_t num_slots = slots.size();
            for (std::size_t i = 0; i < num_slots; ++i)
            {
                auto& slot = *slots[i];
                if (!slot.disconnected)
                    slot.callback(args...);
            }

            --recursion;

            if (recursion == 0u)
                impl.garbage_collect();
        }

    private:
        struct impl {
            slot_list   slots;
            std::size_t recursion = 0u;

            void garbage_collect() noexcept
            {
                auto iter = std::remove_if(slots.begin(), slots.end(),
                    [](const auto& slot) { return slot->disconnected; });

                slots.erase(iter, slots.end());
            }
        };

        std::shared_ptr<impl> impl_;
    };
}
}

#endif
