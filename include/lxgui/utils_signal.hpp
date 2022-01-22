#ifndef LXGUI_UTILS_SIGNAL_HPP
#define LXGUI_UTILS_SIGNAL_HPP

#include <lxgui/utils.hpp>
#include <lxgui/utils_view.hpp>
#include <lxgui/utils_observer.hpp>

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

        // Copiable, movable.
        scoped_connection(const scoped_connection&) = default;
        scoped_connection(scoped_connection&&) = default;
        scoped_connection& operator=(const scoped_connection&) = default;
        scoped_connection& operator=(scoped_connection&&) = default;

        /// Disconnect the slot.
        using connection::disconnect;

        /// Check if this slot is still connected.
        using connection::connected;
    };

    /// Generic class for observing and triggering events.
    template<typename T>
    class signal
    {
    public:
        /// Type of the callable function stored in a slot.
        using function_type = std::function<T>;

    private:
        struct slot : signal_impl::slot_base
        {
            explicit slot(function_type func) noexcept : callback(std::move(func)) {}

            function_type callback;
        };

        /// Type of the slot list (internal).
        /** \note Constraints on the choice container type:
        *          - must allow back insertion
        *          - back insertion must not invalidate iterators, or allow random access
        *          - must allow forward iteration
        *          - iterators can be invalidated on removal
        *          - most common use is iteration, not addition or removal
        *          - ordering of elements is relevant
        */
        template<typename U>
        using container_type = std::vector<U>;
        using slot_list = container_type<utils::owner_ptr<slot>>;

        template<typename BaseIterator>
        struct slot_dereferencer
        {
            using data_type = const function_type&;
            static data_type dereference(const BaseIterator& iter) noexcept
            {
                return (*iter)->callback;
            }
        };

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
        using script_list_view = utils::view::adaptor<slot_list,
            slot_dereferencer,
            non_disconnected_filter>;

        /// Default constructor (no slot).
        signal() : slots_(std::make_shared<slot_list>())
        {
        }

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
            if (recursion_ == 0u)
            {
                slots_->clear();
                return;
            }

            // We are in the middle of an iteration; just mark slot as
            // disconnected. The slots will not be called, however they still
            // occupy a spot in the slot list. This is done to avoid invalidating
            // iterators while iterating over the slot list, if one of the
            // slot happens to disconnect itself. The memory will be cleared
            // up when the signal is no longer being iterated on.
            for (auto& slot : *slots_)
                slot->disconnected = true;
        }

        /// Check if this signal contains any slot.
        /** \return 'true' if at least one slot is connected.
        */
        [[nodiscard]] bool empty() const noexcept
        {
            for (const auto& slot : *slots_)
            {
                if (!slot->disconnected)
                    return false;
            }

            return true;
        }

        /// Return a view onto the connected slots.
        /** \return A view onto the connected slots
        */
        [[nodiscard]] script_list_view slots() const noexcept
        {
            return script_list_view(*slots_);
        }

        /// Connect a new slot to this signal.
        /** \param mFunction The function to store in the slot.
        */
        connection connect(function_type mFunction)
        {
            slots_->push_back(utils::make_owned<slot>(std::move(mFunction)));
            return connection(slots_->back());
        }

        /// Trigger the signal.
        /** \param mArgs Arguments to forward to all the connected slots.
        */
        template<typename ... Args>
        void operator()(Args&& ... mArgs)
        {
            // Make a shared-ownership copy of the slot list, so that the list
            // survives even if this signal is destroyed midway during a slot.
            const auto slots_copy = slots_;

            ++recursion_;

            // Call the slots
            const auto& lSlotsCopy = *slots_copy;
            // NB: Cache the size here, so new slots connected will not trigger
            // NB: Use integer-based iteration since iterators may be invalidated on insertion
            const std::size_t uiNumSlots = lSlotsCopy.size();
            for (std::size_t i = 0; i < uiNumSlots; ++i)
            {
                if (!lSlotsCopy[i]->disconnected)
                    lSlotsCopy[i]->callback(mArgs...);
            }

            --recursion_;

            if (recursion_ == 0u)
                garbage_collect_();
        }

    private:
        void garbage_collect_() noexcept
        {
            auto iter = std::remove_if(slots_->begin(), slots_->end(),
                [](const auto& slot) { return slot->disconnected; });

            slots_->erase(iter, slots_->end());
        }

        std::shared_ptr<slot_list> slots_;
        std::size_t                recursion_ = 0u;
    };
}
}

#endif
