#include "lxgui/gui_button.hpp"
#include "lxgui/gui_editbox.hpp"
#include "lxgui/gui_event.hpp"
#include "lxgui/gui_factory.hpp"
#include "lxgui/gui_fontstring.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_root.hpp"
#include "lxgui/gui_scrollframe.hpp"
#include "lxgui/gui_slider.hpp"
#include "lxgui/gui_statusbar.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/input_dispatcher.hpp"
#include "lxgui/input_world_dispatcher.hpp"
#include "lxgui/utils_filesystem.hpp"
#include "lxgui/utils_string.hpp"

#include <sol/state.hpp>

//#define GLSFML_GUI
//#define GLSDL_GUI
//#define SDL_GUI
//#define SFML_GUI

#if defined(GLSFML_GUI)
// OpenGL + SFML input
#    include "lxgui/impl/gui_gl_renderer.hpp"
#    include "lxgui/impl/input_sfml_source.hpp"

#    include <SFML/Window.hpp>
#    if defined(LXGUI_PLATFORM_OSX)
#        include <OpenGL/gl.h>
#    else
#        include <GL/gl.h>
#    endif
#elif defined(GLSDL_GUI)
// OpenGL + SDL input
#    include "lxgui/impl/gui_gl_renderer.hpp"
#    include "lxgui/impl/input_sdl_source.hpp"
#    define SDL_MAIN_HANDLED
#    include <SDL.h>
#    if defined(LXGUI_PLATFORM_OSX)
#        include <OpenGL/gl.h>
#    else
#        include <GL/gl.h>
#    endif
#elif defined(SDL_GUI)
// SDL
#    include "lxgui/impl/gui_sdl.hpp"
#    include "lxgui/impl/input_sdl_source.hpp"
#    define SDL_MAIN_HANDLED
#    include <SDL.h>
#elif defined(SFML_GUI)
// SFML
#    include "lxgui/impl/gui_sfml.hpp"
#    include "lxgui/impl/input_sfml_source.hpp"

#    include <SFML/Graphics/RenderWindow.hpp>
#    include <SFML/Window.hpp>
#endif

#if defined(LXGUI_PLATFORM_WINDOWS)
#    define NOMINMAX
#    include <windows.h>
#    if defined(LXGUI_COMPILER_MSVC)
#        pragma comment(linker, "/entry:mainCRTStartup")
#    endif
#elif defined(LXGUI_COMPILER_EMSCRIPTEN)
#    include <emscripten.h>
#endif

#include <chrono>
#include <cstdint>
#include <fstream>
#include <thread>

using namespace lxgui;
using timing_clock = std::chrono::high_resolution_clock;

struct main_loop_context {
    bool                     b_running = true;
    bool                     b_focus   = true;
    float                    f_delta   = 0.1f;
    timing_clock::time_point m_prev_time;
    std::size_t              ui_frame_count     = 0;
    float                    f_accumulated_time = 0.0;

    gui::manager* p_manager = nullptr;

#if defined(SDL_GUI)
    SDL_Renderer* pRenderer = nullptr;
#elif defined(GLSDL_GUI)
    SDL_Window*   pWindow    = nullptr;
    SDL_GLContext pGLContext = nullptr;
#elif defined(SFML_GUI)
    sf::RenderWindow* pWindow = nullptr;
#elif defined(GLSFML_GUI)
    sf::Window* p_window = nullptr;
#endif
};

