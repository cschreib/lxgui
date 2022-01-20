#ifndef LXGUI_UTILS_SIGNAL_HPP
#define LXGUI_UTILS_SIGNAL_HPP

#include <lxgui/utils.hpp>
#include <lxgui/utils_view.hpp>

#include <list>
#include <functional>
#include <vector>
#include <memory>
#include <algorithm>

namespace lxgui {
namespace utils
{
    /// Generic class for observing and triggering events.
    template<typename T>
    class signal
    {
    public:
        /// Type of the callable function stored in a slot.
        using function_type = std::function<T>;

    private:
        struct slot
        {
            function_type mCallback;
            bool          bDisconnected = false;
        };

        /// Type of the slot list (internal).
        /** \note Constraints on the choice container type:
        *          - must allow back insertion without invalidating iterators
        *          - must allow forward iteration
        *          - iterators can be invalidated on removal
        *          - most common use is iteration, not addition or removal
        *          - ordering of elements is relevant
        */
        template<typename U>
        using container_type = std::list<U>;
        using slot_list = container_type<slot>;

        template<typename BaseIterator>
        struct slot_dereferencer
        {
            using data_type = const function_type&;
            static data_type dereference(const BaseIterator& mIter) { return mIter->mCallback; }
        };

        template<typename BaseIterator>
        struct non_disconnected_filter
        {
            static bool is_included(const BaseIterator& mIter) { return !mIter->bDisconnected; }
        };

    public:

        /// Type of the view returned by slots().
        using script_list_view = utils::view::adaptor<slot_list,
            slot_dereferencer,
            non_disconnected_filter>;

        /// Default constructor (no slot).
        signal() : pSlots_(std::make_shared<slot_list>())
        {
        }

        // Copiable, movable.
        signal(const signal&) = default;
        signal(signal&&) = default;
        signal& operator=(const signal&) = default;
        signal& operator=(signal&&) = default;

        /// Disconnects all slots.
        void disconnect_all()
        {
            if (uiRecursion_ == 0u)
            {
                pSlots_->clear();
                return;
            }

            // We are in the middle of an iteration; just mark slot as
            // disconnected. The slots will not be called, however they still
            // occupy a spot in the slot list. This is done to avoid invalidating
            // iterators while iterating over the slot list, if one of the
            // slot happens to disconnect itself. The memory will be cleared
            // up when the signal is no longer being iterated on.
            for (auto& mHandler : *pSlots_)
                mHandler.bDisconnected = true;
        }

        /// Check if this signal contains any slot.
        /** \return 'true' if at least one slot is connected.
        */
        bool has_slot() const
        {
            for (const auto& mHandler : *pSlots_)
            {
                if (!mHandler.bDisconnected)
                    return true;
            }

            return false;
        }

        /// Return a view onto the connected slots.
        /** \return A view onto the connected slots
        */
        script_list_view slots() const
        {
            return script_list_view(*pSlots_);
        }

        /// Connect a new slot to this signal.
        /** \param mFunction The function to store in the slot.
        */
        void connect(function_type mFunction)
        {
            pSlots_->push_back({std::move(mFunction), false});
        }

        /// Trigger the signal.
        /** \param mArgs Arguments to forward to all the connected slots.
        */
        template<typename ... Args>
        void operator()(Args&& ... mArgs)
        {
            // Make a shared-ownership copy of the handler list, so that the list
            // survives even if our owner is destroyed midway during a handler.
            const auto pSlotsCopy = pSlots_;

            ++uiRecursion_;

            // Call the slots
            for (const auto& mSlot : *pSlotsCopy)
            {
                if (!mSlot.bDisconnected)
                    mSlot.mCallback(mArgs...);
            }

            --uiRecursion_;

            if (uiRecursion_ == 0u)
                garbage_collect_();
        }

    private:
        void garbage_collect_()
        {
            auto mIterRemove = std::remove_if(pSlots_->begin(), pSlots_->end(),
                [](const auto& mSlot) { return mSlot.bDisconnected; });

            pSlots_->erase(mIterRemove, pSlots_->end());
        }

        std::shared_ptr<slot_list> pSlots_;
        std::size_t                uiRecursion_ = 0u;
    };
}
}

#endif
