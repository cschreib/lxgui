#include "examples_common.hpp"

#include <lxgui/lxgui.hpp>
#include <lxgui/gui_renderer.hpp>
#include <lxgui/gui_texture.hpp>
#include <lxgui/gui_fontstring.hpp>
#include <lxgui/gui_button.hpp>
#include <lxgui/gui_slider.hpp>
#include <lxgui/gui_editbox.hpp>
#include <lxgui/gui_scrollframe.hpp>
#include <lxgui/gui_statusbar.hpp>
#include <lxgui/gui_event.hpp>
#include <lxgui/input.hpp>
#include <lxgui/utils_filesystem.hpp>

#include <sol/state.hpp>

#include <iostream>

using namespace lxgui;

double get_time_delta(const timing_clock::time_point& mT1, const timing_clock::time_point& mT2)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(mT2 - mT1).count() / 1e6;
}

void examples_setup_gui(gui::manager& mManager)
{
    // Automatically select best settings
    gui::renderer* pGUIRenderer = mManager.get_renderer();
    pGUIRenderer->auto_detect_settings();

    std::cout << " Renderer settings:" << std::endl;
    std::cout << "  Renderer: " << pGUIRenderer->get_name() << std::endl;
    std::cout << "  Max texture size: " << pGUIRenderer->get_texture_max_size() << std::endl;
    std::cout << "  Vertex cache supported: " << pGUIRenderer->is_vertex_cache_supported() << std::endl;
    std::cout << "  Vertex cache enabled: " << pGUIRenderer->is_vertex_cache_enabled() << std::endl;
    std::cout << "  Texture atlas supported: " << pGUIRenderer->is_texture_atlas_supported() << std::endl;
    std::cout << "  Texture atlas enabled: " << pGUIRenderer->is_texture_atlas_enabled() << std::endl;
    std::cout << "  Texture atlas page size: " << pGUIRenderer->get_texture_atlas_page_size() << std::endl;
    std::cout << "  Texture per-vertex color supported: " << pGUIRenderer->is_texture_vertex_color_supported() << std::endl;
    std::cout << "  Quad batching enabled: " << pGUIRenderer->is_quad_batching_enabled() << std::endl;

    // The first thing to do is create the lua::state, and register any glue function
    // into the Lua state to call into your C++ application.
    std::cout << " Creating lua..." << std::endl;
    mManager.create_lua([](gui::manager& mManager) {
        // We use a lambda function because this code might be called
        // again later on, for example when one reloads the GUI (the
        // lua state is destroyed and created again).

        //  - register the needed widgets
        mManager.register_region_type<gui::texture>();
        mManager.register_region_type<gui::font_string>();
        mManager.register_frame_type<gui::button>();
        mManager.register_frame_type<gui::slider>();
        mManager.register_frame_type<gui::edit_box>();
        mManager.register_frame_type<gui::scroll_frame>();
        mManager.register_frame_type<gui::status_bar>();

        //  - register additional lua functions (add your own functions here)
        sol::state& mSol = mManager.get_lua();
        mSol.set_function("get_folder_list", [](const std::string& sDir) {
            return utils::get_directory_list(sDir);
        });
        mSol.set_function("get_file_list", [](const std::string& sDir) {
            return utils::get_file_list(sDir);
        });
    });

    // Now the GUI is ready. We need to create some GUI elements to display.

    // Load GUI elements from files:
    std::cout << " Reading gui files..." << std::endl;
    //  - set the directory in which the interface is located
    mManager.add_addon_directory("interface");
    //  - and load all files
    mManager.read_files();

    // Alternatively, you can also create GUI elements directly in C++ code:

    // Create a frame
    // A "root" frame has no parent and is directly owned by the gui::manager.
    // A "child" frame is owned by another frame.
    gui::frame* pFrame = mManager.create_root_frame<gui::frame>("FPSCounter");
    pFrame->set_abs_point(gui::anchor_point::TOPLEFT, "", gui::anchor_point::TOPLEFT);
    pFrame->set_abs_point(gui::anchor_point::BOTTOMRIGHT, "FontstringTestFrameText", gui::anchor_point::TOPRIGHT);

    // Create a font_string in the frame
    gui::font_string* pFont = pFrame->create_region<gui::font_string>(gui::layer_type::ARTWORK, "$parentText");
    pFont->set_abs_point(gui::anchor_point::BOTTOMRIGHT, "$parent", gui::anchor_point::BOTTOMRIGHT, 0, -5);
    pFont->set_font("interface/fonts/main.ttf", 12);
    pFont->set_justify_v(gui::text::vertical_alignment::BOTTOM);
    pFont->set_justify_h(gui::text::alignment::RIGHT);
    pFont->set_outlined(true);
    pFont->set_text_color(gui::color::RED);
    pFont->notify_loaded();

    // Create the scripts for this frame
    // In Lua
    /*pFrame->define_script("OnLoad",
        "self.update_time = 0.5;"
        "self.timer = 1.0;"
        "self.frames = 0;",
        "main.cpp", 146 // You can provide the location of the Lua source, for error handling
    );
    pFrame->define_script("OnUpdate",
        "self.timer = self.timer + arg1;"
        "self.frames = self.frames + 1;"

        "if (self.timer > self.update_time) then"
        "    local fps = self.frames/self.timer;"
        "    self.Text:set_text(\"FPS : \"..math.floor(fps));"

        "    self.timer = 0.0;"
        "    self.frames = 0;"
        "end",
        "main.cpp", 152 // You can provide the location of the Lua source, for error handling
    );*/

    // Or in C++
    float update_time = 0.5f, timer = 1.0f;
    int frames = 0;
    pFrame->define_script("OnUpdate",
        [=](gui::frame& self, gui::event* event) mutable {
            float delta = event->get<float>(0);
            timer += delta;
            ++frames;

            if (timer > update_time) {
                gui::font_string* text = self.get_region<gui::font_string>("Text");
                text->set_text(U"(created in C++)\nFPS : "+utils::to_ustring(floor(frames/timer)));

                timer = 0.0f;
                frames = 0;
            }
        }
    );

    // Tell the Frame is has been fully loaded, and call "OnLoad"
    pFrame->notify_loaded();
}
