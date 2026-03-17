from conan import ConanFile


class Rope(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "MesonToolchain", "PkgConfigDeps"

    def requirements(self):
        self.requires("libcurl/[*]")
        self.requires("fftw/[*]")
        self.requires("gemmi/[*]")
        self.requires("glew/[*]")
        self.requires("glm/[*]")
        self.requires("nlohmann_json/[*]")
        self.requires("zlib/[*]")

        if self.settings.os == "Windows":
            self.requires("sdl/[~2]")
            self.requires("sdl_image/[~2]")

    def build_requirements(self):
        if self.settings.os == "Windows":
            self.tool_requires("pkgconf/[*]")

    def configure(self):
        self.options["fftw"].enable_float = True
        self.options["*"].shared = False
