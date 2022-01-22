#ifndef LXGUI_GUI_EVENT_HPP
#define LXGUI_GUI_EVENT_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/gui_exception.hpp>
#include <lxgui/utils_variant.hpp>

#include <string>
#include <vector>

namespace lxgui {
namespace gui
{
    /// Stores a variable number of arguments for an event.
    class event_data
    {
    public :

        /// Default constructor.
        event_data() = default;

        /// Copiable
        event_data(const event_data&) = default;

        /// Movable
        event_data(event_data&&) = default;

        /// Copiable
        event_data& operator=(const event_data&) = default;

        /// Movable
        event_data& operator=(event_data&&) = default;

        /// Adds a parameter to this event.
        /** \param mValue The value
        */
        void add(const utils::variant& mValue) { lArgList_.push_back(mValue); }

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

    /// Represents a generic event and associated information
    class event
    {
    public :

        /// Default constructor.
        event() = default;

        /// Copiable
        event(const event&) = default;

        /// Movable
        event(event&&) = default;

        /// Copiable
        event& operator=(const event&) = default;

        /// Movable
        event& operator=(event&&) = default;

        /// Constructor.
        /** \param sName         The name of this event
        *   \param bOncePerFrame 'true' if you allow several events of
        *                        this type to be fired during the same
        *                        frame
        */
        explicit event(const std::string& sName);

        /// Sets this event's name.
        /** \param sName The name of this event
        */
        void set_name(const std::string& sName);

        /// Returns the arguments of this event.
        /** \return the arguments of this event
        */
        const event_data& data() const { return mData_; }

        /// Returns the arguments of this event.
        /** \return the arguments of this event
        */
        event_data& data() { return mData_; }

        /// Returns the name of this event.
        /** \return The name of this event
        */
        const std::string& get_name() const;

        /// Adds a parameter to this event.
        /** \param mValue The value
        */
        void add(const utils::variant& mValue) { mData_.add(mValue); }

        /// Returns a parameter of this event.
        /** \param uiIndex The index of the parameter (see get_num_param())
        *   \return A parameter of this event
        */
        const utils::variant& get(std::size_t uiIndex) const { return mData_.get(uiIndex); }

        /// Returns a parameter of this event.
        /** \param uiIndex The index of the parameter (see get_num_param())
        *   \return A parameter of this event
        */
        utils::variant& get(std::size_t uiIndex) { return mData_.get(uiIndex); }

        /// Returns a parameter of this event.
        /** \param uiIndex The index of the parameter (see get_num_param())
        *   \return A parameter of this event
        */
        template<typename T>
        const T& get(std::size_t uiIndex) const { return mData_.get<T>(uiIndex); }

        /// Returns a parameter of this event.
        /** \param uiIndex The index of the parameter (see get_num_param())
        *   \return A parameter of this event
        */
        template<typename T>
        T& get(std::size_t uiIndex) { return mData_.get<T>(uiIndex); }

        /// Returns the number of parameters.
        /** \return The number of parameters
        */
        std::size_t get_num_param() const { return mData_.get_num_param(); }

    private :

        std::string sName_;
        event_data  mData_;
    };
}
}

#endif
