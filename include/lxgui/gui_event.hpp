#ifndef LXGUI_GUI_EVENT_HPP
#define LXGUI_GUI_EVENT_HPP

#include <lxgui/lxgui.hpp>
#include <lxgui/utils_variant.hpp>

#include <string>
#include <vector>

namespace lxgui {
namespace gui
{
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
        explicit event(const std::string& sName, bool bOncePerFrame = false);

        /// Sets this event's name.
        /** \param sName The name of this event
        */
        void set_name(const std::string& sName);

        /// Sets whether this event can only be fired once per frame.
        /** \param bOncePerFrame 'true' if you allow several events of
        *                        this type to be fired during the same
        *                        frame
        */
        void set_once_per_frame(bool bOncePerFrame);

        /// Adds a parameter to this event.
        /** \param mValue The value
        */
        void add(const utils::variant& mValue);

        /// Returns a parameter of this event.
        /** \param uiIndex The index of the parameter (see get_num_param())
        *   \return A parameter of this event
        */
        const utils::variant& get(std::size_t uiIndex) const;

        /// Returns a parameter of this event.
        /** \param uiIndex The index of the parameter (see get_num_param())
        *   \return A parameter of this event
        */
        utils::variant& get(std::size_t uiIndex);

        /// Returns a parameter of this event.
        /** \param uiIndex The index of the parameter (see get_num_param())
        *   \return A parameter of this event
        */
        template<typename T>
        const T& get(std::size_t uiIndex) const
        {
            return utils::get<T>(this->get(uiIndex));
        }

        /// Returns the number of parameters.
        /** \return The number of parameters
        */
        std::size_t get_num_param() const;

        /// Returns the name of this event.
        /** \return The name of this event
        */
        const std::string& get_name() const;

        /// Checks if this event should only be fired once per frame.
        /** \return 'true' if this should only be fired once per frame
        */
        bool is_once_per_frame() const;

    private :

        std::string                 sName_;
        bool                        bOncePerFrame_ = false;
        std::vector<utils::variant> lArgList_;
    };
}
}

#endif
