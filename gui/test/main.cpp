#include <lxgui/gui_manager.hpp>
#include <lxgui/gui_texture.hpp>
#include <lxgui/gui_fontstring.hpp>
#include <lxgui/gui_button.hpp>
#include <lxgui/gui_slider.hpp>
#include <lxgui/gui_editbox.hpp>
#include <lxgui/gui_scrollframe.hpp>
#include <lxgui/gui_statusbar.hpp>
#include <lxgui/gui_event.hpp>
#include <lxgui/gui_out.hpp>
#include <lxgui/input.hpp>
#include <lxgui/luapp_state.hpp>
#include <lxgui/luapp_function.hpp>
#include <lxgui/utils_filesystem.hpp>
#include <lxgui/utils_string.hpp>

#include <sol/state.hpp>

//#define GLSFML_GUI
//#define GLSDL_GUI
//#define SDL_GUI
//#define SFML_GUI

#if defined(GLSFML_GUI)
    // OpenGL + SFML input
    #include <lxgui/impl/gui_gl_renderer.hpp>
    #include <lxgui/impl/input_sfml_source.hpp>
    #include <SFML/Window.hpp>
    #if defined(MACOSX)
        #include <OpenGL/gl.h>
    #else
        #include <GL/gl.h>
    #endif
#elif defined(GLSDL_GUI)
    // OpenGL + SDL input
    #include <lxgui/impl/gui_gl_renderer.hpp>
    #include <lxgui/impl/input_sdl_source.hpp>
    #define SDL_MAIN_HANDLED
    #include <SDL.h>
    #if defined(MACOSX)
        #include <OpenGL/gl.h>
    #else
        #include <GL/gl.h>
    #endif
#elif defined(SDL_GUI)
    // SDL
    #include <lxgui/impl/gui_sdl.hpp>
    #include <lxgui/impl/input_sdl_source.hpp>
    #define SDL_MAIN_HANDLED
    #include <SDL.h>
#elif defined(SFML_GUI)
    // SFML
    #include <lxgui/impl/gui_sfml.hpp>
    #include <lxgui/impl/input_sfml_source.hpp>
    #include <SFML/Window.hpp>
    #include <SFML/Graphics/RenderWindow.hpp>
#endif


#if defined(WIN32)
    #include <windows.h>
    #if defined(MSVC)
        #pragma comment(linker, "/entry:mainCRTStartup")
    #endif
#elif defined(WASM)
    #include <emscripten.h>
#endif

#include <fstream>
#include <chrono>
#include <cstdint>
#include <thread>

using namespace lxgui;
using timing_clock = std::chrono::high_resolution_clock;

struct main_loop_context
{
    bool bRunning = true;
    bool bFocus = true;
    float fDelta = 0.1f;
    timing_clock::time_point mPrevTime;
    timing_clock::time_point mFirstTime;
    uint uiFrameCount = 0;

    gui::manager* pManager = nullptr;

#if defined(SDL_GUI)
    SDL_Renderer* pRenderer = nullptr;
#elif defined(GLSDL_GUI)
    SDL_Window* pWindow = nullptr;
    SDL_GLContext pGLContext = nullptr;
#elif defined(SFML_GUI)
    sf::RenderWindow* pWindow = nullptr;
#elif defined(GLSFML_GUI)
    sf::Window* pWindow = nullptr;
#endif
};