void main_loop(void* p_type_erased_data) {
#if defined(LXGUI_COMPILER_EMSCRIPTEN)
    try {
#endif

        main_loop_context& m_context = *reinterpret_cast<main_loop_context*>(p_type_erased_data);

        input::dispatcher& m_input_dispatcher = m_context.p_manager->get_input_dispatcher();

#if defined(SDL_GUI) || defined(GLSDL_GUI)
        // Get events from SDL
        SDL_Event mEvent;
        while (SDL_PollEvent(&mEvent)) {
            if (mEvent.type == SDL_WINDOWEVENT) {
                if (mEvent.window.event == SDL_WINDOWEVENT_CLOSE) {
#    if defined(LXGUI_COMPILER_EMSCRIPTEN)
                    emscripten_cancel_main_loop();
                    return;
#    else
                    mContext.bRunning = false;
                    return;
#    endif
                } else if (mEvent.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                    mContext.bFocus = false;
                else if (mEvent.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                    mContext.bFocus = true;
            }

            // Feed events to the GUI.
            // NB: Do not use raw keyboard/mouse events from SDL directly. See below.
            static_cast<input::sdl::source&>(mInputDispatcher.get_source()).on_sdl_event(mEvent);
        }
#elif defined(SFML_GUI) || defined(GLSFML_GUI)
    // Get events from SFML
    sf::Event m_event{};
    while (m_context.p_window->pollEvent(m_event)) {
        if (m_event.type == sf::Event::Closed) {
#    if defined(LXGUI_COMPILER_EMSCRIPTEN)
            emscripten_cancel_main_loop();
            return;
#    else
            m_context.b_running = false;
            return;
#    endif
        } else if (m_event.type == sf::Event::LostFocus)
            m_context.b_focus = false;
        else if (m_event.type == sf::Event::GainedFocus)
            m_context.b_focus = true;

        // Feed events to the GUI.
        // NB: Do not use raw keyboard/mouse events from SFML directly. See below.
        static_cast<input::sfml::source&>(m_input_dispatcher.get_source()).on_sfml_event(m_event);
    }
#endif

        if (!m_context.b_focus) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return;
        }

#if defined(GLSDL_GUI)
        SDL_GL_MakeCurrent(mContext.pWindow, mContext.pGLContext);
#endif

        // Update the gui
        timing_clock::time_point m_start = timing_clock::now();
        m_context.p_manager->update_ui(m_context.f_delta);

        // Clear the window
#if defined(GLSFML_GUI) || defined(GLSDL_GUI)
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
#elif defined(SDL_GUI)
    SDL_SetRenderDrawColor(mContext.pRenderer, 50, 50, 50, 255);
    SDL_RenderClear(mContext.pRenderer);
#elif defined(SFML_GUI)
    mContext.pWindow->clear(sf::Color(51, 51, 51));
#endif

        // Render the gui
        m_context.p_manager->render_ui();

        // Display the window
#if defined(SDL_GUI)
        SDL_RenderPresent(mContext.pRenderer);
#elif defined(SFML_GUI) || defined(GLSFML_GUI)
    m_context.p_window->display();
#elif defined(GLSDL_GUI)
    SDL_GL_SwapWindow(mContext.pWindow);
#endif
        timing_clock::time_point m_end = timing_clock::now();
        m_context.f_accumulated_time +=
            std::chrono::duration_cast<std::chrono::microseconds>(m_end - m_start).count() / 1e6;

        timing_clock::time_point m_current_time = timing_clock::now();
        m_context.f_delta = std::chrono::duration_cast<std::chrono::microseconds>(
                                m_current_time - m_context.m_prev_time)
                                .count() /
                            1e6;
        m_context.m_prev_time = m_current_time;

        ++m_context.ui_frame_count;

#if defined(LXGUI_COMPILER_EMSCRIPTEN)
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        emscripten_cancel_main_loop();
        return;
    } catch (...) {
        std::cout << "# Error # : Unhandled exception !" << std::endl;
        emscripten_cancel_main_loop();
        return;
    }
#endif
}

int main(int argc, char* argv[]) {
    auto* p_old_cout_buffer = std::cout.rdbuf();

    try {
        // -------------------------------------------------
        // Read test configuration
        // -------------------------------------------------

        std::size_t ui_window_width  = 800u;
        std::size_t ui_window_height = 600u;
        bool        b_full_screen    = false;
        float       f_scale_factor   = 1.0f;
        bool        b_print_to_log   = false;

        // Read some configuration data
        if (utils::file_exists("config.lua")) {
            sol::state m_lua;
            m_lua.do_file("config.lua");
            ui_window_width  = m_lua["window_width"].get_or(std::size_t{800u});
            ui_window_height = m_lua["window_height"].get_or(std::size_t{600u});
            b_full_screen    = m_lua["fullscreen"].get_or(false);
            f_scale_factor   = m_lua["scale_factor"].get_or(1.0);
            b_print_to_log   = m_lua["print_to_log"].get_or(false);
        }

#if defined(LXGUI_COMPILER_EMSCRIPTEN)
        // In WebAssembly builds, never print to a log file, because we don't have
        // disk write access. Just use the console.
        bPrintToLog = false;
#endif

        std::fstream m_log_cout("cout.txt", std::ios::out);
        std::fstream m_gui("gui.txt", std::ios::out);
        if (!b_print_to_log) {
            // Redirect output from the gui library to the standard output
            gui::out.rdbuf(std::cout.rdbuf());
        } else {
            // Redirect output from the standard output to a file
            m_log_cout.open("cout.txt", std::ios::out);
            std::cout.rdbuf(m_log_cout.rdbuf());

            // Redirect output from the gui library to a log file
            m_gui.open("gui.txt", std::ios::out);
            gui::out.rdbuf(m_gui.rdbuf());
        }

        // -------------------------------------------------
        // Create a window
        // -------------------------------------------------

        std::cout << "Creating window..." << std::endl;
        const std::string s_window_title = "test";

#if defined(GLSFML_GUI)
        sf::Window m_window;
#elif defined(SDL_GUI) || defined(GLSDL_GUI)
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw gui::exception(
                "SDL_Init", "Could not initialise SDL: " + std::string(SDL_GetError()) + ".");
        }

        std::uint32_t uiFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
        if (bFullScreen)
            uiFlags |= SDL_WINDOW_FULLSCREEN;

        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> pWindow(
            SDL_CreateWindow(
                sWindowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                uiWindowWidth, uiWindowHeight, uiFlags),
            &SDL_DestroyWindow);

        if (!pWindow)
            throw gui::exception("SDL_Window", "Could not create window.");

#    if defined(SDL_GUI)
        std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> pRenderer(
            SDL_CreateRenderer(
                pWindow.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE),
            &SDL_DestroyRenderer);

        if (!pRenderer)
            throw gui::exception("SDL_Renderer", "Could not create renderer.");
#    else
        // Helper class to manage the OpenGL context from SDL
        struct GLContext {
            SDL_GLContext pContext = nullptr;

            explicit GLContext(SDL_Window* pWindow) : pContext(SDL_GL_CreateContext(pWindow)) {
                if (pContext == nullptr) {
                    throw gui::exception(
                        "SDL_GL_CreateContext", "Coult not create OpenGL context.");
                }
            }

            ~GLContext() {
                SDL_GL_DeleteContext(pContext);
            }
        };

        GLContext mGLContext(pWindow.get());
        SDL_GL_SetSwapInterval(0);
#    endif

#elif defined(SFML_GUI)
        sf::RenderWindow mWindow;
#endif

#if defined(GLSFML_GUI) || defined(SFML_GUI)
        if (b_full_screen)
            m_window.create(
                sf::VideoMode(ui_window_width, ui_window_height, 32), s_window_title,
                sf::Style::Fullscreen);
        else
            m_window.create(sf::VideoMode(ui_window_width, ui_window_height, 32), s_window_title);
#endif

        // -------------------------------------------------
        // Initialise the GUI
        // -------------------------------------------------

        std::cout << "Creating gui manager..." << std::endl;
        utils::owner_ptr<gui::manager> p_manager;

#if defined(GLSFML_GUI) || defined(GLSDL_GUI)
        // Define the input manager
        std::unique_ptr<input::source> p_input_source;

#    if defined(GLSFML_GUI)
        // Use SFML
        p_input_source = std::make_unique<input::sfml::source>(m_window);
#    elif defined(GLSDL_GUI)
        // Use SDL
        {
            bool bInitializeSDLImage = true;
            SDL_Renderer* pRenderer = nullptr; // set to nullptr when not using an SDL_Renderer
            pInputSource =
                std::make_unique<input::sdl::source>(pWindow.get(), pRenderer, bInitializeSDLImage);
        }
#    endif

        // Define the GUI renderer
        std::unique_ptr<gui::renderer> p_renderer =
            std::make_unique<gui::gl::renderer>(p_input_source->get_window_dimensions());

        p_manager = utils::make_owned<gui::manager>(
            // Provide the input source
            std::move(p_input_source),
            // Provide the GUI renderer implementation
            std::move(p_renderer));
#elif defined(SDL_GUI)
        // Use full SDL implementation
        pManager            = gui::sdl::create_manager(pWindow.get(), pRenderer.get());
#elif defined(SFML_GUI)
        // Use full SFML implementation
        pManager           = gui::sfml::create_manager(mWindow);
#endif

        // Automatically select best settings
        gui::renderer& m_gui_renderer = p_manager->get_renderer();
        m_gui_renderer.auto_detect_settings();

        std::cout << " Preferred languages: ";
        for (const auto& s_language : p_manager->get_localizer().get_preferred_languages())
            std::cout << s_language << ", ";
        std::cout << std::endl;
        std::size_t ui_code_points = 0u;
        for (const auto& m_range : p_manager->get_localizer().get_allowed_code_points())
            ui_code_points += static_cast<std::size_t>(m_range.ui_last - m_range.ui_first) + 1;
        std::cout << " Required Unicode code points: " << ui_code_points << std::endl;
        std::cout << " Renderer settings:" << std::endl;
        std::cout << "  Renderer: " << m_gui_renderer.get_name() << std::endl;
        std::cout << "  Max texture size: " << m_gui_renderer.get_texture_max_size() << std::endl;
        std::cout << "  Vertex cache supported: " << m_gui_renderer.is_vertex_cache_supported()
                  << std::endl;
        std::cout << "  Vertex cache enabled: " << m_gui_renderer.is_vertex_cache_enabled()
                  << std::endl;
        std::cout << "  Texture atlas supported: " << m_gui_renderer.is_texture_atlas_supported()
                  << std::endl;
        std::cout << "  Texture atlas enabled: " << m_gui_renderer.is_texture_atlas_enabled()
                  << std::endl;
        std::cout << "  Texture atlas page size: " << m_gui_renderer.get_texture_atlas_page_size()
                  << std::endl;
        std::cout << "  Texture per-vertex color supported: "
                  << m_gui_renderer.is_texture_vertex_color_supported() << std::endl;
        std::cout << "  Quad batching enabled: " << m_gui_renderer.is_quad_batching_enabled()
                  << std::endl;

        p_manager->set_interface_scaling_factor(f_scale_factor);

        // Load files :
        //  - first set the directory in which the interface is located
        p_manager->add_addon_directory("interface");
        //  - register Lua "glues" (C++ functions and classes callable from Lua)
        p_manager->register_lua_glues([](gui::manager& m_manager) {
            // We use a lambda function because this code might be called
            // again later on, for example when one reloads the GUI (the
            // lua state is destroyed and created again).
            //  - register the needed region types
            gui::factory& m_factory = m_manager.get_factory();
            m_factory.register_region_type<gui::texture>();
            m_factory.register_region_type<gui::font_string>();
            m_factory.register_region_type<gui::button>();
            m_factory.register_region_type<gui::slider>();
            m_factory.register_region_type<gui::edit_box>();
            m_factory.register_region_type<gui::scroll_frame>();
            m_factory.register_region_type<gui::status_bar>();
            //  - register additional lua functions
            sol::state& m_lua = m_manager.get_lua();
            m_lua.set_function("get_folder_list", [](const std::string& s_dir) {
                return sol::as_table(utils::get_directory_list(s_dir));
            });
            m_lua.set_function("get_file_list", [](const std::string& s_dir) {
                return sol::as_table(utils::get_file_list(s_dir));
            });
        });

        //  - and load all files
        std::cout << " Reading gui files..." << std::endl;
        p_manager->load_ui();

        // Create context for the main loop
        main_loop_context m_context;
        m_context.p_manager = p_manager.get();

#if defined(SDL_GUI)
        mContext.pRenderer = pRenderer.get();
#elif defined(GLSDL_GUI)
        mContext.pWindow    = pWindow.get();
        mContext.pGLContext = mGLContext.pContext;
#elif defined(SFML_GUI) || defined(GLSFML_GUI)
        m_context.p_window = &m_window;
#endif

        // -------------------------------------------------
        // Create GUI elements in C++
        // -------------------------------------------------

        // Create the Frame
        // A "root" frame has no parent and is directly owned by the gui::manager.
        // A "child" frame is owned by another frame.
        utils::observer_ptr<gui::frame> p_frame;
        p_frame = p_manager->get_root().create_root_frame<gui::frame>("FPSCounter");
        p_frame->set_point(gui::anchor_point::top_left);
        p_frame->set_point(
            gui::anchor_point::bottom_right, "FontstringTestFrameText",
            gui::anchor_point::top_right);

        // Create the FontString
        utils::observer_ptr<gui::font_string> p_font;
        p_font =
            p_frame->create_layered_region<gui::font_string>(gui::layer::artwork, "$parentText");
        p_font->set_point(gui::anchor_point::bottom_right, gui::vector2f(0, -5));
        p_font->set_font("interface/fonts/main.ttf", 15);
        p_font->set_alignment_y(gui::alignment_y::bottom);
        p_font->set_alignment_x(gui::alignment_x::right);
        p_font->set_outlined(true);
        p_font->set_text_color(gui::color::red);
        p_font->notify_loaded();

        // Create the scripts
        // In Lua
        /*pFrame->add_script("OnLoad",
            "self.update_time = 0.5;"
            "self.timer = 1.0;"
            "self.frames = 0;",
            "main.cpp", 146 // You can provide the location of the Lua source, for error handling
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
            "main.cpp", 152 // You can provide the location of the Lua source, for error handling
        );*/

        // Or in C++:

        float f_timer = 1.0f;
        p_frame->add_script(
            "OnUpdate",
            [f_timer, &m_context](gui::frame& m_self, const gui::event_data& m_data) mutable {
                float f_delta = m_data.get<float>(0);
                f_timer += f_delta;

                if (f_timer > 0.5f) {
                    float f_frame_time =
                        1e6 * m_context.f_accumulated_time / m_context.ui_frame_count;

                    if (auto p_text = m_self.get_region<gui::font_string>("Text")) {
                        p_text->set_text(
                            U"(created in C++)\nFrame time (us) : " +
                            utils::to_ustring(std::round(f_frame_time)));
                    }

                    f_timer                      = 0.0f;
                    m_context.ui_frame_count     = 0;
                    m_context.f_accumulated_time = 0;
                }
            });

        // Tell the Frame is has been fully loaded, and call "OnLoad"
        p_frame->notify_loaded();

        // -------------------------------------------------
        // Reacting to inputs in your game
        // -------------------------------------------------

        // Register callbacks for input to the world.
        // Lxgui offers multiple layers to react to events:
        //  - input::world_dispatcher: processed and filtered events. Use this if you want
        //    to react to events that the UI allows (e.g., no UI element is blocking the mouse,
        //    and no UI element is capturing keyboard input for entering text). This is the
        //    recommended method, and it is illustrated below.
        //  - input::dispatcher: processed and higher level events (double click, drag, etc.),
        //    but with no filtering applied. Use this if you need global events, unfiltered by UI
        //    elements. Usage: pManager->get_input_dispatcher().
        //  - input::source: simple raw events, with no processing or filtering applied.
        //    Use this if you need the raw inputs. Usage:
        //    Usage: pManager->get_input_dispatcher().get_source().
        input::world_dispatcher& m_world_input_dispatcher = p_manager->get_world_input_dispatcher();
        m_world_input_dispatcher.on_key_pressed.connect([&](input::key m_key) {
            // Process keyboard inputs for the game...
            switch (m_key) {
            case input::key::k_escape: {
#if defined(LXGUI_COMPILER_EMSCRIPTEN)
                emscripten_cancel_main_loop();
                return;
#else
                m_context.b_running = false;
                return;
#endif
            }
            case input::key::k_p: gui::out << m_context.p_manager->print_ui() << std::endl; break;
            case input::key::k_k: gui::out << "###" << std::endl; break;
            case input::key::k_c: m_context.p_manager->get_root().toggle_caching(); break;
            case input::key::k_r: m_context.p_manager->reload_ui(); break;
            case input::key::k_b:
                m_context.p_manager->get_renderer().set_quad_batching_enabled(
                    !m_context.p_manager->get_renderer().is_quad_batching_enabled());
                break;
            case input::key::k_a:
                m_context.p_manager->get_renderer().set_texture_atlas_enabled(
                    !m_context.p_manager->get_renderer().is_texture_atlas_enabled());
                break;
            default: break;
            }
        });

        m_world_input_dispatcher.on_mouse_pressed.connect(
            [&](input::mouse_button m_button, const gui::vector2f& m_mouse_pos) {
                // Process mouse inputs for the game...
            });

        // -------------------------------------------------
        // Start the main loop
        // -------------------------------------------------

        m_context.m_prev_time = timing_clock::now();

        std::cout << "Entering loop..." << std::endl;

#if defined(LXGUI_COMPILER_EMSCRIPTEN)
        emscripten_set_main_loop_arg(main_loop, &mContext, -1, 1);
#else
        while (m_context.b_running) {
            main_loop(&m_context);
        }
#endif

        std::cout << "End of loop." << std::endl;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "# Error # : Unhandled exception !" << std::endl;
        return 1;
    }

    std::cout << "End of program." << std::endl;
    std::cout.rdbuf(p_old_cout_buffer);

    return 0;
}
