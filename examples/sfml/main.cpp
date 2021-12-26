#include <lxgui/gui_event.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/input.hpp>

#if defined(LXGUI_PLATFORM_WINDOWS)
    #define NOMINMAX
    #include <windows.h>
    #if defined(LXGUI_COMPILER_MSVC)
        #pragma comment(linker, "/entry:mainCRTStartup")
    #endif
#endif

#include <thread>
#include <iostream>

#include "examples_common.hpp"

#include <lxgui/impl/gui_sfml.hpp>
#include <lxgui/impl/input_sfml_source.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

using namespace lxgui;

int main(int argc, char* argv[])
{
    try
    {
        // Redirect output from the gui library to the standard output.
        // You can redirect it to a file, or your own logger, etc.
        gui::out.rdbuf(std::cout.rdbuf());

        // Create a window
        std::cout << "Creating window..." << std::endl;
        const std::string sWindowTitle = "test";
        const std::size_t uiWindowWidth  = 800u;
        const std::size_t uiWindowHeight = 600u;

        sf::RenderWindow mWindow(sf::VideoMode(uiWindowWidth, uiWindowHeight, 32), sWindowTitle);

        // Initialize the gui
        std::cout << "Creating gui manager..." << std::endl;

        // Use full SFML implementation
        utils::owner_ptr<gui::manager> pManager = gui::sfml::create_manager(mWindow);
        pManager->enable_caching(false);

        // Setup the GUI (see examples_common.cpp)
        examples_setup_gui(*pManager);

        // Start the main loop
        bool bRunning = true;
        bool bFocus = true;
        float fDelta = 0.0f;
        timing_clock::time_point mPrevTime = timing_clock::now();
        input::manager& mInputMgr = pManager->get_input_manager();

        std::cout << "Entering loop..." << std::endl;

        while (bRunning)
        {
            // Get events from SFML
            sf::Event mEvent;
            while (mWindow.pollEvent(mEvent))
            {
                if (mEvent.type == sf::Event::Closed)
                    bRunning = false;
                else if (mEvent.type == sf::Event::LostFocus)
                    bFocus = false;
                else if (mEvent.type == sf::Event::GainedFocus)
                    bFocus = true;
                else if (mEvent.type == sf::Event::KeyReleased)
                {
                    // This uses events straight from SFML, but the GUI may want to
                    // capture some of them (for example: the user is typing in an edit_box).
                    // Therefore, before we can react to these events, we must check that
                    // the input isn't being "focussed" into the GUI:
                    if (!mInputMgr.is_keyboard_focused())
                    {
                        switch (mEvent.key.code)
                        {
                            case sf::Keyboard::Key::Escape:
                                // Escape pressed: stop the program
                                bRunning = false;
                                break;
                            default:
                                break;
                        }
                    }
                }

                static_cast<input::sfml::source&>(mInputMgr.get_source()).on_sfml_event(mEvent);
            }

            // Check if WORLD input is allowed
            if (mInputMgr.can_receive_input("WORLD"))
            {
                // Process mouse and click events in the game...
            }

            // If the window is not focussed, do nothing and wait until focus comes back
            if (!bFocus)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            // Update the gui
            pManager->update(fDelta);

            // Your own rendering would go here!
            // For this example, we just clear the window
            mWindow.clear(sf::Color(51,51,51));

            // Render the gui on top of the world
            pManager->render_ui();

            // Display the window
            mWindow.display();

            // Compute time delta since last frame
            timing_clock::time_point mCurrentTime = timing_clock::now();
            fDelta = get_time_delta(mPrevTime, mCurrentTime);
            mPrevTime = mCurrentTime;
        }

        std::cout << "End of loop." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cout << "# Error # : Unhandled exception !" << std::endl;
        return 1;
    }

    std::cout << "End of program." << std::endl;

    return 0;
}
