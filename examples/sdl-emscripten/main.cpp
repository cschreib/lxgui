#include <lxgui/lxgui.hpp>
#include <lxgui/gui_event.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/input_dispatcher.hpp>
#include <lxgui/input_world_dispatcher.hpp>

#include <emscripten.h>

#include <thread>
#include <iostream>

#include "examples_common.hpp"

#include <lxgui/impl/gui_sdl.hpp>
#include <lxgui/impl/input_sdl_source.hpp>
#define SDL_MAIN_HANDLED
#include <SDL.h>

using namespace lxgui;

// Emscripten needs us to specify a main loop function.
// This structure will hold the data we need from main().
struct main_loop_context
{
    bool bFocus = true;
    float fDelta = 0.1f;
    timing_clock::time_point mPrevTime;
    std::size_t uiFrameCount = 0u;

    gui::manager* pManager = nullptr;
    SDL_Renderer* pRenderer = nullptr;
};

void main_loop(void* pTypeErasedData)
try
{
    main_loop_context& mContext = *reinterpret_cast<main_loop_context*>(pTypeErasedData);
    input::dispatcher& mInputDispatcher = mContext.pManager->get_input_dispatcher();

    // Get events from SDL
    SDL_Event mEvent;
    while (SDL_PollEvent(&mEvent))
    {
        if (mEvent.type == SDL_WINDOWEVENT)
        {
            if (mEvent.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                emscripten_cancel_main_loop();
                return;
            }
            else if (mEvent.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                mContext.bFocus = false;
            else if (mEvent.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                mContext.bFocus = true;
        }

        // Feed events to the GUI.
        // NB: Do not use raw keyboard/mouse events from SDL directly. See below.
        static_cast<input::sdl::source&>(mInputDispatcher.get_source()).on_sdl_event(mEvent);
    }

    // If the window is not focussed, do nothing and wait until focus comes back
    if (!mContext.bFocus)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return;
    }

    // Update the gui
    mContext.pManager->update_ui(mContext.fDelta);

    // Your own rendering would go here!
    // For this example, we just clear the window
    SDL_SetRenderDrawColor(mContext.pRenderer, 50, 50, 50, 255);
    SDL_RenderClear(mContext.pRenderer);

    // Render the gui on top of the world
    mContext.pManager->render_ui();

    // Display the window
    SDL_RenderPresent(mContext.pRenderer);

    // Compute time delta since last frame
    timing_clock::time_point mCurrentTime = timing_clock::now();
    mContext.fDelta = get_time_delta(mContext.mPrevTime, mCurrentTime);
    mContext.mPrevTime = mCurrentTime;
}
catch (const std::exception& e)
{
    std::cout << e.what() << std::endl;
    emscripten_cancel_main_loop();
    return;
}
catch (...)
{
    std::cout << "# Error # : Unhandled exception !" << std::endl;
    emscripten_cancel_main_loop();
    return;
}

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
        utils::owner_ptr<gui::manager> pManager =
            gui::sdl::create_manager(pWindow.get(), pRenderer.get());

        // Setup the GUI (see examples_common.cpp)
        examples_setup_gui(*pManager);

        // Start the main loop
        main_loop_context mContext;
        mContext.mPrevTime = timing_clock::now();
        mContext.pManager = pManager.get();
        mContext.pRenderer = pRenderer.get();

        // Register a callback on Escape to terminate the program.
        // Doing it this way, we only react to keyboard input that is not captured by the GUI.
        input::world_dispatcher& mWorldInputDispatcher = pManager->get_world_input_dispatcher();
        mWorldInputDispatcher.on_key_pressed.connect([&](input::key mKey)
        {
            if (mKey == input::key::K_ESCAPE)
                emscripten_cancel_main_loop();
        });

        std::cout << "Entering loop..." << std::endl;

        emscripten_set_main_loop_arg(main_loop, &mContext, -1, 1);

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
