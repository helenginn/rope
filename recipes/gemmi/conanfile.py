from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.files import get

class GemmiConan(ConanFile):
    name = "gemmi"
    version = "0.6.7"
    license = "Mozilla Public License 2.0"
    url = "https://github.com/project-gemmi/gemmi"
    description = "Library for structural biology"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    def source(self):
        get(
            self,
            **self.conan_data["sources"][str(self.version)],
        )

    def build_requirements(self):
        self.tool_requires("cmake/[*]")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["gemmi_cpp"]
