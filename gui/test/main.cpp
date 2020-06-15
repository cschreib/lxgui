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
#include <lxgui/luapp_function.hpp>
#include <lxgui/utils_filesystem.hpp>
#include <lxgui/utils_string.hpp>

#include <SFML/Window.hpp>

//#define GLFW_INPUT
//#define GL_GUI

#ifdef GL_GUI
#include <lxgui/impl/gui_gl_renderer.hpp>
#ifdef MACOSX
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#else
#include <lxgui/impl/gui_sfml.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#endif

#ifdef GLFW_INPUT
#include <lxgui/impl/input_glfw_source.hpp>
#else
#include <lxgui/impl/input_sfml_source.hpp>
#endif

#ifdef WIN32
#include <windows.h>
#ifdef MSVC
#pragma comment(linker, "/entry:mainCRTStartup")
#endif
#endif

#include <fstream>

using namespace lxgui;

int l_get_folder_list(lua_State* pLua);
int l_get_file_list(lua_State* pLua);
int l_cut_file_path(lua_State* pLua);

int main(int argc, char* argv[])
{
    std::fstream mLogCout("cout.txt", std::ios::out);
    std::cout.rdbuf(mLogCout.rdbuf());

    try
    {
        uint uiWindowWidth  = 800;
        uint uiWindowHeight = 600;
        bool bFullScreen    = false;
        std::string sLocale = "enGB";

        // Read some configuration data
        if (utils::file_exists("config.lua"))
        {
            lua::state mLua;
            mLua.do_file("config.lua");
            uiWindowWidth  = mLua.get_global_int("window_width",  false, 800);
            uiWindowHeight = mLua.get_global_int("window_height", false, 600);
            bFullScreen    = mLua.get_global_bool("fullscreen",   false, false);
            sLocale        = mLua.get_global_string("locale",     false, "enGB");
        }

        // Redirect output from the gui library to a log file
        std::fstream mGUI("gui.txt", std::ios::out);
        gui::out.rdbuf(mGUI.rdbuf());

        // Create a window
        std::cout << "Creating window..." << std::endl;
    #ifdef GL_GUI
        sf::Window mWindow;
    #else
        sf::RenderWindow mWindow;
    #endif

        if (bFullScreen)
            mWindow.create(sf::VideoMode(uiWindowWidth, uiWindowHeight, 32), "test", sf::Style::Fullscreen);
        else
            mWindow.create(sf::VideoMode(uiWindowWidth, uiWindowHeight, 32), "test");

        // Initialize the gui
        std::cout << "Creating gui manager..." << std::endl;
        std::unique_ptr<gui::manager> pManager;

    #ifdef GL_GUI
        // Define the GUI renderer
        std::unique_ptr<gui::renderer_impl> pRendererImpl =
            std::unique_ptr<gui::renderer_impl>(new gui::gl::renderer());

        // Define the input manager
        std::unique_ptr<input::source_impl> pInputSource;
    #ifdef GLFW_INPUT
        // Use GLFW
        pInputSource = std::unique_ptr<input::source_impl>(new input::glfw::source(
            utils::to_string((uint)mWindow.getSystemHandle()), mWindow.getSize().x, mWindow.getSize().y
        ));
    #else
        // Use SFML
        pInputSource = std::unique_ptr<input::source_impl>(new input::sfml::source(mWindow));
    #endif

        pManager = std::unique_ptr<gui::manager>(new gui::manager(
            // Provide the input source
            std::move(pInputSource),
            // The locale
            sLocale,
            // Dimensions of the render window
            mWindow.getSize().x, mWindow.getSize().y,
            // Provide the GUI renderer implementation
            std::move(pRendererImpl)
        ));
    #else
        // Use full SFML implementation
        pManager = gui::sfml::create_manager(mWindow, sLocale);
    #endif

        pManager->enable_caching(false);

        // Load files :
        //  - first set the directory in which the interface is located
        pManager->add_addon_directory("interface");
        //  - create the lua::state
        std::cout << " Creating lua..." << std::endl;
        pManager->create_lua([&pManager](){
            // We use a lambda function because this code might be called
            // again later on, for example when one reloads the GUI (the
            // lua state is destroyed and created again).
            //  - register the needed widgets
            pManager->register_region_type<gui::texture>();
            pManager->register_region_type<gui::font_string>();
            pManager->register_frame_type<gui::button>();
            pManager->register_frame_type<gui::slider>();
            pManager->register_frame_type<gui::edit_box>();
            pManager->register_frame_type<gui::scroll_frame>();
            pManager->register_frame_type<gui::status_bar>();
            //  - register additional lua functions
            pManager->get_lua()->reg("get_folder_list", l_get_folder_list);
            pManager->get_lua()->reg("get_file_list",   l_get_file_list);
            pManager->get_lua()->reg("cut_file_path",   l_cut_file_path);
        });

        //  - and load all files
        std::cout << " Reading gui files..." << std::endl;
        pManager->read_files();

        // Create GUI by code :

        // Create the Frame
        std::unique_ptr<gui::frame> pFrame = pManager->create_frame<gui::frame>("FPSCounter");
        pFrame->set_rel_dimensions(1.0f, 1.0f);
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
                float delta = event->get(0)->get<float>();
                timer += delta;
                ++frames;

                if (timer > update_time) {
                    gui::font_string* text = self->get_region<gui::font_string>("Text");
                    text->set_text("(created in C++)\nFPS : "+utils::to_string(floor(frames/timer)));

                    timer = 0.0f;
                    frames = 0;
                }
            }
        );

        // Tell the Frame is has been fully loaded, and call "OnLoad"
        pFrame->notify_loaded();

        // Transfer ownership to the GUI manager
        pManager->add_root_uiobject(std::move(pFrame));

        // Start the main loop
        bool bRunning = true;
        bool bFocus = true;
        float fDelta = 0.1f;
        sf::Clock mClock, mPerfClock;
        uint uiFrameCount = 0;

        input::manager* pInputMgr = pManager->get_input_manager();

        std::cout << "Entering loop..." << std::endl;
        while (bRunning)
        {
            // Get events from SFML
            sf::Event mEvent;
            while (mWindow.pollEvent(mEvent))
            {
                if (mEvent.type      == sf::Event::Closed)
                    bRunning = false;
                else if (mEvent.type == sf::Event::LostFocus)
                    bFocus = false;
                else if (mEvent.type == sf::Event::GainedFocus)
                    bFocus = true;

            #ifndef GLFW_INPUT
                static_cast<input::sfml::source*>(pInputMgr->get_source())->on_sfml_event(mEvent);
            #endif
            }

            // Check if WORLD input is allowed
            if (pInputMgr->can_receive_input("WORLD"))
            {
                // Process mouse and click events in the game...
            }

            if (pInputMgr->key_is_pressed(input::key::K_ESCAPE))
                bRunning = false;
            else if (pInputMgr->key_is_pressed(input::key::K_P))
                gui::out << pManager->print_ui() << std::endl;
            else if (pInputMgr->key_is_pressed(input::key::K_K))
                gui::out << "###" << std::endl;
            else if (pInputMgr->key_is_pressed(input::key::K_C))
                pManager->enable_caching(!pManager->is_caching_enabled());
            else if (pInputMgr->key_is_pressed(input::key::K_R))
                pManager->reload_ui();

            if (!bFocus)
            {
                sf::sleep(sf::seconds(0.1f));
                continue;
            }

            // Update the gui
            pManager->update(fDelta);

            // Clear the window
        #ifdef GL_GUI
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        #else
            mWindow.clear(sf::Color(51,51,51));
        #endif

            // Render the gui
            pManager->render_ui();

            // Display the window
            mWindow.display();

            fDelta = mClock.getElapsedTime().asSeconds();
            mClock.restart();

            ++uiFrameCount;
        }
        std::cout << "End of loop, mean FPS : " << uiFrameCount/mPerfClock.getElapsedTime().asSeconds() << std::endl;
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

