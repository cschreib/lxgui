#ifndef LXGUI_GUI_EVENT_HPP
#define LXGUI_GUI_EVENT_HPP

#include "lxgui/gui_exception.hpp"
#include "lxgui/lxgui.hpp"
#include "lxgui/utils_variant.hpp"

#include <initializer_list>
#include <vector>

namespace lxgui::gui {

/// Stores a variable number of arguments for an event.
class event_data {
public:
    /// Default constructor.
    event_data() = default;

    /// List constructor.
    event_data(std::initializer_list<utils::variant> data);

    // Copiable, movable
    event_data(const event_data&) = default;
    event_data(event_data&&)      = default;
    event_data& operator=(const event_data&) = default;
    event_data& operator=(event_data&&) = default;

    /// Adds a parameter to this event.
    /** \param value The value
     */
    template<typename T>
    void add(T&& value) {
        arg_list_.push_back(std::forward<T>(value));
    }

    /// Returns a parameter of this event.
    /** \param uiIndex The index of the parameter (see get_num_param())
     *   \return A parameter of this event
     */
    const utils::variant& get(std::size_t ui_index) const {
        if (ui_index >= arg_list_.size())
            throw gui::exception("event_data", "index past size of data");
        return arg_list_[ui_index];
    }

    /// Returns a parameter of this event.
    /** \param uiIndex The index of the parameter (see get_num_param())
     *   \return A parameter of this event
     */
    utils::variant& get(std::size_t ui_index) {
        if (ui_index >= arg_list_.size())
            throw gui::exception("event_data", "index past size of data");
        return arg_list_[ui_index];
    }

    /// Returns a parameter of this event.
    /** \param uiIndex The index of the parameter (see get_num_param())
     *   \return A parameter of this event
     */
    template<typename T>
    const T& get(std::size_t ui_index) const {
        return utils::get<T>(this->get(ui_index));
    }

    /// Returns a parameter of this event.
    /** \param uiIndex The index of the parameter (see get_num_param())
     *   \return A parameter of this event
     */
    template<typename T>
    T& get(std::size_t ui_index) {
        return utils::get<T>(this->get(ui_index));
    }

    /// Returns the number of parameters.
    /** \return The number of parameters
     */
    std::size_t get_num_param() const {
        return arg_list_.size();
    }

private:
    std::vector<utils::variant> arg_list_;
};

} // namespace lxgui::gui

#endif
