from conan import ConanFile


class Rope(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "MesonToolchain", "PkgConfigDeps"

    def build_requirements(self):
        self.tool_requires("meson/[*]")
        self.tool_requires("ninja/[*]")
        if self.settings.os == "Windows":
            self.tool_requires("pkgconf/[*]")

    def requirements(self):
        self.requires("libcurl/[*]")
        self.requires("fftw/[*]")
        self.requires("gemmi/[0.6.7]")
        self.requires("glm/[*]")
        self.requires("nlohmann_json/[*]")
        self.requires("zlib/[*]")
        self.requires("doctest/[*]")
        self.requires("cli11/[*]")

        if self.settings.os == "Windows":
            self.requires("sdl/[~2]")
            self.requires("sdl_image/[~2]")
            self.requires("glew/[*]")

    def configure(self):
        self.options["fftw"].enable_float = True
        self.options["*"].shared = False
        if self.settings.os == "Windows":
            self.options["libcurl/*"].with_ssl = "schannel"

