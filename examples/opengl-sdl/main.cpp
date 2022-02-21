#include <lxgui/gui_event.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/input_dispatcher.hpp>
#include <lxgui/input_world_dispatcher.hpp>
#include <lxgui/lxgui.hpp>

#if defined(LXGUI_PLATFORM_WINDOWS)
#    define NOMINMAX
#    include <windows.h>
#    if defined(LXGUI_COMPILER_MSVC)
#        pragma comment(linker, "/entry:mainCRTStartup")
#    endif
#endif

#include "examples_common.hpp"

#include <iostream>
#include <lxgui/impl/gui_gl_renderer.hpp>
#include <lxgui/impl/input_sdl_source.hpp>
#include <thread>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#if defined(LXGUI_PLATFORM_OSX)
#    include <OpenGL/gl.h>
#else
#    include <GL/gl.h>
#endif

using namespace lxgui;

// Helper class to manage the OpenGL context from SDL
struct GLContext {
    SDL_GLContext context = nullptr;

    explicit GLContext(SDL_Window* window) : context(SDL_GL_CreateContext(window)) {
        if (context == nullptr)
            throw gui::exception("SDL_GL_CreateContext", "Could not create OpenGL context.");
    }

    ~GLContext() noexcept {
        SDL_GL_DeleteContext(context);
    }
};

int main(int argc, char* argv[]) {
    try {
        // Redirect output from the gui library to the standard output.
        // You can redirect it to a file, or your own logger, etc.
        gui::out.rdbuf(std::cout.rdbuf());

        // Create a window
        std::cout << "Creating window..." << std::endl;
        const std::string window_title  = "test";
        const std::size_t window_width  = 800u;
        const std::size_t window_height = 600u;

        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            throw gui::exception(
                "SDL_Init", "Could not initialise SDL: " + std::string(SDL_GetError()) + ".");
        }

        const std::uint32_t flags =
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;

        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
            SDL_CreateWindow(
                window_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                window_width, window_height, flags),
            &SDL_DestroyWindow);

        if (!window)
            throw gui::exception("SDL_Window", "Could not create window.");

        // Create OpenGL context
        GLContext gl_context(window.get());
        SDL_GL_SetSwapInterval(0);

        // Initialize the gui
        std::cout << "Creating gui manager..." << std::endl;

        // Define the input manager
        std::unique_ptr<input::source> input_source;
        {
            bool          initialise_sdl_image = true;
            SDL_Renderer* renderer             = nullptr;
            input_source =
                std::make_unique<input::sdl::source>(window.get(), renderer, initialise_sdl_image);
        }

        // Define the GUI renderer
        std::unique_ptr<gui::renderer> renderer =
            std::make_unique<gui::gl::renderer>(input_source->get_window_dimensions());

        // Create the GUI manager
        utils::owner_ptr<gui::manager> manager = utils::make_owned<gui::manager>(
            // Provide the input source
            std::move(input_source),
            // Provide the GUI renderer implementation
            std::move(renderer));

        // Setup the GUI (see examples_common.cpp)
        examples_setup_gui(*manager);

        // Start the main loop
        bool                     running                = true;
        bool                     focus                  = true;
        float                    delta                  = 0.0f;
        timing_clock::time_point prev_time              = timing_clock::now();
        input::dispatcher&       input_dispatcher       = manager->get_input_dispatcher();
        input::world_dispatcher& world_input_dispatcher = manager->get_world_input_dispatcher();

        // Register a callback on Escape to terminate the program.
        // Doing it this way, we only react to keyboard input that is not captured by the GUI.
        world_input_dispatcher.on_key_pressed.connect([&](input::key key_id) {
            if (key_id == input::key::k_escape)
                running = false;
        });

        std::cout << "Entering loop..." << std::endl;

        while (running) {
            // Get events from SDL
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_WINDOWEVENT) {
                    if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                        running = false;
                    else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                        focus = false;
                    else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                        focus = true;
                }

                // Feed events to the GUI.
                // NB: Do not use raw keyboard/mouse events from SDL directly. See below.
                static_cast<input::sdl::source&>(input_dispatcher.get_source()).on_sdl_event(event);
            }

            // If the window is not focused, do nothing and wait until focus comes back
            if (!focus) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            // Update the gui
            SDL_GL_MakeCurrent(window.get(), gl_context.context);
            manager->update_ui(delta);

            // Your own rendering would go here!
            // For this example, we just clear the window
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Render the gui on top of the world
            manager->render_ui();

            // Display the window
            SDL_GL_SwapWindow(window.get());

            // Compute time delta since last frame
            timing_clock::time_point current_time = timing_clock::now();
            delta                                 = get_time_delta(prev_time, current_time);
            prev_time                             = current_time;
        }

        std::cout << "End of loop." << std::endl;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "# Error #: Unhandled exception !" << std::endl;
        return 1;
    }

    std::cout << "End of program." << std::endl;

    return 0;
}
