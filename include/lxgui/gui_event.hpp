#ifndef LXGUI_GUI_EVENT_HPP
#define LXGUI_GUI_EVENT_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/gui_exception.hpp>
#include <lxgui/utils_variant.hpp>

#include <vector>
#include <initializer_list>

namespace lxgui {
namespace gui
{
    /// Stores a variable number of arguments for an event.
    class event_data
    {
    public :

        /// Default constructor.
        event_data() = default;

        /// List constructor.
        event_data(std::initializer_list<utils::variant> lData);

        // Copiable, movable
        event_data(const event_data&) = default;
        event_data(event_data&&) = default;
        event_data& operator=(const event_data&) = default;
        event_data& operator=(event_data&&) = default;

        /// Adds a parameter to this event.
        /** \param mValue The value
        */
        template<typename T>
        void add(T&& mValue) { lArgList_.push_back(std::forward<T>(mValue)); }

        /// Returns a parameter of this event.
        /** \param uiIndex The index of the parameter (see get_num_param())
        *   \return A parameter of this event
        */
        const utils::variant& get(std::size_t uiIndex) const
        {
            if (uiIndex >= lArgList_.size())
                throw gui::exception("event_data", "index past size of data");
            return lArgList_[uiIndex];
        }

        /// Returns a parameter of this event.
        /** \param uiIndex The index of the parameter (see get_num_param())
        *   \return A parameter of this event
        */
        utils::variant& get(std::size_t uiIndex)
        {
            if (uiIndex >= lArgList_.size())
                throw gui::exception("event_data", "index past size of data");
            return lArgList_[uiIndex];
        }

        /// Returns a parameter of this event.
        /** \param uiIndex The index of the parameter (see get_num_param())
        *   \return A parameter of this event
        */
        template<typename T>
        const T& get(std::size_t uiIndex) const
        {
            return utils::get<T>(this->get(uiIndex));
        }

        /// Returns a parameter of this event.
        /** \param uiIndex The index of the parameter (see get_num_param())
        *   \return A parameter of this event
        */
        template<typename T>
        T& get(std::size_t uiIndex)
        {
            return utils::get<T>(this->get(uiIndex));
        }

        /// Returns the number of parameters.
        /** \return The number of parameters
        */
        std::size_t get_num_param() const { return lArgList_.size(); }

    private :

        std::vector<utils::variant> lArgList_;
    };
}
}

#endif
