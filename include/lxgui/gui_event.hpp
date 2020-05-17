#ifndef GUI_EVENT_HPP
#define GUI_EVENT_HPP

#include <lxgui/luapp_var.hpp>
#include <string>
#include <vector>

namespace lxgui {
namespace gui
{
    /// Contains an event informations
    class event
    {
    public :

        /// Default constructor.
        event() = default;

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
        void add(const utils::any& mValue);

        /// Returns a parameter of this event.
        /** \return A parameter of this event
        */
        const utils::any* get(uint uiIndex) const;

        /// Returns a parameter of this event.
        /** \return A parameter of this event
        */
        template<typename T>
        const T& get(uint uiIndex) const
        {
            return get(uiIndex)->get<T>();
        }

        /// Returns the number of parameter.
        /** \return The number of parameter
        */
        uint get_num_param() const;

        /// Returns the name of this event.
        /** \return The name of this event
        */
        const std::string& get_name() const;

        /// Checks if this event should only be fired once per frame.
        /** \return 'true' if this should only be fired once per frame
        */
        bool is_once_per_frame() const;

        utils::any& operator [] (uint uiIndex);

        const utils::any& operator [] (uint uiIndex) const;

    private :

        std::string           sName_;
        bool                  bOncePerFrame_ = false;
        std::vector<utils::any> lArgList_;
    };
}
}

#endif
