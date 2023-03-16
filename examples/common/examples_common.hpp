#include <chrono>
#include <lxgui/gui_manager.hpp>
#include <lxgui/lxgui.hpp>

// Type for clock, used to measure delta between frames
using timing_clock = std::chrono::high_resolution_clock;

// Utility function to compute the time difference (in seconds) between two time points
float get_time_delta(const timing_clock::time_point& t1, const timing_clock::time_point& t2);

// Example function to load the GUI from the "interface" folder, and create some GUI elements in
// code
void examples_setup_gui(lxgui::gui::manager& manager);
