project('rope', 'cpp', 'c', version: '0.1.7',
        meson_version: '>=1.1.0')
build_dir = meson.current_build_dir()
links = meson.get_cross_property('links', [])
compile_args = meson.get_cross_property('compile_args', [])

# Check if we are building natively or for emscripten
is_native = (links == [])

# C++ version
add_global_arguments('-std=c++20', language: 'cpp')

# Debugging
message('Building RoPE version', meson.project_version(), 'in', build_dir)
message('  PREFIX:', get_option('prefix'))
message('  NATIVE:', is_native)

# Dependencies
curl = dependency('libcurl', required: is_native)
fftw = dependency('fftw3f', required: is_native)
gemmi = dependency('gemmi', modules: ['gemmi::gemmi_cpp'], required: is_native)
gl = dependency('gl', method: 'system', required: false)
glew = dependency('glew', required: false)
glm = dependency('glm')
json = dependency('nlohmann_json')
sdl2 = dependency('SDL2', required: false)
sdl2image = dependency('SDL2_image', required: false)
thread_dep = dependency('threads')  # Check: is this required for emscripten?
zlib = dependency('zlib')

# Dependency groups
header_deps = [glm, json]
core_deps = [curl, fftw, gemmi, thread_dep, zlib]
graphics_deps = [gl, glew, sdl2, sdl2image]
cli_deps = [header_deps, core_deps]
gui_deps = [header_deps, core_deps, graphics_deps]

# Paths
data_dir = get_option('prefix') / get_option('datadir') / 'vagabond'
assets_dir = data_dir / 'assets'

# Install assets
install_subdir('assets/geometry', install_dir: assets_dir)
install_subdir('assets/shaders_450', install_dir: assets_dir / 'shaders', strip_directory: true)
install_subdir('assets/images', install_dir: assets_dir)

# Execute meson.build files in subdirectories
subdir('config')
subdir('vagabond/utils')
subdir('vagabond/c4x')
subdir('vagabond/core')
subdir('vagabond/cmd')
subdir('vagabond/gui/elements')
subdir('vagabond/gui')

# Create executables
if is_native  # Native builds

  # Additional subdirectories for native builds
  subdir('vagabond/untangle')

  executable('rope.gui', 'vagabond/gui/main.cpp',
    link_with: [gui, core],
    link_args: [links],
    dependencies: gui_deps,
    install: true)

  executable('rope', 'vagabond/cmd/main.cpp',
    link_with: [core, cli],
    link_args: [links],
    dependencies: cli_deps,
    install: true)

  executable('rope.gui.test',
    ['vagabond/gui/tests/main.cpp',
     'vagabond/gui/tests/CheckList.cpp',
     'vagabond/gui/tests/CheckList.h'],
    link_with: [gui, core],
    link_args: [links],
    dependencies: gui_deps,
    install: true)

else  # emscripten
  extrafiles = ['vagabond/gui/print_file.cpp']
  start_cpp = 'vagabond/gui/main.cpp'
  compile_args += ['-DNDEBUG', '-DBOOST_UBLAS_NDEBUG']
  executable('index.html', extrafiles, start_cpp,
    link_with: [core, gui],
    cpp_args: ['-I/usr/local/include', compile_args],
    link_args: [links],
    dependencies: header_deps,
    install: true)
endif

