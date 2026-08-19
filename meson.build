project('rope', 'cpp', 'c', version: '0.1.8',
        default_options: ['cpp_std=c++20'],  # C++ version
        meson_version: '>=1.1.0')
build_dir = meson.current_build_dir()
links = meson.get_external_property('links', [])
compile_args = meson.get_external_property('compile_args', [])

# Check if we are building natively or for emscripten
is_native = (links == [])

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
cli11 = dependency('CLI11', required: is_native)
json = dependency('nlohmann_json')
sdl2 = dependency('sdl2', required: false)
sdl2image = dependency('sdl2_image', required: false)
thread_dep = dependency('threads')  # Check: is this required for emscripten?
zlib = dependency('zlib')

# Dependency groups
header_deps = [glm, json]
core_deps = [curl, fftw, gemmi, thread_dep, zlib]
graphics_deps = []
cli_deps = [header_deps, core_deps, cli11]

if is_native
  graphics_deps = [gl, glew, sdl2, sdl2image]
endif

gui_deps = [header_deps, core_deps, graphics_deps]

# Windows wiring
if host_machine.system() == 'windows'
  compile_args += ['-DNOMINMAX']
endif

# Test-mode wiring
if is_native and get_option('enable_tests')
  doctest_dep = dependency('doctest', required: true)
  compile_args += ['-DROPE_INLINE_TESTS']
  cli_deps += [doctest_dep]
  gui_deps += [doctest_dep]
endif

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
subdir('vagabond/cli')
subdir('vagabond/gui/elements')
subdir('vagabond/gui')

# Create executables
if is_native  # Native builds

  if get_option('enable_tests')
    subdir('tests')
  endif

  # Additional subdirectories for native builds
  subdir('vagabond/untangle')

  executable('rope.gui', 'vagabond/gui/main.cpp',
    link_with: [gui, core],
    cpp_args: [compile_args],
    link_args: [links],
    dependencies: gui_deps,
    install: true)

  executable('rope', 'vagabond/cmd/main.cpp',
    link_with: [core, cli],
    cpp_args: [compile_args],
    link_args: [links],
    dependencies: cli_deps,
    install: true)

  executable('rope.cli', 'vagabond/cli/main.cpp',
    link_with: [core, cli2],
    cpp_args: [compile_args],
    link_args: [links],
    dependencies: cli_deps,
    install: true)

  executable('rope.gui.test',
    ['vagabond/gui/tests/main.cpp',
     'vagabond/gui/tests/CheckList.cpp',
     'vagabond/gui/tests/CheckList.h'],
    link_with: [gui, core],
    cpp_args: [compile_args],
    link_args: [links],
    dependencies: gui_deps,
    install: true)

else  # emscripten
  extrafiles = ['vagabond/gui/print_file.cpp']
  start_cpp = 'vagabond/gui/main.cpp'
  compile_args += ['-DNDEBUG', '-DBOOST_UBLAS_NDEBUG']
  message(links)
  executable('index.html', extrafiles, start_cpp,
    link_with: [core, gui],
    cpp_args: [compile_args],
    link_args: [links],
    dependencies: header_deps,
    install: true)
endif
