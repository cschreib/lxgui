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
#include <lxgui/impl/gui_gl_manager.hpp>

#include <SFML/Window.hpp>
#include <fstream>

//#define OIS_INPUT

#ifdef OIS_INPUT
#include <lxgui/impl/ois_input_impl.hpp>
#else
#include <lxgui/impl/sfml_input_impl.hpp>
#endif

#ifdef WIN32
#include <windows.h>
#ifdef MSVC
#pragma comment(linker, "/entry:mainCRTStartup")
#endif
#endif
#include <GL/gl.h>

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
        sf::Window mWindow;
        if (bFullScreen)
            mWindow.create(sf::VideoMode(uiWindowWidth, uiWindowHeight, 32), "test", sf::Style::Fullscreen);
        else
            mWindow.create(sf::VideoMode(uiWindowWidth, uiWindowHeight, 32), "test");

        // Define the input handling method
        input::handler mHandler;
    #ifdef OIS_INPUT
        // OIS if available
        mHandler = utils::refptr<input::handler_impl>(new input::ois_handler(
            utils::to_string((uint)mWindow.getSystemHandle()), mWindow.getSize().x, mWindow.getSize().y
        ));
    #else
        // Else fall back to SFML input.
        // Note that the current SFML input handler is not fully functional,
        // since SFML doesn't provide keyboard layout independent key codes.
        mHandler = utils::refptr<input::handler_impl>(new input::sfml_handler(mWindow));
        utils::wptr<input::sfml_handler> pSFMLHandler = utils::wptr<input::sfml_handler>::dyn_cast(
            mHandler.get_impl()
        );
    #endif

        // Initialize the gui
        std::cout << "Creating gui manager..." << std::endl;
        gui::manager mManager(
            // Provide the input handler
            mHandler,
            // The locale
            sLocale,
            // Dimensions of the render window
            mWindow.getSize().x, mWindow.getSize().y,
            // The OpenGL implementation of the gui
            utils::refptr<gui::manager_impl>(new gui::gl::manager())
        );

        mManager.enable_caching(false);

        // Load files :
        //  - first set the directory in which the interface is located
        mManager.add_addon_directory("interface");
        //  - create the lua::state
        std::cout << " Creating lua..." << std::endl;
        mManager.create_lua([&mManager](){
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
            mManager.get_lua()->reg("get_folder_list", l_get_folder_list);
            mManager.get_lua()->reg("get_file_list",   l_get_file_list);
            mManager.get_lua()->reg("cut_file_path",   l_cut_file_path);
        });

        //  - and load all files
        std::cout << " Reading gui files..." << std::endl;
        mManager.read_files();

        // Create GUI by code :

        // Create the Frame
        gui::frame* pFrame = mManager.create_frame<gui::frame>("FPSCounter");
        pFrame->set_rel_dimensions(1.0f, 1.0f);
        pFrame->set_abs_point(gui::ANCHOR_BOTTOMRIGHT, "FontstringTestFrameText", gui::ANCHOR_TOPRIGHT);

        // Create the FontString
        gui::font_string* pFont = pFrame->create_region<gui::font_string>(gui::LAYER_ARTWORK, "$parentText");
        pFont->set_abs_point(gui::ANCHOR_BOTTOMRIGHT, "$parent", gui::ANCHOR_BOTTOMRIGHT, 0, -5);
        pFont->set_font("interface/fonts/main.ttf", 12);
        pFont->set_justify_v(gui::text::ALIGN_BOTTOM);
        pFont->set_justify_h(gui::text::ALIGN_RIGHT);
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
            [&](gui::frame* self, gui::event* event) {
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

        // Start the main loop
        bool bRunning = true;
        bool bFocus = true;
        float fDelta = 0.1f;
        sf::Clock mClock, mPerfClock;
        uint uiFrameCount = 0;

        utils::wptr<input::manager> pInputMgr = mManager.get_input_manager();

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

            #ifndef OIS_INPUT
                pSFMLHandler->on_sfml_event(mEvent);
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
                gui::out << mManager.print_ui() << std::endl;
            else if (pInputMgr->key_is_pressed(input::key::K_K))
                gui::out << "###" << std::endl;
            else if (pInputMgr->key_is_pressed(input::key::K_C))
                mManager.enable_caching(!mManager.is_caching_enabled());
            else if (pInputMgr->key_is_pressed(input::key::K_R))
                mManager.reload_ui();

            if (!bFocus)
            {
                sf::sleep(sf::seconds(0.1f));
                continue;
            }

            // Update the gui
            mManager.update(fDelta);

            // Clear the window
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // Render the gui
            mManager.render_ui();

            // Display the window
            mWindow.display();

            fDelta = mClock.getElapsedTime().asSeconds();
            mClock.restart();

            ++uiFrameCount;
        }
        std::cout << "End of loop, mean FPS : " << uiFrameCount/mPerfClock.getElapsedTime().asSeconds() << std::endl;
    }
    catch (utils::exception& e)
    {
        std::cout << e.get_description() << std::endl;
        return 1;
    }
    catch (std::exception& e)
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
    mFunc.add(0, "folder", lua::TYPE_STRING);

    if (mFunc.check())
    {
        std::vector<std::string> dirs = utils::get_directory_list(mFunc.get(0)->get_string());
        std::vector<std::string>::iterator iter;
        foreach (iter, dirs)
            mFunc.push(*iter);
    }

    return mFunc.on_return();
}

int l_get_file_list(lua_State* pLua)
{
    lua::function mFunc("get_file_list", pLua);
    mFunc.add(0, "folder", lua::TYPE_STRING);

    if (mFunc.check())
    {
        std::vector<std::string> files = utils::get_file_list(mFunc.get(0)->get_string());
        std::vector<std::string>::iterator iter;
        foreach (iter, files)
            mFunc.push(*iter);
    }

    return mFunc.on_return();
}

int l_cut_file_path(lua_State* pLua)
{
    lua::function mFunc("cut_file_path", pLua, 1);
    mFunc.add(0, "path", lua::TYPE_STRING);

    if (mFunc.check())
    {
        std::string sPath = mFunc.get(0)->get_string();
        std::vector<std::string> lWords = utils::cut(sPath, "/");

        std::vector<std::string>::iterator iter, iter2;
        foreach (iter, lWords)
        {
            std::vector<std::string> lSubWords = utils::cut(*iter, "\\");
            if (lSubWords.size() > 1)
            {
                iter = lWords.erase(iter);
                foreach (iter2, lSubWords)
                    iter = lWords.insert(iter, *iter2);
            }
        }

        std::string sFile = lWords.back();
        if (sFile.find(".") != sFile.npos)
            lWords.pop_back();
        else
            sFile = "";

        std::string sFolder;
        foreach (iter, lWords)
        {
            if (sFolder.empty())
                sFolder += *iter;
            else
                sFolder += "/" + *iter;
        }

        lua::state* pState = mFunc.get_state();
        pState->new_table();
        pState->set_field_string("file", sFile);
        pState->set_field_string("folder", sFolder);
        pState->new_table();
        pState->set_field("folders");
        pState->get_field("folders");

        uint i = 1;
        foreach (iter, lWords)
        {
            pState->set_field_string(i, *iter);
            ++i;
        }

        pState->pop();
        mFunc.notify_pushed();
    }

    return mFunc.on_return();
}
