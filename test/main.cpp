#include "lxgui/gui_button.hpp"
#include "lxgui/gui_edit_box.hpp"
#include "lxgui/gui_factory.hpp"
#include "lxgui/gui_font_string.hpp"
#include "lxgui/gui_localizer.hpp"
#include "lxgui/gui_manager.hpp"
#include "lxgui/gui_out.hpp"
#include "lxgui/gui_root.hpp"
#include "lxgui/gui_scroll_frame.hpp"
#include "lxgui/gui_slider.hpp"
#include "lxgui/gui_status_bar.hpp"
#include "lxgui/gui_texture.hpp"
#include "lxgui/input_dispatcher.hpp"
#include "lxgui/input_world_dispatcher.hpp"
#include "lxgui/utils_file_system.hpp"
#include "lxgui/utils_string.hpp"

#include <lxgui/extern_sol2_state.hpp>

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
    bool                     running = true;
    bool                     focus   = true;
    float                    delta   = 0.1f;
    timing_clock::time_point prev_time;
    std::size_t              frame_count      = 0;
    float                    accumulated_time = 0.0;

    gui::manager* manager = nullptr;

#if defined(SDL_GUI)
    SDL_Renderer* renderer = nullptr;
#elif defined(GLSDL_GUI)
    SDL_Window*   window     = nullptr;
    SDL_GLContext gl_context = nullptr;
#elif defined(SFML_GUI)
    sf::RenderWindow* window = nullptr;
#elif defined(GLSFML_GUI)
    sf::Window* window = nullptr;
#endif
};

void main_loop(void* type_erased_data) {
#if defined(LXGUI_COMPILER_EMSCRIPTEN)
    try {
#endif

        main_loop_context& context = *reinterpret_cast<main_loop_context*>(type_erased_data);

        input::dispatcher& input_dispatcher = context.manager->get_input_dispatcher();

#if defined(SDL_GUI) || defined(GLSDL_GUI)
        // Get events from SDL
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
#    if defined(LXGUI_COMPILER_EMSCRIPTEN)
                    emscripten_cancel_main_loop();
                    return;
#    else
                    context.running = false;
                    return;
#    endif
                } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                    context.focus = false;
                else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                    context.focus = true;
            }

            // Feed events to the GUI.
            // NB: Do not use raw keyboard/mouse events from SDL directly. See below.
            static_cast<input::sdl::source&>(input_dispatcher.get_source()).on_sdl_event(event);
        }
#elif defined(SFML_GUI) || defined(GLSFML_GUI)
    // Get events from SFML
    sf::Event event{};
    while (context.window->pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
#    if defined(LXGUI_COMPILER_EMSCRIPTEN)
            emscripten_cancel_main_loop();
            return;
#    else
            context.running = false;
            return;
#    endif
        } else if (event.type == sf::Event::LostFocus)
            context.focus = false;
        else if (event.type == sf::Event::GainedFocus)
            context.focus = true;

        // Feed events to the GUI.
        // NB: Do not use raw keyboard/mouse events from SFML directly. See below.
        static_cast<input::sfml::source&>(input_dispatcher.get_source()).on_sfml_event(event);
    }
#endif

        if (!context.focus) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return;
        }

#if defined(GLSDL_GUI)
        SDL_GL_MakeCurrent(context.window, context.gl_context);
#endif

        // Update the gui
        timing_clock::time_point start = timing_clock::now();
        context.manager->update_ui(context.delta);

        // Clear the window
#if defined(GLSFML_GUI) || defined(GLSDL_GUI)
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
#elif defined(SDL_GUI)
    SDL_SetRenderDrawColor(context.renderer, 50, 50, 50, 255);
    SDL_RenderClear(context.renderer);
#elif defined(SFML_GUI)
    context.window->clear(sf::Color(51, 51, 51));
#endif

        // Render the gui
        context.manager->render_ui();

        // Display the window
#if defined(SDL_GUI)
        SDL_RenderPresent(context.renderer);
#elif defined(SFML_GUI) || defined(GLSFML_GUI)
    context.window->display();
#elif defined(GLSDL_GUI)
    SDL_GL_SwapWindow(context.window);
#endif
        timing_clock::time_point end = timing_clock::now();
        context.accumulated_time +=
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1e6;

        timing_clock::time_point current_time = timing_clock::now();
        context.delta =
            std::chrono::duration_cast<std::chrono::microseconds>(current_time - context.prev_time)
                .count() /
            1e6;
        context.prev_time = current_time;

        ++context.frame_count;