void main_loop(void* pTypeErasedData)
{
#if defined(WASM)
    try
    {
#endif

    main_loop_context& mContext = *reinterpret_cast<main_loop_context*>(pTypeErasedData);

    input::manager* pInputMgr = mContext.pManager->get_input_manager();

#if defined(SDL_GUI) || defined(GLSDL_GUI)
    // Get events from SDL
    SDL_Event mEvent;
    while (SDL_PollEvent(&mEvent))
    {
        if (mEvent.type == SDL_WINDOWEVENT)
        {
            if (mEvent.window.event == SDL_WINDOWEVENT_CLOSE)
            {
            #if defined(WASM)
                emscripten_cancel_main_loop();
                return;
            #else
                mContext.bRunning = false;
                return;
            #endif
            }
            else if (mEvent.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                mContext.bFocus = false;
            else if (mEvent.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                mContext.bFocus = true;
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
                    {
                    #if defined(WASM)
                        emscripten_cancel_main_loop();
                        return;
                    #else
                        mContext.bRunning = false;
                        return;
                    #endif
                    }
                    case SDLK_p:
                        gui::out << mContext.pManager->print_ui() << std::endl;
                        break;
                    case SDLK_k:
                        gui::out << "###" << std::endl;
                        break;
                    case SDLK_c:
                        mContext.pManager->enable_caching(!mContext.pManager->is_caching_enabled());
                        break;
                    case SDLK_r:
                        mContext.pManager->reload_ui();
                        break;
                    default:
                        break;
                }
            }
        }

        // Feed events to the GUI
        static_cast<input::sdl::source*>(pInputMgr->get_source())->on_sdl_event(mEvent);
    }
#elif defined(SFML_GUI) || defined(GLSFML_GUI)
    // Get events from SFML
    sf::Event mEvent;
    while (mContext.pWindow->pollEvent(mEvent))
    {
        if (mEvent.type == sf::Event::Closed)
        {
            #if defined(WASM)
                emscripten_cancel_main_loop();
                return;
            #else
                mContext.bRunning = false;
                return;
            #endif
        }
        else if (mEvent.type == sf::Event::LostFocus)
            mContext.bFocus = false;
        else if (mEvent.type == sf::Event::GainedFocus)
            mContext.bFocus = true;
        else if (mEvent.type == sf::Event::KeyReleased)
        {
            // This uses events straight from SFML, but the GUI may want to
            // capture some of them (for example: the user is typing in an edit_box).
            // Therefore, before we can react to these events, we must check that
            // the input isn't being "focussed":
            if (!pInputMgr->is_keyboard_focused())
            {
                switch (mEvent.key.code)
                {
                    case sf::Keyboard::Key::Escape:
                    {
                    #if defined(WASM)
                        emscripten_cancel_main_loop();
                        return;
                    #else
                        mContext.bRunning = false;
                        return;
                    #endif
                    }
                    case sf::Keyboard::Key::P:
                        gui::out << mContext.pManager->print_ui() << std::endl;
                        break;
                    case sf::Keyboard::Key::K:
                        gui::out << "###" << std::endl;
                        break;
                    case sf::Keyboard::Key::C:
                        mContext.pManager->enable_caching(!mContext.pManager->is_caching_enabled());
                        break;
                    case sf::Keyboard::Key::R:
                        mContext.pManager->reload_ui();
                        break;
                    default:
                        break;
                }
            }
        }

        static_cast<input::sfml::source*>(pInputMgr->get_source())->on_sfml_event(mEvent);
    }
#endif

    // Check if WORLD input is allowed
    if (pInputMgr->can_receive_input("WORLD"))
    {
        // Process mouse and click events in the game...
    }

    if (!mContext.bFocus)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return;
    }

#if defined(GLSDL_GUI)
    SDL_GL_MakeCurrent(mContext.pWindow, mContext.pGLContext);
#endif

    // Update the gui
    mContext.pManager->update(mContext.fDelta);

    // Clear the window
#if defined(GLSFML_GUI) || defined(GLSDL_GUI)
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
#elif defined(SDL_GUI)
    SDL_SetRenderDrawColor(mContext.pRenderer, 50, 50, 50, 255);
    SDL_RenderClear(mContext.pRenderer);
#elif defined(SFML_GUI)
    mContext.pWindow->clear(sf::Color(51,51,51));
#endif

    // Render the gui
    mContext.pManager->render_ui();

    // Display the window
#if defined(SDL_GUI)
    SDL_RenderPresent(mContext.pRenderer);
#elif defined(SFML_GUI) || defined(GLSFML_GUI)
    mContext.pWindow->display();
#elif defined(GLSDL_GUI)
    SDL_GL_SwapWindow(mContext.pWindow);
#endif

    timing_clock::time_point mCurrentTime = timing_clock::now();
    mContext.fDelta = std::chrono::duration_cast<std::chrono::microseconds>(
        mCurrentTime - mContext.mPrevTime).count() / 1e6;
    mContext.mPrevTime = mCurrentTime;

    ++mContext.uiFrameCount;

#if defined(WASM)
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
#endif
}

