[![Build Status](https://api.travis-ci.com/cschreib/lxgui.svg?branch=master)](https://travis-ci.com/cschreib/lxgui)

# What is lxgui?

There are plenty of different GUI libraries out there. They all have something that makes them unique. This is also the case of lxgui. Its main advantages are:

* **Platform independence**. The library is coded in standard C++11. Platform dependent concepts, such as rendering or input, are handled by back-end plugins (for rendering: either SFML, or pure OpenGL; for input: SFML, GLFW, or OIS).
* **Fully extensible**. Except for the base GUI components (gui::frame), every widget is designed to be used as a plugin: gui::texture, gui::font_string, gui::button, gui::edit_box, ... New widgets can be added easily in your own code without modifying lxgui.
* **Fully documented**. Every class in the library is documented. Doxygen documentation is included (and available online here).
* **GUI data from XML and Lua files**. The library can use a combination of XML files (for GUI structure) and Lua scripts (for event handling, etc) to construct a fully functional GUI. One can also create everything using C++ code if needed.
* **A familiar API...**. The XML and Lua API are directly inspired from World of Warcraft's successful GUI system. It is not an exact copy, but most of the important features are there (virtual widgets, inheritance, ...).
* **Caching**. The whole GUI can be cached into screen-sized render targets, so that interfaces with lots of widgets render extremely fast (provided it is not animated, and mostly event-driven).

![Sample screenshot](/gui/test/expected.png)

In developing this library, I have tried to make use of as few external libraries as possible, so compiling it is rather easy. Using CMake, you can compile using the command line, or create projects files for your favorite IDE (Code::Blocks, Visual Studio, ...). The front end GUI library itself only depends on Lua. XML parsing is done by a custom library included in this repository.

The only rendering back end available uses OpenGL. It depends on Freetype for font loading and rendering, and libpng for texture loading (hence, only PNG textures are supported, but other file types can be added with little effort). For the input back end, you can use SFML2, GLFW, or OIS.

Here is a brief list of the available widgets:

* **uiobject** (abstract): the very base of every GUI widget; can be placed on screen, and that's about it.
* **layered_region** (abstract): can be rendered on the screen.
* **frame**: can contain layered_regions (sorted by layer) and other frames, and respond to events.
* **texture**: can render a texture file, a gradient, or a plain color.
* **font_string**: can render text.
* **button**: a click-able frame with several states: normal, pushed, highlight.
* **check_button**: a button with a check box.
* **slider**: a frame with a texture that can be dragged vertically or horizontally.
* **status_bar**: a frame with a texture that grows depending on some value (typical use : health bars, ...).
* **edit_box**: an editable text box (multi-line edit_boxes are not yet fully supported).
* **scroll_frame**: a frame that has scrollable content.

# How do I use it? A tutorial.

Setting up the GUI in C++ is rather straight forward:

```c++
// Create an SFML render window, for example
sf::RenderWindow mWindow;

// Define the language that will be used by the interface
// (purely informative: it's always up to each addon to localize
// itself according to this value).
const std::string sLocale = "enGB";

// Initialize the GUI
std::unique_ptr<gui::manager> pManager = gui::sfml::create_manager(mWindow, sLocale);

// Grab a pointer to the SFML input manager so we can feed events to it later
input::sfml_manager* pSFMLInput = static_cast<input::sfml_manager*>(
    pManager->get_input_manager()->get_impl()
);

// Load files:
//  - first set the directory in which the interface is located
pManager->add_addon_directory("interface");
//  - create the lua::state
pManager->create_lua([&pManager](){
    // This code might be called again later on, for example when one
    // reloads the GUI (the lua state is destroyed and created again).
    //  - register the needed widgets
    pManager->register_region_type<gui::texture>();
    pManager->register_region_type<gui::font_string>();
    pManager->register_frame_type<gui::button>();
    pManager->register_frame_type<gui::slider>();
    pManager->register_frame_type<gui::edit_box>();
    pManager->register_frame_type<gui::scroll_frame>();
    pManager->register_frame_type<gui::status_bar>();
    //  - register additional lua "glue" functions if needed
    // ...
});

//  - and eventually load all files
pManager->read_files();

// Start the main loop
sf::Clock mClock;
while (true)
{
    // Retrieve the window events
    sf::Event mEvent;
    while (mWindow.pollEvent(mEvent))
    {
        // ...

        // Send these to the input manager
        pSFMLHandler->on_sfml_event(mEvent);
    }

    // Compute time spent since last GUI update
    float fDelta = mClock.getElapsedTime().asSeconds();
    mClock.restart();

    // Update the GUI
    pManager->update(fDeltaTime);

    // Render the GUI
    pManager->render_ui();
}

// Resources are cleared up automatically on destruction
```

With these few lines of code, you can then create as many "interface addons" in XML and Lua as you wish. Let's consider a very simple example: we want to create an FPS counter at the bottom right corner of the screen.

First create a new addon, by going to the "interface" folder, and creating a new folder "FPSCounter". In this folder, we create a "table of content" file which lists all the .xml and .lua files this addons uses, and some other informations (addon author, GUI version, saved variables, ...). It has to be called "FPSCounter.toc":

```
## Interface: 0001
## Title: A neat FPS counter
## Version: 1.0
## Author: You
## SavedVariables:

addon.xml
```

As you can see, we will only require a single .xml file : "addon.xml". Let's create it in the same folder. Every XML file must contain a <Ui> tag :

```xml
<Ui>
</Ui>
```

Then, within this tag, we need to create a frame (which is more or less a GUI container):

```xml
    <Frame name="FPSCounter">
        <Size>
            <RelDimension x="1.0" y="1.0"/>
        </Size>
        <Anchors>
            <Anchor point="CENTER"/>
        </Anchors>
    </Frame>
```

This creates a Frame named "FPSCounter" that fills the whole screen: the <Size> tag gives it relative size of "1.0" (relative to the parent frame, but since there is none, it is relative to the screen size), and the <Anchor> tag positions the frame in the middle of the screen.
Now, within the Frame, we create a FontString object, that can render text:

```xml
    <Frame name="FPSCounter">
        <Size>
            <RelDimension x="1.0" y="1.0"/>
        </Size>
        <Anchors>
            <Anchor point="CENTER"/>
        </Anchors>
        <Layers><Layer>
            <FontString name="$parentText" font="interface/fonts/main.ttf" text="" fontHeight="12" justifyH="RIGHT" justifyV="BOTTOM" outline="NORMAL">
                <Anchors>
                    <Anchor point="BOTTOMRIGHT">
                        <Offset>
                            <AbsDimension x="-5" y="-5"/>
                        </Offset>
                    </Anchor>
                </Anchors>
                <Color r="0" g="1" b="0"/>
            </FontString>
        </Layer></Layers>
    </Frame>
```

We've named our FontString "$parentText": "$parent" gets replaced by it's parent name, so in the end it is called "FPSCounterText". Intuitively, the "font" attribute specifies which font file to use for rendering (can be a .ttf or .otf file), "fontHeight" the size of the font, "justifyH" and "justifyV" gives the horizontal and vertical justification, and "outline" creates a black border around the letters, so that it is readable regardless of the background content. We anchor it at the bottom right corner of its parent frame, with a small offset, and give it a green color.

Now that the GUI structure is in place, we still need to display the number of frame per second. To do so, we will define two "scripts" for the "FPSCounter" Frame:

```xml
        <Scripts>
            <OnLoad>
                -- This is Lua code !
                self.update_time = 0.5;
                self.timer = 1.0;
                self.frames = 0;
            </OnLoad>
            <OnUpdate>
                -- This is Lua code !
                self.timer = self.timer + arg1;
                self.frames = self.frames + 1;

                if (self.timer > self.update_time) then
                    local fps = self.frames/self.timer;
                    self.Text:set_text("FPS : "..fps);

                    self.timer = 0.0;
                    self.frames = 0;
                end
            </OnUpdate>
        </Scripts>
```

The "OnLoad" script gets executed only once, when the Frame is created. It is used here to initialize some variables. The "OnUpdate" script is called every frame (use it carefully...). It provides the time elapsed since last update in the "arg1" variable.  We use it to record the number of frames that are rendered, and update the FPS counter every half seconds.
The "self" variable in Lua is the equivalent of "this" in C++: it is a reference to the "FPSCounter" Frame. Note that, since we've called the FontString "$parentText", we can use the handy shortcut "self.Text" instead of the full name "FPSCounterText" to reference the FontString object in Lua.

Once this is done, we have the full .xml file:

```xml
<Ui>
    <Frame name="FPSCounter">
        <Size>
            <RelDimension x="1.0" y="1.0"/>
        </Size>
        <Anchors>
            <Anchor point="CENTER"/>
        </Anchors>
        <Layers><Layer>
            <FontString name="$parentText" font="interface/fonts/main.ttf" text="" fontHeight="12" justifyH="RIGHT" justifyV="BOTTOM" outline="NORMAL">
                <Anchors>
                    <Anchor point="BOTTOMRIGHT">
                        <Offset>
                            <AbsDimension x="-5" y="-5"/>
                        </Offset>
                    </Anchor>
                </Anchors>
                <Color r="0" g="1" b="0"/>
            </FontString>
        </Layer></Layers>
        <Scripts>
            <OnLoad>
                -- This is Lua code !
                self.update_time = 0.5;
                self.timer = 1.0;
                self.frames = 0;
            </OnLoad>
            <OnUpdate>
                -- This is Lua code !
                self.timer = self.timer + arg1;
                self.frames = self.frames + 1;

                if (self.timer > self.update_time) then
                    local fps = self.frames/self.timer;
                    self.Text:set_text("FPS : "..math.floor(fps));

                    self.timer = 0.0;
                    self.frames = 0;
                end
            </OnUpdate>
        </Scripts>
    </Frame>
</Ui>
```

... and a working GUI addon !
One last thing to do before being able to see it in your program is to go to the "interface" folder, and create a file called "addons.txt". It will contain the list of addons that you want to load. In our case just write:

```
FPSCounter:1
```

The "1" means "load". If you put a "0" or remove that line, your addon will not be loaded.

Doing the very same thing in C++ would give the following code :

```c++
// Create the Frame
gui::frame* pFrame = pManager->create_frame<gui::frame>("FPSCounter");
pFrame->set_rel_dimensins(1.0f, 1.0f);
pFrame->set_abs_point(gui::anchor_point::CENTER, "", gui::anchor_point::CENTER);

// Create the FontString
gui::font_string* pFont = pFrame->create_region<gui::font_string>(gui::LAYER_ARTWORK, "$parentText");
pFont->set_abs_point(gui::anchor_point::BOTTOMRIGHT, "$parent", gui::anchor_point::BOTTOMRIGHT, -5, -5);
pFont->set_font("interface/fonts/main.ttf", 12);
pFont->set_justify_v(gui::text::vertical_alignment::BOTTOM);
pFont->set_justify_h(gui::text::alignment::RIGHT);
pFont->set_outlined(true);
pFont->set_text_color(gui::color::GREEN);
pFont->notify_loaded();

// Create the scripts in C++ (one can also provide a string containing some Lua code)
float update_time = 0.5f, timer = 1.0f;
int frames = 0;
pFrame->define_script("OnUpdate",
    [&](gui::frame* self, gui::event* event) {
        float delta = event->get<float>(0);
        timer += delta;
        ++frames;

        if (timer > update_time)
        {
            gui::font_string* text = self->get_region<gui::font_string>("Text");
            text->set_text("FPS : "+utils::to_string(floor(frames/timer)));

            timer = 0.0f;
            frames = 0;
        }
    }
);

// Tell the Frame is has been fully loaded.
pFrame->notify_loaded();
```

As you can see from the screenshot above, this system can be used to create very complex GUIs (the "File selector" frame is actually a working file explorer!). This is mainly due to a powerful inheritance system: you can create a "template" frame (making it "virtual"), that contains many object, many properties, and then instantiate several other frames that will use this "template" ("inherit" from it). This reduces the necessary code, and can help you make consistent GUIs: for example, you can create a "ButtonTemplate", and use it for all the buttons of your GUI.

Included in the source package (in the "gui/test" directory) is a test program that should compile and work fine if you have installed the whole thing properly. It is supposed to render exactly as the sample screenshot above. It can also serve as a demo program, and you can see for yourself what the XML and Lua code looks like for larger scale GUIs.
