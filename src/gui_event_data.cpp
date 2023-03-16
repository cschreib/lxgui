#include "lxgui/gui_event_data.hpp"

namespace lxgui::gui {

event_data::event_data(std::initializer_list<utils::variant> data) : arg_list_(data) {}

} // namespace lxgui::gui
