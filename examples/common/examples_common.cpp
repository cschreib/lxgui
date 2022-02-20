#include "examples_common.hpp"

#include <iostream>
#include <lxgui/extern_sol2_state.hpp>
#include <lxgui/gui_button.hpp>
#include <lxgui/gui_editbox.hpp>
#include <lxgui/gui_event.hpp>
#include <lxgui/gui_factory.hpp>
#include <lxgui/gui_fontstring.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/gui_renderer.hpp>
#include <lxgui/gui_root.hpp>
#include <lxgui/gui_scrollframe.hpp>
#include <lxgui/gui_slider.hpp>
#include <lxgui/gui_statusbar.hpp>
#include <lxgui/gui_texture.hpp>
#include <lxgui/input_dispatcher.hpp>
#include <lxgui/input_world_dispatcher.hpp>
#include <lxgui/lxgui.hpp>
#include <lxgui/utils_filesystem.hpp>

using namespace lxgui;

float get_time_delta(const timing_clock::time_point& t1, const timing_clock::time_point& t2) {
    return std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1e6f;
}

void examples_setup_gui(gui::manager& manager) {
    // -------------------------------------------------
    // Initialise the GUI
    // -------------------------------------------------

    // Automatically select best settings
    gui::renderer& gui_renderer = manager.get_renderer();
    gui_renderer.auto_detect_settings();

    std::cout << " Renderer settings:" << std::endl;
    std::cout << "  Renderer: " << gui_renderer.get_name() << std::endl;
    std::cout << "  Max texture size: " << gui_renderer.get_texture_max_size() << std::endl;
    std::cout << "  Vertex cache supported: " << gui_renderer.is_vertex_cache_supported()
              << std::endl;
    std::cout << "  Vertex cache enabled: " << gui_renderer.is_vertex_cache_enabled() << std::endl;
    std::cout << "  Texture atlas supported: " << gui_renderer.is_texture_atlas_supported()
              << std::endl;
    std::cout << "  Texture atlas enabled: " << gui_renderer.is_texture_atlas_enabled()
              << std::endl;
    std::cout << "  Texture atlas page size: " << gui_renderer.get_texture_atlas_page_size()
              << std::endl;
    std::cout << "  Texture per-vertex color supported: "
              << gui_renderer.is_texture_vertex_color_supported() << std::endl;
    std::cout << "  Quad batching enabled: " << gui_renderer.is_quad_batching_enabled()
              << std::endl;

    // The first thing to do is register any required C++ classes and functions
    // onto the Lua state.
    manager.register_lua_glues([](gui::manager& mgr) {
        // We use a lambda function because this code might be called
        // again later on, for example when one reloads the GUI (the
        // lua state is destroyed and created again).

        //  - register the needed region types
        gui::factory& fac = mgr.get_factory();
        fac.register_region_type<gui::texture>();
        fac.register_region_type<gui::font_string>();
        fac.register_region_type<gui::button>();
        fac.register_region_type<gui::slider>();
        fac.register_region_type<gui::edit_box>();
        fac.register_region_type<gui::scroll_frame>();
        fac.register_region_type<gui::status_bar>();

        //  - register additional lua functions (add your own functions here)
        sol::state& lua = mgr.get_lua();
        lua.set_function("get_folder_list", [](const std::string& dir) {
            return sol::as_table(utils::get_directory_list(dir));
        });
        lua.set_function("get_file_list", [](const std::string& dir) {
            return sol::as_table(utils::get_file_list(dir));
        });
    });

    // Now the GUI is initialised and ready, but it is empty.
    // We need to create some GUI elements to display.

    // Load GUI elements from files:
    std::cout << " Reading gui files..." << std::endl;
    //  - set the directory in which the interface is located
    manager.add_addon_directory("interface");
    //  - and load all files
    manager.load_ui();

    // Alternatively, you can also create GUI elements directly in C++ code, see below.

    // -------------------------------------------------
    // Create GUI elements in C++
    // -------------------------------------------------

    // Create a frame
    // A "root" frame has no parent and is directly owned by the gui::manager.
    // A "child" frame is owned by another frame.
    utils::observer_ptr<gui::frame> fps_frame;
    fps_frame = manager.get_root().create_root_frame<gui::frame>("FPSCounter");
    fps_frame->set_point(gui::anchor_point::top_left);
    fps_frame->set_point(
        gui::anchor_point::bottom_right, "FontstringTestFrameText", gui::anchor_point::top_right);

    // Create a font_string in the frame
    utils::observer_ptr<gui::font_string> fps_text;
    fps_text =
        fps_frame->create_layered_region<gui::font_string>(gui::layer::artwork, "$parentText");
    fps_text->set_point(gui::anchor_point::bottom_right, gui::vector2f(0, -5));
    fps_text->set_font("interface/fonts/main.ttf", 15);
    fps_text->set_alignment_y(gui::alignment_y::bottom);
    fps_text->set_alignment_x(gui::alignment_x::right);
    fps_text->set_outlined(true);
    fps_text->set_text_color(gui::color::red);
    fps_text->notify_loaded();

    // Create the scripts for this frame
    // In Lua
    /*fps_frame->add_script("OnLoad",
        "self.update_time = 0.5;"
        "self.timer = 1.0;"
        "self.frames = 0;",
        "examples_common.cpp", 122 // You can provide the file/line information, for error handling
    );
    fps_frame->add_script("OnUpdate",
        "self.timer = self.timer + arg1;"
        "self.frames = self.frames + 1;"

        "if (self.timer > self.update_time) then"
        "    local fps = self.frames/self.timer;"
        "    self.Text:set_text(\"FPS : \"..math.floor(fps));"

        "    self.timer = 0.0;"
        "    self.frames = 0;"
        "end",
        "examples_common.cpp", 135 // You can provide the file/line information, for error handling
    );*/

    // Or in C++
    float       update_time = 0.5f, timer = 1.0f;
    std::size_t num_frames = 0;
    fps_frame->add_script("OnUpdate", [=](gui::frame& self, const gui::event_data& data) mutable {
        float delta = data.get<float>(0);
        timer += delta;
        ++num_frames;

        if (timer > update_time) {
            if (auto txt = self.get_region<gui::font_string>("Text")) {
                txt->set_text(
                    U"(created in C++)\nFPS : " +
                    utils::to_ustring(std::floor(num_frames / timer)));
            }

            timer      = 0.0f;
            num_frames = 0;
        }
    });

    // Tell the Frame is has been fully loaded, and call "OnLoad"
    fps_frame->notify_loaded();

    // The GUI now has some elements to display.
    // The next step is setting up callbacks to react to inputs events that are not
    // captured by the UI.

    // -------------------------------------------------
    // Reacting to inputs in your game
    // -------------------------------------------------

    // Lxgui offers multiple layers to react to events:
    //
    //  - input::world_dispatcher: processed and filtered events. Use this if you want
    //    to react to events that the UI allows (e.g., no UI element is blocking the mouse,
    //    and no UI element is capturing keyboard input for entering text). This is the
    //    recommended method, and it is illustrated below.
    //
    //  - input::dispatcher: processed and higher level events (double click, drag, etc.),
    //    but with no filtering applied. Use this if you need global events, unfiltered by UI
    //    elements. Usage: manager.get_input_dispatcher().
    //
    //  - input::source: simple raw events, with no processing or filtering applied.
    //    Use this if you need the raw inputs. Usage:
    //    Usage: manager.get_input_dispatcher().get_source().

    input::world_dispatcher& world_input_dispatcher = manager.get_world_input_dispatcher();
    world_input_dispatcher.on_key_pressed.connect([&](input::key key_code) {
        // Process keyboard inputs for the game...
        switch (key_code) {
        case input::key::k_p: gui::out << manager.print_ui() << std::endl; break;
        case input::key::k_c: manager.get_root().toggle_caching(); break;
        case input::key::k_r: manager.reload_ui(); break;
        case input::key::k_b:
            manager.get_renderer().set_quad_batching_enabled(
                !manager.get_renderer().is_quad_batching_enabled());
            break;
        case input::key::k_a:
            manager.get_renderer().set_texture_atlas_enabled(
                !manager.get_renderer().is_texture_atlas_enabled());
            break;
        default: break;
        }
    });

    world_input_dispatcher.on_mouse_pressed.connect(
        [&](input::mouse_button button_code, const gui::vector2f& mouse_pos) {
            // Process mouse inputs for the game...
        });
}