#if defined(LXGUI_COMPILER_EMSCRIPTEN)
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        emscripten_cancel_main_loop();
        return;
    } catch (...) {
        std::cout << "# Error #: Unhandled exception !" << std::endl;
        emscripten_cancel_main_loop();
        return;
    }
#endif
}

int main(int /*argc*/, char* /*argv*/[]) {
    auto* old_cout_buffer = std::cout.rdbuf();

    try {
        // -------------------------------------------------
        // Read test configuration
        // -------------------------------------------------

        std::size_t window_width  = 800u;
        std::size_t window_height = 600u;
        bool        full_screen   = false;
        float       scale_factor  = 1.0f;
        bool        print_to_log  = false;

        // Read some configuration data
        if (utils::file_exists("config.lua")) {
            sol::state lua;
            lua.do_file("config.lua");
            window_width  = lua["window_width"].get_or(std::size_t{800u});
            window_height = lua["window_height"].get_or(std::size_t{600u});
            full_screen   = lua["fullscreen"].get_or(false);
            scale_factor  = lua["scale_factor"].get_or(1.0);
            print_to_log  = lua["print_to_log"].get_or(false);
        }

#if defined(LXGUI_COMPILER_EMSCRIPTEN)
        // In WebAssembly builds, never print to a log file, because we don't have
        // disk write access. Just use the console.
        print_to_log = false;
#endif

        std::fstream log_cout("cout.txt", std::ios::out);
        std::fstream log_gui("gui.txt", std::ios::out);
        if (!print_to_log) {
            // Redirect output from the gui library to the standard output
            gui::out.rdbuf(std::cout.rdbuf());
        } else {
            // Redirect output from the standard output to a file
            log_cout.open("cout.txt", std::ios::out);
            std::cout.rdbuf(log_cout.rdbuf());

            // Redirect output from the gui library to a log file
            log_gui.open("gui.txt", std::ios::out);
            gui::out.rdbuf(log_gui.rdbuf());
        }

        // -------------------------------------------------
        // Create a window
        // -------------------------------------------------

        std::cout << "Creating window..." << std::endl;
        const std::string window_title = "test";

#if defined(GLSFML_GUI)
        sf::Window window;
#elif defined(SDL_GUI) || defined(GLSDL_GUI)
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw gui::exception(
                "SDL_Init", "Could not initialise SDL: " + std::string(SDL_GetError()) + ".");
        }

        std::uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
        if (full_screen)
            flags |= SDL_WINDOW_FULLSCREEN;

        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
            SDL_CreateWindow(
                window_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                window_width, window_height, flags),
            &SDL_DestroyWindow);

        if (!window)
            throw gui::exception("SDL_Window", "Could not create window.");

#    if defined(SDL_GUI)
        std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer(
            SDL_CreateRenderer(
                window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE),
            &SDL_DestroyRenderer);

        if (!renderer)
            throw gui::exception("SDL_Renderer", "Could not create renderer.");
#    else
        // Helper class to manage the OpenGL context from SDL
        struct GLContext {
            SDL_GLContext context = nullptr;

            explicit GLContext(SDL_Window* window) : context(SDL_GL_CreateContext(window)) {
                if (context == nullptr) {
                    throw gui::exception(
                        "SDL_GL_CreateContext", "Coult not create OpenGL context.");
                }
            }

            ~GLContext() {
                SDL_GL_DeleteContext(context);
            }
        };

        GLContext gl_context(window.get());
        SDL_GL_SetSwapInterval(0);
#    endif

#elif defined(SFML_GUI)
        sf::RenderWindow window;
#endif

#if defined(GLSFML_GUI) || defined(SFML_GUI)
        if (full_screen)
            window.create(
                sf::VideoMode(window_width, window_height, 32), window_title,
                sf::Style::Fullscreen);
        else
            window.create(sf::VideoMode(window_width, window_height, 32), window_title);
#endif

        // -------------------------------------------------
        // Initialise the GUI
        // -------------------------------------------------

        std::cout << "Creating gui manager..." << std::endl;
        utils::owner_ptr<gui::manager> manager;

#if defined(GLSFML_GUI) || defined(GLSDL_GUI)
        // Define the input manager
        std::unique_ptr<input::source> input_source;

