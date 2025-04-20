from conan import ConanFile


class LxguiRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    options = {
        "shared": [True, False],
        "with_sfml": [True, False],
        "with_sdl": [True, False],
        "with_opengl": [True, False],
        "with_xml": [True, False],
        "with_yaml": [True, False],
    }
    default_options = {
        "shared": True,
        "with_sfml": True,
        "with_sdl": True,
        "with_opengl": True,
        "with_xml": True,
        "with_yaml": True
    }

    def config_options(self):
        if self.settings.os == "Emscripten":
            del self.options.with_sfml

    def requirements(self):
        self.requires("sol2/3.5.0") # may need to pull custom version
        self.requires("fmt/[>=11.0]")
        self.requires("observable_unique_ptr/0.7.2")
        self.requires("magic_enum/[>=0.9]")
        self.requires("utfcpp/[>=3.1]", transitive_headers=False)

        if self.options.with_xml:
            self.requires("pugixml/[>=1.10]", transitive_headers=False)
        if self.options.with_yaml:
            self.requires("rapidyaml/[~0.8]", transitive_headers=False)
        if self.settings.os != "Emscripten" and self.options.with_sfml:
            self.requires("sfml/[>=2.5 <3]") # Could be transitive_headers=False with little effort
        if self.options.with_opengl:
            if self.settings.os != "Emscripten":
                self.requires("opengl/system", transitive_headers=False)
                self.requires("glew/[>=2.2]", transitive_headers=False)
                self.requires("libpng/[>=1.6]", transitive_headers=False)
                self.requires("zlib/[>=1.2]", transitive_headers=False)
                self.requires("freetype/[>=2.10]") # Could be transitive_headers=False with little effort
        if self.options.with_sdl:
            if self.settings.os != "Emscripten":
                self.requires("sdl/[>=2 <3]", force=True, transitive_headers=False)
                self.requires("sdl_ttf/[>=2 <3]", transitive_headers=False)
                self.requires("sdl_image/[>=2 <3]", transitive_headers=False)

        self.requires("flac/1.4.3", override=True)

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.31]")
