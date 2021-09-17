#include <lxgui/lxgui.hpp>
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

#include <lxgui/impl/gui_sdl.hpp>
#include <lxgui/impl/input_sdl_source.hpp>
#define SDL_MAIN_HANDLED
#include <SDL.h>

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
        const uint uiWindowWidth  = 800;
        const uint uiWindowHeight = 600;

        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            throw gui::exception("SDL_Init", "Could not initialise SDL: "+
                std::string(SDL_GetError())+".");
        }

        const std::uint32_t uiFlags =
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;

        std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> pWindow(
            SDL_CreateWindow(
                sWindowTitle.c_str(),
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                uiWindowWidth,
                uiWindowHeight,
                uiFlags
            ),
            &SDL_DestroyWindow
        );

        if (!pWindow)
            throw gui::exception("SDL_Window", "Could not create window.");

        // Create SDL renderer
        std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> pRenderer(
            SDL_CreateRenderer(
                pWindow.get(), -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE
            ),
            &SDL_DestroyRenderer
        );

        if (!pRenderer)
            throw gui::exception("SDL_Renderer", "Could not create renderer.");

        SDL_RendererInfo mRendererInfo;
        SDL_GetRendererInfo(pRenderer.get(), &mRendererInfo);
        gui::out << "SDL renderer: " << mRendererInfo.name << std::endl;

        // Initialize the gui
        std::cout << "Creating gui manager..." << std::endl;

        // Use full SDL implementation
        std::unique_ptr<gui::manager> pManager =
            gui::sdl::create_manager(pWindow.get(), pRenderer.get());

        pManager->enable_caching(false);

        // Setup the GUI (see examples_common.cpp)
        examples_setup_gui(*pManager);

        // Start the main loop
        bool bRunning = true;
        bool bFocus = true;
        double fDelta = 0.0;
        timing_clock::time_point mPrevTime = timing_clock::now();
        input::manager* pInputMgr = pManager->get_input_manager();

        std::cout << "Entering loop..." << std::endl;

        while (bRunning)
        {
            // Get events from SDL
            SDL_Event mEvent;
            while (SDL_PollEvent(&mEvent))
            {
                if (mEvent.type == SDL_WINDOWEVENT)
                {
                    if (mEvent.window.event == SDL_WINDOWEVENT_CLOSE)
                        bRunning = false;
                    else if (mEvent.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                        bFocus = false;
                    else if (mEvent.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                        bFocus = true;
                }
                else if (mEvent.type == SDL_KEYUP)
                {
                    // This uses events straight from SDL, but the GUI may want to
                    // capture some of them (for example: the user is typing in an edit_box).
                    // Therefore, before we can react to these events, we must check that
                    // the input isn't being "focussed":
                    if (!pInputMgr->is_keyboard_focused())
                    {
                        switch (mEvent.key.keysym.sym)
                        {
                            case SDLK_ESCAPE:
                                // Escape pressed: stop the program
                                bRunning = false;
                                break;
                            default:
                                break;
                        }
                    }
                }

                // Feed events to the GUI
                static_cast<input::sdl::source*>(pInputMgr->get_source())->on_sdl_event(mEvent);
            }

            // Check if WORLD input is allowed
            if (pInputMgr->can_receive_input("WORLD"))
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
            SDL_SetRenderDrawColor(pRenderer.get(), 50, 50, 50, 255);
            SDL_RenderClear(pRenderer.get());

            // Render the gui on top of the world
            pManager->render_ui();

            // Display the window
            SDL_RenderPresent(pRenderer.get());

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