#    if defined(GLSFML_GUI)
        // Use SFML
        input_source = std::make_unique<input::sfml::source>(window);
#    elif defined(GLSDL_GUI)
        // Use SDL
        {
            bool initialize_sdl_image = true;
            SDL_Renderer* renderer = nullptr; // set to nullptr when not using an SDL_Renderer
            input_source =
                std::make_unique<input::sdl::source>(window.get(), renderer, initialize_sdl_image);
        }
#    endif

        // Define the GUI renderer
        std::unique_ptr<gui::renderer> renderer =
            std::make_unique<gui::gl::renderer>(input_source->get_window_dimensions());

        manager = utils::make_owned<gui::manager>(
            // Provide the input source
            std::move(input_source),
            // Provide the GUI renderer implementation
            std::move(renderer));
#elif defined(SDL_GUI)
        // Use full SDL implementation
        manager            = gui::sdl::create_manager(window.get(), renderer.get());
#elif defined(SFML_GUI)
        // Use full SFML implementation
        manager        = gui::sfml::create_manager(window);
#endif

        // Automatically select best settings
        gui::renderer& gui_renderer = manager->get_renderer();
        gui_renderer.auto_detect_settings();

        std::cout << " Preferred languages: ";
        for (const auto& language : manager->get_localizer().get_preferred_languages())
            std::cout << language << ", ";
        std::cout << std::endl;
        std::size_t num_code_points = 0u;
        for (const auto& range : manager->get_localizer().get_allowed_code_points())
            num_code_points += static_cast<std::size_t>(range.last - range.first) + 1;
        std::cout << " Required Unicode code points: " << num_code_points << std::endl;
        std::cout << " Renderer settings:" << std::endl;
        std::cout << "  Renderer: " << gui_renderer.get_name() << std::endl;
        std::cout << "  Max texture size: " << gui_renderer.get_texture_max_size() << std::endl;
        std::cout << "  Vertex cache supported: " << gui_renderer.is_vertex_cache_supported()
                  << std::endl;
        std::cout << "  Vertex cache enabled: " << gui_renderer.is_vertex_cache_enabled()
                  << std::endl;
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

        manager->set_interface_scaling_factor(scale_factor);

        // Register the needed region types
        gui::factory& fac = manager->get_factory();
        fac.register_region_type<gui::texture>();
        fac.register_region_type<gui::font_string>();
        fac.register_region_type<gui::button>();
        fac.register_region_type<gui::slider>();
        fac.register_region_type<gui::edit_box>();
        fac.register_region_type<gui::scroll_frame>();
        fac.register_region_type<gui::status_bar>();

        // Load files:
        //  - first set the directory in which the interface is located,
        manager->add_addon_directory("interface");

        //  - then (optionally) set the directory where global text translations are located;
        //    you may not need this, as each addon will be parsed for its own translations,
        manager->add_localization_directory("locale");

        //  - register Lua "glues" (C++ functions and classes callable from Lua),
        manager->on_create_lua.connect([&](sol::state& lua) {
            // We use a lambda function because this code might be called
            // again later on, for example when one reloads the GUI (the
            // lua state is destroyed and created again).
            lua.set_function("get_folder_list", [](const std::string& dir) {
                return sol::as_table(utils::get_directory_list(dir));
            });
            lua.set_function("get_file_list", [](const std::string& dir) {
                return sol::as_table(utils::get_file_list(dir));
            });
        });

        //  - and load all files.
        std::cout << " Reading gui files..." << std::endl;
        manager->load_ui();

        // Create context for the main loop
        main_loop_context context;
        context.manager = manager.get();

#if defined(SDL_GUI)
        context.renderer = renderer.get();
#elif defined(GLSDL_GUI)
        context.window     = window.get();
        context.gl_context = gl_context.context;
#elif defined(SFML_GUI) || defined(GLSFML_GUI)
        context.window = &window;
