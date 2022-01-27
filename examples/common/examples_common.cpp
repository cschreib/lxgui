#include "examples_common.hpp"

#include <lxgui/lxgui.hpp>
#include <lxgui/gui_renderer.hpp>
#include <lxgui/gui_factory.hpp>
#include <lxgui/gui_texture.hpp>
#include <lxgui/gui_fontstring.hpp>
#include <lxgui/gui_button.hpp>
#include <lxgui/gui_slider.hpp>
#include <lxgui/gui_editbox.hpp>
#include <lxgui/gui_scrollframe.hpp>
#include <lxgui/gui_statusbar.hpp>
#include <lxgui/gui_event.hpp>
#include <lxgui/gui_root.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/input_dispatcher.hpp>
#include <lxgui/input_world_dispatcher.hpp>
#include <lxgui/utils_filesystem.hpp>

#include <sol/state.hpp>

#include <iostream>

using namespace lxgui;

float get_time_delta(const timing_clock::time_point& mT1, const timing_clock::time_point& mT2)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(mT2 - mT1).count() / 1e6f;
}

void examples_setup_gui(gui::manager& mManager)
{
    // -------------------------------------------------
    // Initialise the GUI
    // -------------------------------------------------

    // Automatically select best settings
    gui::renderer& mGUIRenderer = mManager.get_renderer();
    mGUIRenderer.auto_detect_settings();

    std::cout << " Renderer settings:" << std::endl;
    std::cout << "  Renderer: " << mGUIRenderer.get_name() << std::endl;
    std::cout << "  Max texture size: " << mGUIRenderer.get_texture_max_size() << std::endl;
    std::cout << "  Vertex cache supported: " << mGUIRenderer.is_vertex_cache_supported() << std::endl;
    std::cout << "  Vertex cache enabled: " << mGUIRenderer.is_vertex_cache_enabled() << std::endl;
    std::cout << "  Texture atlas supported: " << mGUIRenderer.is_texture_atlas_supported() << std::endl;
    std::cout << "  Texture atlas enabled: " << mGUIRenderer.is_texture_atlas_enabled() << std::endl;
    std::cout << "  Texture atlas page size: " << mGUIRenderer.get_texture_atlas_page_size() << std::endl;
    std::cout << "  Texture per-vertex color supported: " << mGUIRenderer.is_texture_vertex_color_supported() << std::endl;
    std::cout << "  Quad batching enabled: " << mGUIRenderer.is_quad_batching_enabled() << std::endl;

    // The first thing to do is register any required C++ classes and functions
    // onto the Lua state.
    mManager.register_lua_glues([](gui::manager& mManager)
    {
        // We use a lambda function because this code might be called
        // again later on, for example when one reloads the GUI (the
        // lua state is destroyed and created again).

        //  - register the needed region types
        gui::factory& mFactory = mManager.get_factory();
        mFactory.register_region_type<gui::texture>();
        mFactory.register_region_type<gui::font_string>();
        mFactory.register_region_type<gui::button>();
        mFactory.register_region_type<gui::slider>();
        mFactory.register_region_type<gui::edit_box>();
        mFactory.register_region_type<gui::scroll_frame>();
        mFactory.register_region_type<gui::status_bar>();

        //  - register additional lua functions (add your own functions here)
        sol::state& mLua = mManager.get_lua();
        mLua.set_function("get_folder_list", [](const std::string& sDir)
        {
            return sol::as_table(utils::get_directory_list(sDir));
        });
        mLua.set_function("get_file_list", [](const std::string& sDir)
        {
            return sol::as_table(utils::get_file_list(sDir));
        });
    });

    // Now the GUI is initialised and ready, but it is empty.
    // We need to create some GUI elements to display.

    // Load GUI elements from files:
    std::cout << " Reading gui files..." << std::endl;
    //  - set the directory in which the interface is located
    mManager.add_addon_directory("interface");
    //  - and load all files
    mManager.load_ui();

    // Alternatively, you can also create GUI elements directly in C++ code, see below.

    // -------------------------------------------------
    // Create GUI elements in C++
    // -------------------------------------------------

    // Create a frame
    // A "root" frame has no parent and is directly owned by the gui::manager.
    // A "child" frame is owned by another frame.
    utils::observer_ptr<gui::frame> pFrame;
    pFrame = mManager.get_root().create_root_frame<gui::frame>("FPSCounter");
    pFrame->set_point(gui::anchor_data(gui::anchor_point::TOPLEFT));
    pFrame->set_point(gui::anchor_data(
        gui::anchor_point::BOTTOMRIGHT, "FontstringTestFrameText", gui::anchor_point::TOPRIGHT));

    // Create a font_string in the frame
    utils::observer_ptr<gui::font_string> pFont;
    pFont = pFrame->create_layered_region<gui::font_string>(gui::layer::ARTWORK, "$parentText");
    pFont->set_point(gui::anchor_data(gui::anchor_point::BOTTOMRIGHT, gui::vector2f(0, -5)));
    pFont->set_font("interface/fonts/main.ttf", 15);
    pFont->set_justify_v(gui::text::vertical_alignment::BOTTOM);
    pFont->set_justify_h(gui::text::alignment::RIGHT);
    pFont->set_outlined(true);
    pFont->set_text_color(gui::color::RED);
    pFont->notify_loaded();

    // Create the scripts for this frame
    // In Lua
    /*pFrame->add_script("OnLoad",
        "self.update_time = 0.5;"
        "self.timer = 1.0;"
        "self.frames = 0;",
        "examples_common.cpp", 122 // You can provide the file/line information, for error handling
    );
    pFrame->add_script("OnUpdate",
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
    float fUpdateTime = 0.5f, fTimer = 1.0f;
    std::size_t uiFrames = 0;
    pFrame->add_script("OnUpdate", [=](gui::frame& mSelf, const gui::event_data& mData) mutable
    {
        float fDelta = mData.get<float>(0);
        fTimer += fDelta;
        ++uiFrames;

        if (fTimer > fUpdateTime)
        {
            if (auto pText = mSelf.get_region<gui::font_string>("Text"))
            {
                pText->set_text(U"(created in C++)\nFPS : " +
                    utils::to_ustring(std::floor(uiFrames/fTimer)));
            }

            fTimer = 0.0f;
            uiFrames = 0;
        }
    });

    // Tell the Frame is has been fully loaded, and call "OnLoad"
    pFrame->notify_loaded();

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
    //    elements. Usage: mManager.get_input_dispatcher().
    //
    //  - input::source: simple raw events, with no processing or filtering applied.
    //    Use this if you need the raw inputs. Usage:
    //    Usage: mManager.get_input_dispatcher().get_source().

    input::world_dispatcher& mWorldInputDispatcher = mManager.get_world_input_dispatcher();
    mWorldInputDispatcher.on_key_pressed.connect(
        [&](input::key mKey)
        {
            // Process keyboard inputs for the game...
            switch (mKey)
            {
            case input::key::K_P:
                gui::out << mManager.print_ui() << std::endl;
                break;
            case input::key::K_C:
                mManager.get_root().toggle_caching();
                break;
            case input::key::K_R:
                mManager.reload_ui();
                break;
            case input::key::K_B:
                mManager.get_renderer().set_quad_batching_enabled(
                    !mManager.get_renderer().is_quad_batching_enabled());
                break;
            case input::key::K_A:
                mManager.get_renderer().set_texture_atlas_enabled(
                    !mManager.get_renderer().is_texture_atlas_enabled());
                break;
            default:
                break;
            }
        }
    );

    mWorldInputDispatcher.on_mouse_pressed.connect(
        [&](input::mouse_button mButton, const gui::vector2f& mMousePos)
        {
            // Process mouse inputs for the game...
        }
    );
}
