#include "lxgui/gui_out.hpp"

namespace lxgui::gui {

std::ostream      out(std::cout.rdbuf());
const std::string warning = "warning: ";
const std::string error   = "error: ";

} // namespace lxgui::gui