#endif

        // -------------------------------------------------
        // Create GUI elements in C++
        // -------------------------------------------------

        // Create the Frame
        // A "root" frame has no parent and is directly owned by the gui::manager.
        // A "child" frame is owned by another frame.
        utils::observer_ptr<gui::frame> fps_frame;
        fps_frame = manager->get_root().create_root_frame<gui::frame>("FPSCounter");
        fps_frame->set_point(gui::point::top_left);
        fps_frame->set_point(
            gui::point::bottom_right, "FontstringTestFrameText", gui::point::top_right);

        // Create the FontString
        utils::observer_ptr<gui::font_string> fps_text;
        fps_text =
            fps_frame->create_layered_region<gui::font_string>(gui::layer::artwork, "$parentText");
        fps_text->set_point(gui::point::bottom_right, gui::vector2f(0, -5));
        fps_text->set_font("interface/fonts/main.ttf", 15);
        fps_text->set_alignment_y(gui::alignment_y::bottom);
        fps_text->set_alignment_x(gui::alignment_x::right);
        fps_text->set_outlined(true);
        fps_text->set_text_color(gui::color::red);
        fps_text->notify_loaded();

        // Create the scripts
        // In Lua
        /*fps_frame->add_script("OnLoad",
            "self.update_time = 0.5;"
            "self.timer = 1.0;"
            "self.frames = 0;",
            "main.cpp", 146 // You can provide the location of the Lua source, for error handling
        );
        fps_frame->add_script("OnUpdate",
            "self.timer = self.timer + arg1;"
            "self.frames = self.frames + 1;"

            "if (self.timer > self.update_time) then"
            "    local fps = self.frames/self.timer;"
            "    self.Text:set_text(\"FPS: \"..math.floor(fps));"

            "    self.timer = 0.0;"
            "    self.frames = 0;"
            "end",
            "main.cpp", 152 // You can provide the location of the Lua source, for error handling
        );*/

        // Or in C++:

        float timer = 1.0f;
        fps_frame->add_script(
            "OnUpdate", [timer, &context](gui::frame& self, const gui::event_data& data) mutable {
                float delta = data.get<float>(0);
                timer += delta;

                if (timer > 0.5f) {
                    float frame_time = 1e6 * context.accumulated_time / context.frame_count;

                    if (auto txt = self.get_region<gui::font_string>("Text")) {
                        txt->set_text(
                            U"(created in C++)\nFrame time (us): " +
                            utils::to_ustring(std::round(frame_time)));
                    }

                    timer                    = 0.0f;
                    context.frame_count      = 0;
                    context.accumulated_time = 0;
                }
            });

        // Tell the Frame is has been fully loaded, and call "OnLoad"
        fps_frame->notify_loaded();

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
        //    elements. Usage: manager->get_input_dispatcher().
        //  - input::source: simple raw events, with no processing or filtering applied.
        //    Use this if you need the raw inputs. Usage:
        //    Usage: manager->get_input_dispatcher().get_source().
        input::world_dispatcher& world_input_dispatcher = manager->get_world_input_dispatcher();
        world_input_dispatcher.on_key_pressed.connect([&](input::key key_code) {
            // Process keyboard inputs for the game...
            switch (key_code) {
            case input::key::k_escape: {
#if defined(LXGUI_COMPILER_EMSCRIPTEN)
                emscripten_cancel_main_loop();
                return;
#else
                context.running = false;
                return;
#endif
            }
            case input::key::k_p: gui::out << context.manager->print_ui() << std::endl; break;
            case input::key::k_k: gui::out << "###" << std::endl; break;
            case input::key::k_c: context.manager->get_root().toggle_caching(); break;
            case input::key::k_r: context.manager->reload_ui(); break;
            case input::key::k_b:
                context.manager->get_renderer().set_quad_batching_enabled(
                    !context.manager->get_renderer().is_quad_batching_enabled());
                break;
            case input::key::k_a:
                context.manager->get_renderer().set_texture_atlas_enabled(
                    !context.manager->get_renderer().is_texture_atlas_enabled());
                break;
            default: break;
            }
        });

        world_input_dispatcher.on_mouse_pressed.connect(
            [&](input::mouse_button /*button_code*/, const gui::vector2f& /*mouse_pos*/) {
                // Process mouse inputs for the game...
            });

        // -------------------------------------------------
        // Start the main loop
        // -------------------------------------------------

        context.prev_time = timing_clock::now();

        std::cout << "Entering loop..." << std::endl;

#if defined(LXGUI_COMPILER_EMSCRIPTEN)
        emscripten_set_main_loop_arg(main_loop, &context, -1, 1);
#else
        while (context.running) {
            main_loop(&context);
        }
#endif

        std::cout << "End of loop." << std::endl;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "# Error #: Unhandled exception !" << std::endl;
        return 1;
    }

    std::cout << "End of program." << std::endl;
    std::cout.rdbuf(old_cout_buffer);

    return 0;
}
