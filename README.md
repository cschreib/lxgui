![Build Status](https://github.com/cschreib/lxgui/actions/workflows/cmake.yml/badge.svg) ![Build Status](https://github.com/cschreib/lxgui/actions/workflows/doc.yml/badge.svg)

<!-- MarkdownTOC autolink="true" -->

- [What is lxgui?](#what-is-lxgui)
    - [In a nutshell](#in-a-nutshell)
    - [Available GUI widgets](#available-gui-widgets)
    - [Demonstration](#demonstration)
    - [Gallery](#gallery)
    - [Front-end and back-ends](#front-end-and-back-ends)
    - [Configurable rendering options](#configurable-rendering-options)
- [Getting started](#getting-started)
    - [Build for Linux, OSX, Windows](#build-for-linux-osx-windows)
        - [Required dependencies \(for all back-ends\)](#required-dependencies-for-all-back-ends)
        - [Dependencies for pure SFML back-end](#dependencies-for-pure-sfml-back-end)
        - [Dependencies for pure SDL back-end](#dependencies-for-pure-sdl-back-end)
        - [Dependencies for OpenGL + SFML back-end](#dependencies-for-opengl--sfml-back-end)
        - [Dependencies for OpenGL + SDL back-end](#dependencies-for-opengl--sdl-back-end)
    - [Build for WebAssembly / Emscripten](#build-for-webassembly--emscripten)
- [How do I use it? A tutorial.](#how-do-i-use-it-a-tutorial)
    - [Setting up the GUI in C++](#setting-up-the-gui-in-c)
    - [Creating a GUI addon in XML and Lua](#creating-a-gui-addon-in-xml-and-lua)
    - [Equivalent in pure C++](#equivalent-in-pure-c)

<!-- /MarkdownTOC -->

# What is lxgui?

## In a nutshell

There are plenty of different GUI libraries out there. They all have something that makes them unique. This is also the case of lxgui. Its main advantages are:

* **Fast**. The library implements a number of caching mechanisms to minimize the number of GPU draw calls and overall CPU usage. This enables real-time rendering even on more limited platforms, such as WebAssembly on mobile systems. See the [List of configurable rendering options](#list-of-configurable-rendering-options) for more information.
* **Platform independent**. The library is coded in standard C++17. Platform dependent concepts, such as rendering or input, are handled by back-end plugins (for rendering: SFML, SDL, or pure OpenGL; for input: SFML or SDL). Builds on Linux, MacOS, Windows, and WebAssembly.
* **High-DPI aware**. The interface can be scaled by an arbitrary factor when rendered on the screen. This can be used to improve accessibility for visually-impaired users.
* **Non intrusive**. The library will fit in your existing application without taking over your main loop. All it needs is being fed events, a call to `update()`, a call to `render()`, and nothing more.
* **Fully extensible**. Except for the base GUI components (gui::frame), every widget is designed to be used as a plugin: gui::texture, gui::font_string, gui::button, gui::edit_box, ... New widgets can be added easily in your own code without modifying lxgui.
* **Fully documented**. Every class in the library is documented. Doxygen documentation is included (and available on-line [here](https://cschreib.github.io/lxgui/html/annotated.html) for the C++ API, and [here](https://cschreib.github.io/lxgui/lua/index.html) for the Lua API).
* **Design with XML and Lua files**. The library can use a combination of XML files (for GUI structure) and Lua scripts (for event handling, etc) to construct a fully functional GUI. One can also create everything directly C++ if the flexibility of Lua+XML is not required.
* **Internationalization and localization support**. The library supports translatable text with a fully flexible system, allowing correct display of the GUI in multiple languages. This is optional: if only one language is required, one can just hard-code strings without worrying about translations. At present, only left-to-right languages are supported.
* **A familiar API...**. The XML and Lua APIs are directly inspired from World of Warcraft's successful GUI system. It is not an exact copy, but most of the important features are there (virtual widgets, inheritance, ...).


## Available GUI widgets

* **uiobject** (abstract): the very base of every GUI widget; can be placed on screen, and that's about it.
* **layered_region** (abstract): can be rendered on the screen.
* **frame**: can contain layered_regions (sorted by layer) and other frames, and respond to events.
* **texture**: can render a texture file, a gradient, or a plain color.
* **font_string**: can render text.
* **button**: a click-able frame with several states: normal, pushed, highlight.
* **check_button**: a button with a check box.
* **slider**: a frame with a texture that can be dragged vertically or horizontally.
* **status_bar**: a frame with a texture that grows depending on some value (typical use: health bars, ...).
* **edit_box**: an editable text box (multi-line edit_boxes are not yet fully supported).
* **scroll_frame**: a frame that has scrollable content.

As you can see from the screenshot below, lxgui can be used to create very complex GUIs (the "File selector" frame is actually a working file explorer!). This is mainly due to a powerful inheritance system: you can create a "template" frame (making it "virtual"), that contains many object, many properties, and then instantiate several other frames that will use this "template" ("inherit" from it). This reduces the necessary code, and can help you make consistent GUIs: for example, you can create a "ButtonTemplate", and use it for all the buttons of your GUI.


## Demonstration

A WebAssembly live demo is accessible on-line [here](https://cschreib.github.io/lxgui/demo/lxgui-test-opengl-sdl-emscripten.html) (if your browser supports WebGL2) or [here](https://cschreib.github.io/lxgui/demo/lxgui-test-sdl-emscripten.html) (if your browser only supports WebGL1). Bootstrap examples are available in the `gui/examples` directory in this repository, and demonstrate the steps required to include lxgui in a CMake project.

Included in the source package (in the `gui/test` directory) is a test program that should compile and work fine if you have installed the whole thing properly. It is supposed to render exactly as the sample screenshot below. It can also serve as a demo program, and you can see for yourself what the XML and Lua code looks like for larger scale GUIs.


## Gallery

Please head to the [screenshots page](screenshots/screenshots.md) for examples of lxgui being used in real-world projects.

Below is a screenshot of the test program included with the library (the same interface is displayed in the live demo linked above). It is meant to test and demonstrate most of the features available in the library. Note that the "look-and-feel" displayed here is purely for demonstration; every element of the interface (colors, dialog shapes and style) is defined in fully customizable Lua and XML files.

![Sample screenshot](/gui/test/expected.png)

This screenshot was generated on a Release (optimised) build of lxgui with the OpenGL+SFML back-end, on a system running Linux Mint 20.2, with a Ryzen 5 2600 CPU, 16GB of RAM, an Nvidia GTX 960 2GB GPU with proprietary drivers, and a standard resolution monitor. All optimisations were turned on except screen caching.


## Front-end and back-ends

Using CMake, you can compile using the command line, or create projects files for your favorite IDE. The front-end GUI library itself only depends on [Lua](http://www.lua.org/) (>5.1), [sol2](https://github.com/ThePhD/sol2) (included as a submodule), [utfcpp](https://github.com/nemtrif/utfcpp) (included as a submodule), [oup](https://github.com/cschreib/observable_unique_ptr) (included as submodule), and [fmtlib](https://github.com/fmtlib/fmt) (included as submodule). XML parsing is done using a custom library included in this repository.

Available rendering back-ends:

 - OpenGL. This back-end depends on [Freetype](https://www.freetype.org/) for font loading and rendering, and [libpng](http://www.libpng.org/pub/png/libpng.html) for texture loading (hence, only PNG textures are supported, but other file types can be added with little effort), as well as [GLEW](http://glew.sourceforge.net/) for OpenGL support. It can be compiled either in "legacy" OpenGL (fixed pipeline) for maximum compatibility, or OpenGL 3 (programmable pipeline) for maximum performance. This back-end generally offers the best possible performance, but it also has the largest number of third-party dependencies, hence it can be harder to set up.

 - SFML2. This back-end uses [SFML2](https://www.sfml-dev.org/) for everything, and thus only depends on SFML. It runs a little bit slower than the OpenGL back-end, as the extra layer from SFML adds a bit of overhead. At present, some limitations in the SFML API also prevents using VBOs.

 - SDL2. This back-end uses [SDL2](https://www.libsdl.org/) for rendering, [SDL2_tff](https://www.libsdl.org/projects/SDL_ttf/) for font loading and rendering, and [SDL2_image](https://www.libsdl.org/projects/SDL_image/) for texture loading. It is the slowest available back-end, but also the one that supports the largest number of platforms (including platforms lacking a GPU). VBOs are not supported, and neither is per-vertex color. Text rendering is limited to Unicode code points which fit in a 16bit integer.

Available input back-ends:

 - SFML2. This back-ends depends on SFML-Window for event handling and SFML-Graphics for loading cursor files. It supports all features except for the automatic detection of hi-DPI displays.
 - SDL2. This back-end uses SDL2 for event handling and SDL2_image for loading cursor files. It supports all features.

If you have the choice, the currently recommended back-ends are OpenGL for rendering, and SDL for input. The WebAssembly build supports all back-ends except SFML.


## Configurable rendering options

- Enable/disable vertex caches (VBOs). When supported by the rendering back-end, vertex caches always improve performance by reducing the transfer of data between the CPU and GPU. This is currently supported on all back-ends except SDL and the "legacy" OpenGL fixed-pipeline.
- Enable/disable quad batching. When enabled, quads sharing the same texture and rendering modes (i.e., blend mode, view) are accumulated into a vertex array on the CPU, which is then sent to the GPU and drawn in a single operation. This reduces the number of draw calls, and synchronization points between the CPU and GPU, hence can significantly improve performances. This is particularly the case on platforms where draw calls are expensive, like WebAssembly.
- Enable/disable texture atlases. Texture atlases combine multiple textures into a single GPU texture, so that multiple objects can be drawn with fewer GPU state changes. This reduces the number of draw calls, and can improve performance, particularly when quad batching is enabled. However, this also disables automatic texture tiling, which requires creating more vertices when tiling is needed. This can decrease performance if neither quad batching nor vertex caches are enabled.
- Texture atlas page size. This defaults to 4096 (or lower, if the back-end does not support textures of this size). This provides a decent amount of batching without using too much memory. This can be increased to improve draw call batching if the machine has enough free GPU memory.
- Enable/disable render target caching. When render targets are supported by the rendering back-end and this option is turned on, each "strata" of the GUI (i.e., global layers) is rendered on a separate screen-size render target, which is only re-rendered if its content changes. This increases GPU memory usage, but can significantly improve performance for GUIs that are mostly static and/or event-driven. For GUIs that update on every frame (i.e., with animations or frequent data update), the performance can be negatively affected.

Except for render target caching, all options are enabled by default (if supported), which should offer the best performance in most cases. It can be that your particular use case does not benefit as much from the default caching and batching implementations; this can be easily checked by trying various combinations of these options, and selecting the combination that offers the best performances for your particular use case and target hardware.


# Getting started

Firstly, ensure your C++ compiler is up to date. This library requires a compiler that is C++17 compliant (GCC >= 8, clang >= 7, Apple-Clang >= 11, or Visual Studio >= 2017).

Then, clone the project, including the submodules (this is important! the library will not compile otherwise):
```
git clone --recurse-submodules https://github.com/cschreib/lxgui
```

The rest of the build instructions depends on your target operating system; please follow the instructions in the next sections accordingly.


## Build for Linux, OSX, Windows

Make your choice of rendering and input back-end from within the following sub-sections, and install all the appropriate dependencies listed there. Once this is done, you can build and install lxgui with the standard cmake commands:

```
mkdir build
cd build
cmake ../ <your CMake options here>
cmake --build . --config Release
cmake --install . --config Release
```


### Required dependencies (for all back-ends)

Install Lua (>5.1):

- dnf based distros (Fedora):
```
sudo dnf install -y lua-devel
```
- apt based distros (Debian, Ubuntu):
```
sudo apt install liblua5.2-dev
```
- OSX:
```
 brew install lua
```
- Windows:
```
 vcpkg install lua
```


### Dependencies for pure SFML back-end

Install SFML2:

- dnf based distros (Fedora):
```
sudo dnf install -y SFML-devel
```
- apt based distros (Debian, Ubuntu):
```
sudo apt install libsfml-dev
```
- OSX:
```
brew install sfml
```
- Windows:
```
vcpkg install sfml
```


### Dependencies for pure SDL back-end

Install SDL2, SDL2_image, and SDL2_ttf:

- dnf based distros (Fedora):
```
sudo dnf install -y SDL2-devel SDL2_image-devel SDL2_ttf-devel
```
- apt based distros (Debian, Ubuntu):
```
sudo apt install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev
```
- OSX:
```
brew install sdl2 sdl2_ttf sdl2_image
```
- Windows:
```
vcpkg install sdl2 sdl2-ttf sdl2-image
```


### Dependencies for OpenGL + SFML back-end

Install OpenGL, Freetype, libpng, and SFML2:

- dnf based distros (Fedora):
```
sudo dnf install -y freetype-devel SFML-devel mesa-libGLU-devel
```
- apt based distros (Debian, Ubuntu):
```
sudo apt install libsfml-dev libz-dev libpng-dev libfreetype6-dev libglew-dev libglu1-mesa-dev
```
- OSX:
```
brew install sfml zlib libpng freetype glew
```

- Windows:
```
vcpkg install sfml zlib libpng freetype glew
```


### Dependencies for OpenGL + SDL back-end

Install OpenGL, Freetype, libpng, SDL2, and SDL2_image:

- dnf based distros (Fedora):
```
sudo dnf install -y SDL2-devel SDL2_image-devel freetype-devel mesa-libGLU-devel
```

- apt based distros (Debian, Ubuntu):
```
sudo apt install libsdl2-dev libsdl2-image-dev libz-dev libpng-dev libfreetype6-dev libglew-dev libglu1-mesa-dev
```

- OSX:
```
brew install sdl2 sdl2-image zlib libpng freetype glew
```

- Windows:
```
vcpkg install sdl2 sdl2-image zlib libpng freetype glew
```


## Build for WebAssembly / Emscripten

The WebAssembly build only supports the SDL2 back-end for input, and either the SDL2 or OpenGL back-ends for rendering (programmable pipeline only; the legacy fixed pipeline is not supported in WebGL). SDL2, OpenGL, and libpng are all already provided by default in Emscripten, so the only required dependency to setup is Lua and Freetype (at the time of writing this guide, the Freetype version in Emscripten was too old). Pre-compiled libraries are provided in `dependencies/wasm.zip`, but you can also build them from source yourself easily.

The SDL2 rendering back-end will support all platforms supported by SDL2, which should cover pretty much everything, but it may run slower on some platforms. The OpenGL back-end uses OpenGL ES 3, hence will only run on platforms supporting WebGL2, but it should provide the best performance. In practice, performance is highly dependent on the the host platform and browser. For example: earlier in the development of lxgui, and on my desktop machine, the SDL2 back-end was slower (30 FPS) than the OpenGL back-end (40 FPS) in Firefox, but in Chrome they both ran at the maximum 60 FPS. This is likely to change in the future, with browser updates and changes in the lxgui implementation.

With Emscripten [installed and sourced](https://emscripten.org/docs/getting_started/downloads.html) in your current terminal, run

```
mkdir build
cd build
emcmake cmake ../ <your CMake options here>
emmake make
emmake make install
```


# How do I use it? A tutorial.

## Setting up the GUI in C++

Setting up the GUI in C++ is rather straightforward. The example code below is based on the SFML back-end, but can be adapted to other back-ends easily (see examples for help).

```c++
// Create an SFML render window
sf::RenderWindow mWindow;

// Initialize the GUI using the SFML back-end
utils::owner_ptr<gui::manager> pManager = gui::sfml::create_manager(mWindow);

// Grab a pointer to the SFML input manager so we can feed events to it later
input::sfml::source& mSFMLInput = static_cast<input::sfml::source&>(
    pManager->get_input_manager().get_source()
);

// Load GUI addons:
// In lxgui, the GUI is formed of multiple modular "addons", which each define
// the appearance and behavior of a specific GUI element (e.g., one addon for
// the player status bars, one addon for the inventory, etc.).
// See below for an example addon.

//  - First set the directory in which the GUI addons are located
pManager->add_addon_directory("interface");
//  - Then create the Lua state
pManager->create_lua([](gui::manager& mManager)
{
    // This code might be called again later on, for example when one
    // reloads the GUI (the Lua state is destroyed and created again).
    //  - register the needed widgets
    mManager.register_region_type<gui::texture>();
    mManager.register_region_type<gui::font_string>();
    mManager.register_frame_type<gui::button>();
    mManager.register_frame_type<gui::slider>();
    mManager.register_frame_type<gui::edit_box>();
    mManager.register_frame_type<gui::scroll_frame>();
    mManager.register_frame_type<gui::status_bar>();
    //  - register your own additional Lua "glue" functions, if needed
    // ...
});

//  - and eventually load all addons
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
        mSFMLInput.on_sfml_event(mEvent);
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

With these few lines of code, you can create as many "interface addons" in XML and Lua as you wish. Let's consider a very simple example: we want to create an FPS counter at the bottom right corner of the screen.


## Creating a GUI addon in XML and Lua

First create a new addon, by going to the `interface` folder, and creating a new folder `FPSCounter`. In this folder, we create a "table of content" file which lists all the `*.xml` and `*.lua` files this addons uses, and some other informations (addon author, GUI version, saved variables, ...). It has to be called `FPSCounter.toc` (after the name of the addon directory):

```
## Interface: 0001
## Title: A neat FPS counter
## Version: 1.0
## Author: You
## SavedVariables:

addon.xml
```

As you can see, we will only require a single XML file: `addon.xml`. Let's create it in the same folder. Every XML file must contain a `<Ui>` tag:

```xml
<Ui>
</Ui>
```

Then, within this tag, we need to create a frame (which is more or less a GUI container):

```xml
    <Frame name="FPSCounter">
        <Anchors>
            <Anchor point="TOPLEFT" relativePoint="TOPLEFT"/>
            <Anchor point="BOTTOMRIGHT" relativePoint="TOPLEFT"/>
        </Anchors>
    </Frame>
```

This creates a Frame named `FPSCounter` that fills the whole screen: the `<Anchor>` tags forces the top-left and bottom-right corners to match the screen's top-left and bottom-right corners. Now, within the Frame, we create a FontString object, which can render text:

```xml
    <Frame name="FPSCounter">
        <Anchors>
            <Anchor point="TOPLEFT" relativePoint="TOPLEFT"/>
            <Anchor point="BOTTOMRIGHT" relativePoint="TOPLEFT"/>
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

We named our FontString `$parentText`. In names, `$parent` gets automatically replaced by the name of the object's parent; in this case, its full name will end up as `FPSCounterText`.

Intuitively, the `font` attribute specifies which font file to use for rendering (can be a `*.ttf` or `*.otf` file), `fontHeight` the size of the font (in points), `justifyH` and `justifyV` specify the horizontal and vertical alignment, and `outline` creates a black border around the letters, so that the text is readable regardless of the background content. We anchor it at the bottom right corner of its parent frame, with a small offset in the `<Offset>` tag (also specified in points), and give it a green color with the `<Color>` tag.

NB: the GUI positioning is done in "points". By default, on traditional displays a point is equivalent to a pixel, but it can be equivalent to two or more pixels on modern hi-DPI displays. In addition, the GUI can always be rescaled by an abitrary scaling factor (in the same way that you can zoom on a web page in your browser). This rescaling factor is set to `1.0` by default, but changing its value also changes the number of pixels per points.

Now that the GUI structure is in place, we still need to display the number of frame per second. To do so, we will define two "scripts" for the `FPSCounter` Frame:

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

The `<OnLoad>` script gets executed only once, when the Frame is created. It is used here to initialize some variables. The `<OnUpdate>` script is called every frame (use it carefully...). It provides the time elapsed since last update in the `arg1` variable. We use it to record the number of frames that are rendered, and update the FPS counter every half seconds.

The `self` variable in Lua is the equivalent of `this` in C++: it is a reference to the object running the script, here the `FPSCounter` Frame. Note that, since we called the FontString `$parentText`, we can use the handy shortcut `self.Text` instead of the full name `FPSCounterText` to reference the FontString object in Lua. This is good practice, and allows for more generic and modular code.

Once this is done, we have the full XML file:

```xml
<Ui>
    <Frame name="FPSCounter">
        <Anchors>
            <Anchor point="TOPLEFT" relativePoint="TOPLEFT"/>
            <Anchor point="BOTTOMRIGHT" relativePoint="TOPLEFT"/>
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

... and a working GUI addon!

One last thing to do before being able to see it in your program is to go to the `interface` folder, and create a file called `addons.txt`. This file must exist, and must contain the list of addons that you want to load. In our case just write:

```
FPSCounter:1
```

The `1` means "load". If you put a `0` or remove that line, your addon will not be loaded.


## Equivalent in pure C++

Re-creating the above addon in pure C++ is perfectly possible. This can be done with the following code:

```c++
// Create the Frame
utils::observer_ptr<gui::frame> pFrame;
pFrame = pManager->create_root_frame<gui::frame>("FPSCounter");
pFrame->set_abs_point(gui::anchor_point::TOPLEFT, "", gui::anchor_point::TOPLEFT);
pFrame->set_abs_point(gui::anchor_point::BOTTOMRIGHT, "", gui::anchor_point::BOTTOMRIGHT);

// Create the FontString
utils::observer_ptr<gui::font_string> pFont;
pFont = pFrame->create_region<gui::font_string>(gui::LAYER_ARTWORK, "$parentText");
pFont->set_abs_point(gui::anchor_point::BOTTOMRIGHT, "$parent", gui::anchor_point::BOTTOMRIGHT, -5, -5);
pFont->set_font("interface/fonts/main.ttf", 12);
pFont->set_justify_v(gui::text::vertical_alignment::BOTTOM);
pFont->set_justify_h(gui::text::alignment::RIGHT);
pFont->set_outlined(true);
pFont->set_text_color(gui::color::GREEN);
pFont->notify_loaded(); // must be called on all objects when they are fully set up

// Create the scripts in C++ (one can also provide a string containing some Lua code)
float update_time = 0.5f, timer = 1.0f;
int frames = 0;
pFrame->define_script("OnUpdate", [=](gui::frame& self, gui::event* event) mutable
{
    float delta = event->get<float>(0);
    timer += delta;
    ++frames;

    if (timer > update_time)
    {
        gui::font_string* text = self.get_region<gui::font_string>("Text");
        text->set_text("FPS : "+utils::to_string(floor(frames/timer)));

        timer = 0.0f;
        frames = 0;
    }
});

// Tell the Frame is has been fully loaded.
pFrame->notify_loaded();
```
