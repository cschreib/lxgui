#include "lxgui/gui_event.hpp"

namespace lxgui::gui {

event_data::event_data(std::initializer_list<utils::variant> data) : arg_list_(data) {}

} // namespace lxgui::gui
