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

#include <lxgui/impl/gui_gl_renderer.hpp>
#include <lxgui/impl/input_sdl_source.hpp>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#if defined(LXGUI_PLATFORM_OSX)
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

using namespace lxgui;

// Helper class to manage the OpenGL context from SDL
struct GLContext
{
    SDL_GLContext pContext = nullptr;

    explicit GLContext(SDL_Window* pWindow) : pContext(SDL_GL_CreateContext(pWindow))
    {
        if (pContext == nullptr)
            throw gui::exception("SDL_GL_CreateContext", "Coult not create OpenGL context.");
    }

    ~GLContext() noexcept { SDL_GL_DeleteContext(pContext); }
};

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

        // Create OpenGL context
        GLContext mGLContext(pWindow.get());
        SDL_GL_SetSwapInterval(0);

        // Initialize the gui
        std::cout << "Creating gui manager..." << std::endl;

        // Define the input manager
        std::unique_ptr<input::source> pInputSource;
        {
            bool bInitialiseSDLImage = true;
            SDL_Renderer* pRenderer = nullptr;
            pInputSource = std::make_unique<input::sdl::source>(
                pWindow.get(), pRenderer, bInitialiseSDLImage);
        }

        // Define the GUI renderer
        std::unique_ptr<gui::renderer> pRenderer = std::make_unique<gui::gl::renderer>(
            pInputSource->get_window_dimensions());

        // Create the GUI manager
        utils::owner_ptr<gui::manager> pManager = utils::make_owned<gui::manager>(
            // Provide the input source
            std::move(pInputSource),
            // Provide the GUI renderer implementation
            std::move(pRenderer)
        );

        // Setup the GUI (see examples_common.cpp)
        examples_setup_gui(*pManager);

        // Start the main loop
        bool bRunning = true;
        bool bFocus = true;
        double fDelta = 0.0;
        timing_clock::time_point mPrevTime = timing_clock::now();
        input::manager& mInputMgr = pManager->get_input_manager();

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
                    if (!mInputMgr.is_keyboard_focused())
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
                static_cast<input::sdl::source&>(mInputMgr.get_source()).on_sdl_event(mEvent);
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
            SDL_GL_MakeCurrent(pWindow.get(), mGLContext.pContext);
            pManager->update_ui(fDelta);

            // Your own rendering would go here!
            // For this example, we just clear the window
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Render the gui on top of the world
            pManager->render_ui();

            // Display the window
            SDL_GL_SwapWindow(pWindow.get());

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
