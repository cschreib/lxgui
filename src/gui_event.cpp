#include "lxgui/gui_event.hpp"

namespace lxgui::gui {

event_data::event_data(std::initializer_list<utils::variant> l_data) : l_arg_list_(l_data) {}

} // namespace lxgui::gui