int main(int argc, char* argv[])
{
#if !defined(WASM)
    // Redirect output from the standard output to a file
    std::fstream mLogCout("cout.txt", std::ios::out);
    auto* pOldBuffer = std::cout.rdbuf();
    std::cout.rdbuf(mLogCout.rdbuf());
#endif

    try
    {
        uint uiWindowWidth  = 800;
        uint uiWindowHeight = 600;
        bool bFullScreen    = false;
        std::string sLocale = "enGB";
        float fScaleFactor  = 1.0f;

        // Read some configuration data
        if (utils::file_exists("config.lua"))
        {
            lua::state mLua;
            mLua.do_file("config.lua");
            uiWindowWidth  = mLua.get_global_int("window_width",    false, 800);
            uiWindowHeight = mLua.get_global_int("window_height",   false, 600);
            bFullScreen    = mLua.get_global_bool("fullscreen",     false, false);
            sLocale        = mLua.get_global_string("locale",       false, "enGB");
            fScaleFactor   = mLua.get_global_double("scale_factor", false, 1.0);
        }

    #if defined(WASM)
        // Redirect output from the gui library to the standard output
        gui::out.rdbuf(std::cout.rdbuf());
    #else
        // Redirect output from the gui library to a log file
        std::fstream mGUI("gui.txt", std::ios::out);
        gui::out.rdbuf(mGUI.rdbuf());
    #endif

        // Create a window
        std::cout << "Creating window..." << std::endl;
        const std::string sWindowTitle = "test";

    #if defined(GLSFML_GUI)
        sf::Window mWindow;
    #elif defined(SDL_GUI) || defined(GLSDL_GUI)
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            throw gui::exception("SDL_Init", "Could not initialise SDL: "+
                std::string(SDL_GetError())+".");
        }

        std::uint32_t uiFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
        if (bFullScreen) uiFlags |= SDL_WINDOW_FULLSCREEN;

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

    #if defined(SDL_GUI)
        std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> pRenderer(
            SDL_CreateRenderer(
                pWindow.get(), -1,
                SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE
            ),
            &SDL_DestroyRenderer
        );

        if (!pRenderer)
            throw gui::exception("SDL_Renderer", "Could not create renderer.");
    #else
        // Helper class to manage the OpenGL context from SDL
        struct GLContext
        {
            SDL_GLContext pContext = nullptr;

            explicit GLContext(SDL_Window* pWindow) : pContext(SDL_GL_CreateContext(pWindow))
            {
                if (pContext == nullptr)
                {
                    throw gui::exception("SDL_GL_CreateContext", "Coult not create OpenGL context.");
                }
            }

            ~GLContext()
            {
                SDL_GL_DeleteContext(pContext);
            }
        };

        GLContext mGLContext(pWindow.get());
        SDL_GL_SetSwapInterval(0);
    #endif

    #elif defined(SFML_GUI)
        sf::RenderWindow mWindow;
    #endif

    #if defined(GLSFML_GUI) || defined(SFML_GUI)
        if (bFullScreen)
            mWindow.create(sf::VideoMode(uiWindowWidth, uiWindowHeight, 32), sWindowTitle, sf::Style::Fullscreen);
        else
            mWindow.create(sf::VideoMode(uiWindowWidth, uiWindowHeight, 32), sWindowTitle);
    #endif

        // Initialize the gui
        std::cout << "Creating gui manager..." << std::endl;
        std::unique_ptr<gui::manager> pManager;

    #if defined(GLSFML_GUI) || defined(GLSDL_GUI)
        // Define the input manager
        std::unique_ptr<input::source> pInputSource;

    #if defined(GLSFML_GUI)
        // Use SFML
        pInputSource = std::unique_ptr<input::source>(new input::sfml::source(mWindow));
    #elif defined(GLSDL_GUI)
        // Use SDL
        {
            bool bInitializeSDLImage = true;
            SDL_Renderer* pRenderer = nullptr; // set to nullptr when not using an SDL_Renderer
            pInputSource = std::unique_ptr<input::source>(new input::sdl::source(
                pWindow.get(), pRenderer, bInitializeSDLImage));
        }
    #endif

        // Define the GUI renderer
        std::unique_ptr<gui::renderer> pRenderer =
            std::unique_ptr<gui::renderer>(new gui::gl::renderer(
                pInputSource->get_window_width(),
                pInputSource->get_window_height()));

        pManager = std::unique_ptr<gui::manager>(new gui::manager(
            // Provide the input source
            std::move(pInputSource),
            // Provide the GUI renderer implementation
            std::move(pRenderer),
            // The locale
            sLocale
        ));
    #elif defined(SDL_GUI)
        // Use full SDL implementation
        pManager = gui::sdl::create_manager(pWindow.get(), pRenderer.get(), sLocale);
    #elif defined(SFML_GUI)
        // Use full SFML implementation
        pManager = gui::sfml::create_manager(mWindow, sLocale);
    #endif

        // Automatically select best settings
        gui::renderer* pGUIRenderer = pManager->get_renderer();
        pGUIRenderer->auto_detect_settings();

        std::cout << " Renderer settings:" << std::endl;
        std::cout << "  Renderer: " << pGUIRenderer->get_name() << std::endl;
        std::cout << "  Max texture size: " << pGUIRenderer->get_texture_max_size() << std::endl;
        std::cout << "  Vertex cache supported: " << pGUIRenderer->is_vertex_cache_supported() << std::endl;
        std::cout << "  Vertex cache enabled: " << pGUIRenderer->is_vertex_cache_enabled() << std::endl;
        std::cout << "  Texture atlas natively supported: " << pGUIRenderer->is_texture_atlas_natively_supported() << std::endl;
        std::cout << "  Texture atlas enabled: " << pGUIRenderer->is_texture_atlas_enabled() << std::endl;
        std::cout << "  Texture atlas page size: " << pGUIRenderer->get_texture_atlas_page_size() << std::endl;

        pManager->enable_caching(false);

        pManager->set_interface_scaling_factor(fScaleFactor);

        // Load files :
        //  - first set the directory in which the interface is located
        pManager->add_addon_directory("interface");
        //  - create the lua::state
        std::cout << " Creating lua..." << std::endl;
        pManager->create_lua([](gui::manager& mManager) {
            // We use a lambda function because this code might be called
            // again later on, for example when one reloads the GUI (the
            // lua state is destroyed and created again).
            //  - register the needed widgets
            mManager.register_region_type<gui::texture>();
            mManager.register_region_type<gui::font_string>();
            mManager.register_frame_type<gui::button>();
            mManager.register_frame_type<gui::slider>();
            mManager.register_frame_type<gui::edit_box>();
            mManager.register_frame_type<gui::scroll_frame>();
            mManager.register_frame_type<gui::status_bar>();
            //  - register additional lua functions
            sol::state& mSol = mManager.get_lua();
            mSol.set_function("get_folder_list", [](const std::string& sDir) {
                return utils::get_directory_list(sDir);
            });
            mSol.set_function("get_file_list", [](const std::string& sDir) {
                return utils::get_file_list(sDir);
            });
        });

        //  - and load all files
        std::cout << " Reading gui files..." << std::endl;
        pManager->read_files();

        // Create GUI by code :

        // Create the Frame
        // A "root" frame has no parent and is directly owned by the gui::manager.
        // A "child" frame is owned by another frame.
        gui::frame* pFrame = pManager->create_root_frame<gui::frame>("FPSCounter");
        pFrame->set_abs_point(gui::anchor_point::TOPLEFT, "", gui::anchor_point::TOPLEFT);
        pFrame->set_abs_point(gui::anchor_point::BOTTOMRIGHT, "FontstringTestFrameText", gui::anchor_point::TOPRIGHT);

        // Create the FontString
        gui::font_string* pFont = pFrame->create_region<gui::font_string>(gui::layer_type::ARTWORK, "$parentText");
        pFont->set_abs_point(gui::anchor_point::BOTTOMRIGHT, "$parent", gui::anchor_point::BOTTOMRIGHT, 0, -5);
        pFont->set_font("interface/fonts/main.ttf", 12);
        pFont->set_justify_v(gui::text::vertical_alignment::BOTTOM);
        pFont->set_justify_h(gui::text::alignment::RIGHT);
        pFont->set_outlined(true);
        pFont->set_text_color(gui::color::RED);
        pFont->notify_loaded();

        // Create the scripts
        // In Lua
        /*pFrame->define_script("OnLoad",
            "self.update_time = 0.5;"
            "self.timer = 1.0;"
            "self.frames = 0;",
            "main.cpp", 146 // You can provide the location of the Lua source, for error handling
        );
        pFrame->define_script("OnUpdate",
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

        // Or in C++
        float update_time = 0.5f, timer = 1.0f;
        int frames = 0;
        pFrame->define_script("OnUpdate",
            [=](gui::frame* self, gui::event* event) mutable {
                float delta = event->get<float>(0);
                timer += delta;
                ++frames;

                if (timer > update_time) {
                    gui::font_string* text = self->get_region<gui::font_string>("Text");
                    text->set_text(U"(created in C++)\nFPS : "+utils::to_ustring(floor(frames/timer)));

                    timer = 0.0f;
                    frames = 0;
                }
            }
        );

        // Tell the Frame is has been fully loaded, and call "OnLoad"
        pFrame->notify_loaded();

        // Start the main loop

        main_loop_context mContext;
        mContext.mPrevTime = timing_clock::now();
        mContext.mFirstTime = mContext.mPrevTime;
        mContext.pManager = pManager.get();

    #if defined(SDL_GUI)
        mContext.pRenderer = pRenderer.get();
    #elif defined(GLSDL_GUI)
        mContext.pWindow = pWindow.get();
        mContext.pGLContext = mGLContext.pContext;
    #elif defined(SFML_GUI) || defined(GLSFML_GUI)
        mContext.pWindow = &mWindow;
    #endif

        std::cout << "Entering loop..." << std::endl;

    #if defined(WASM)
        emscripten_set_main_loop_arg(main_loop, &mContext, -1, 1);
    #else
        while (mContext.bRunning)
        {
            main_loop(&mContext);
        }
    #endif

        float fTotalTime = std::chrono::duration_cast<std::chrono::microseconds>(
            timing_clock::now() - mContext.mFirstTime).count() / 1e6;
        std::cout << "End of loop, mean FPS : " << mContext.uiFrameCount/fTotalTime << std::endl;
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

#if !defined(WASM)
    std::cout.rdbuf(pOldBuffer);
#endif

    return 0;
}
