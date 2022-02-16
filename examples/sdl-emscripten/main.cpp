#include "examples_common.hpp"

#include <emscripten.h>
#include <iostream>
#include <lxgui/gui_event.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/impl/gui_sdl.hpp>
#include <lxgui/impl/input_sdl_source.hpp>
#include <lxgui/input_dispatcher.hpp>
#include <lxgui/input_world_dispatcher.hpp>
#include <lxgui/lxgui.hpp>
#include <thread>
#define SDL_MAIN_HANDLED
#include <SDL.h>

using namespace lxgui;

// Emscripten needs us to specify a main loop function.
// This structure will hold the data we need from main().
struct main_loop_context {
    bool                     focus = true;
    float                    delta = 0.1f;
    timing_clock::time_point prev_time;

    gui::manager* manager  = nullptr;
    SDL_Renderer* renderer = nullptr;
};

void main_loop(void* pTypeErasedData) try {
    main_loop_context& context          = *reinterpret_cast<main_loop_context*>(pTypeErasedData);
    input::dispatcher& input_dispatcher = context.manager->get_input_dispatcher();

    // Get events from SDL
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_WINDOWEVENT) {
            if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                emscripten_cancel_main_loop();
                return;
            } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                context.focus = false;
            else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                context.focus = true;
        }

        // Feed events to the GUI.
        // NB: Do not use raw keyboard/mouse events from SDL directly. See below.
        static_cast<input::sdl::source&>(input_dispatcher.get_source()).on_sdl_event(event);
    }

    // If the window is not focussed, do nothing and wait until focus comes back
    if (!context.focus) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return;
    }

    // Update the gui
    context.manager->update_ui(context.delta);

    // Your own rendering would go here!
    // For this example, we just clear the window
    SDL_SetRenderDrawColor(context.renderer, 50, 50, 50, 255);
    SDL_RenderClear(context.renderer);

    // Render the gui on top of the world
    context.manager->render_ui();

    // Display the window
    SDL_RenderPresent(context.renderer);

    // Compute time delta since last frame
    timing_clock::time_point current_time = timing_clock::now();
    context.delta                         = get_time_delta(context.prev_time, current_time);
    context.prev_time                     = current_time;
} catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    emscripten_cancel_main_loop();
    return;
} catch (...) {
    std::cout << "# Error # : Unhandled exception !" << std::endl;
    emscripten_cancel_main_loop();
    return;
}

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

        // Create SDL renderer
        std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer(
            SDL_CreateRenderer(
                window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE),
            &SDL_DestroyRenderer);

        if (!renderer)
            throw gui::exception("SDL_Renderer", "Could not create renderer.");

        SDL_RendererInfo renderer_info;
        SDL_GetRendererInfo(renderer.get(), &renderer_info);
        gui::out << "SDL renderer: " << renderer_info.name << std::endl;

        // Initialize the gui
        std::cout << "Creating gui manager..." << std::endl;

        // Use full SDL implementation
        utils::owner_ptr<gui::manager> manager =
            gui::sdl::create_manager(window.get(), renderer.get());

        // Setup the GUI (see examples_common.cpp)
        examples_setup_gui(*manager);

        // Start the main loop
        main_loop_context context;
        context.prev_time = timing_clock::now();
        context.manager   = manager.get();
        context.renderer  = renderer.get();

        // Register a callback on Escape to terminate the program.
        // Doing it this way, we only react to keyboard input that is not captured by the GUI.
        input::world_dispatcher& world_input_dispatcher = manager->get_world_input_dispatcher();
        world_input_dispatcher.on_key_pressed.connect([&](input::key mKey) {
            if (mKey == input::key::k_escape)
                emscripten_cancel_main_loop();
        });

        std::cout << "Entering loop..." << std::endl;

        emscripten_set_main_loop_arg(main_loop, &context, -1, 1);

        std::cout << "End of loop." << std::endl;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "# Error # : Unhandled exception !" << std::endl;
        return 1;
    }

    std::cout << "End of program." << std::endl;

    return 0;
}
