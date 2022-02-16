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

#include <SFML/Window.hpp>
#include <iostream>
#include <lxgui/impl/gui_gl_renderer.hpp>
#include <lxgui/impl/input_sfml_source.hpp>
#include <thread>
#if defined(LXGUI_PLATFORM_OSX)
#    include <OpenGL/gl.h>
#else
#    include <GL/gl.h>
#endif

using namespace lxgui;

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

        sf::Window window(sf::VideoMode(window_width, window_height, 32), window_title);

        // Initialize the gui
        std::cout << "Creating gui manager..." << std::endl;

        // Define the input manager
        std::unique_ptr<input::source> input_source = std::make_unique<input::sfml::source>(window);

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
        world_input_dispatcher.on_key_pressed.connect([&](input::key mKey) {
            if (mKey == input::key::k_escape)
                running = false;
        });

        std::cout << "Entering loop..." << std::endl;

        while (running) {
            // Get events from SFML
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    running = false;
                else if (event.type == sf::Event::LostFocus)
                    focus = false;
                else if (event.type == sf::Event::GainedFocus)
                    focus = true;

                // Feed events to the GUI.
                // NB: Do not use raw keyboard/mouse events from SFML directly. See
                // examples_common.cpp.
                static_cast<input::sfml::source&>(input_dispatcher.get_source())
                    .on_sfml_event(event);
            }

            // If the window is not focussed, do nothing and wait until focus comes back
            if (!focus) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            // Update the gui
            manager->update_ui(delta);

            // Your own rendering would go here!
            // For this example, we just clear the window
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Render the gui on top of the world
            manager->render_ui();

            // Display the window
            window.display();

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
        std::cout << "# Error # : Unhandled exception !" << std::endl;
        return 1;
    }

    std::cout << "End of program." << std::endl;

    return 0;
}
