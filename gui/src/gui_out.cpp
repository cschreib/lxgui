#include "lxgui/gui_out.hpp"

namespace gui
{
std::ostream      out(std::cout.rdbuf());
const std::string warning = "# Warning # : ";
const std::string error = "# Error # : ";
}