int l_get_folder_list(lua_State* pLua)
{
    lua::function mFunc("get_folder_list", pLua);
    mFunc.add(0, "folder", lua::type::STRING);

    if (mFunc.check())
    {
        const std::string sDir = mFunc.get(0)->get_string();
        for (const auto& sSubDir : utils::get_directory_list(sDir))
            mFunc.push(sSubDir);
    }

    return mFunc.on_return();
}

int l_get_file_list(lua_State* pLua)
{
    lua::function mFunc("get_file_list", pLua);
    mFunc.add(0, "folder", lua::type::STRING);

    if (mFunc.check())
    {
        const std::string sDir = mFunc.get(0)->get_string();
        for (const auto& sFile : utils::get_file_list(sDir))
            mFunc.push(sFile);
    }

    return mFunc.on_return();
}

int l_cut_file_path(lua_State* pLua)
{
    lua::function mFunc("cut_file_path", pLua, 1);
    mFunc.add(0, "path", lua::type::STRING);

    if (mFunc.check())
    {
        std::string sPath = mFunc.get(0)->get_string();
        std::vector<std::string> lWords = utils::cut(sPath, "/");

        for (auto iter = lWords.begin(); iter != lWords.end(); ++iter)
        {
            std::vector<std::string> lSubWords = utils::cut(*iter, "\\");
            if (lSubWords.size() > 1)
            {
                iter = lWords.erase(iter);
                for (const auto& sSubWord : lSubWords)
                    iter = lWords.insert(iter, sSubWord);
            }
        }

        std::string sFile = lWords.back();
        if (sFile.find(".") != sFile.npos)
            lWords.pop_back();
        else
            sFile = "";

        std::string sFolder;
        for (const auto& sWord : lWords)
        {
            if (sFolder.empty())
                sFolder += sWord;
            else
                sFolder += "/" + sWord;
        }

        lua::state* pState = mFunc.get_state();
        pState->new_table();
        pState->set_field_string("file", sFile);
        pState->set_field_string("folder", sFolder);
        pState->new_table();
        pState->set_field("folders");
        pState->get_field("folders");

        uint i = 1;
        for (const auto& sWord : lWords)
        {
            pState->set_field_string(i, sWord);
            ++i;
        }

        pState->pop();
        mFunc.notify_pushed();
    }

    return mFunc.on_return();
}
