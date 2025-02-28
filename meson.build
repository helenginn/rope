project('rope', 'cpp', 'c', version : '0.1.7')
build_dir = meson.current_build_dir()
links = meson.get_cross_property('links', [])
compile_args = meson.get_cross_property('compile_args', [])

# Check if we are building natively or for emscripten
is_native = (links == [])

# Debugging
message('Building RoPE version', meson.project_version(), 'in', build_dir)
message('  PREFIX:', get_option('prefix'))
message('  NATIVE:', is_native)

# Dependencies
curl = dependency('libcurl')
fftw = dependency('fftw3f', required: is_native)
gemmi = dependency('gemmi', modules: ['gemmi::gemmi_cpp'], required: is_native)
gl = dependency('glew')
glm = dependency('glm', required: false)
json = dependency('nlohmann_json')
sdl2 = dependency('SDL2', required: false)
sdl2image = dependency('SDL2_image', required: false)
thread_dep = dependency('threads')
zlib = dependency('zlib')

# Paths
data_dir = build_dir / 'vagabond'
assets_dir = data_dir / 'assets'
examples = data_dir / 'assets' / 'examples'
combined = join_paths(build_dir, data_dir)

# Install assets
install_data(install_dir: examples)  # What does this install?
install_subdir('assets/geometry', install_dir: assets_dir)
install_subdir('assets/shaders_450', install_dir: assets_dir / 'shaders', strip_directory: true)
install_subdir('assets/images', install_dir: assets_dir)

# Execute meson.build files in subdirectories
deps = [glm, json]
subdir('config')
#add_data = find_program('get_config.sh')
#out_path = 'config.h'
#gen_src = custom_target('config-output',
#                        input : ['config.h.in'],
#                        output : [out_path],
#                        command : [add_data, '@INPUT@', '@OUTPUT@',
#						            combined],
#                        build_by_default: true,
#                        build_always_stale: false)
subdir('vagabond/utils')
subdir('vagabond/c4x')
subdir('vagabond/core')
subdir('vagabond/cmd')
subdir('vagabond/gui/elements')
subdir('vagabond/gui')

# Create executables
if is_native  # Native builds
  deps += [gl, sdl2, sdl2image, curl, thread_dep, gemmi, fftw, zlib]

  # Additional subdirectories for native builds
  subdir('vagabond/untangle')
  subdir('vagabond/c4x/tests')
  subdir('vagabond/utils/tests')
  subdir('vagabond/core/tests')
  subdir('vagabond/gui/tests')
  subdir('vagabond/gui/elements/tests')

  executable('rope.gui', 'vagabond/gui/main.cpp',
    link_with: [gui, core],
    cpp_args: ['-std=c++11'],
    link_args: [links],
    dependencies: deps,
    install: true)

  executable('rope', 'vagabond/cmd/main.cpp',
    link_with: [core, cli],
    cpp_args: ['-std=c++14'],
    link_args: [links],
    dependencies: [fftw, json, gemmi, thread_dep, curl],
    install: true)

  executable('rope.gui.test',
    ['vagabond/gui/tests/main.cpp',
     'vagabond/gui/tests/CheckList.cpp',
     'vagabond/gui/tests/CheckList.h'],
    link_with: [gui, core],
    cpp_args: ['-std=c++11'],
    link_args: [links],
    dependencies: deps,
    install: true)

else  # emscripten
  extrafiles = ['vagabond/gui/print_file.cpp']
  start_cpp = 'vagabond/gui/main.cpp'
  compile_args += ['-DNDEBUG', '-DBOOST_UBLAS_NDEBUG']
  executable('index.html', extrafiles, start_cpp,
    link_with: [core, gui],
    cpp_args: ['-I/usr/local/include', '-std=c++11', compile_args],
    link_args: [links],
    dependencies: deps,
    install: true)
endif

