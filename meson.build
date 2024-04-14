project('rope', 'cpp', 'c', version : '1.5')

links = meson.get_cross_property('links', [])
compile_args = meson.get_cross_property('compile_args', [])
glm = dependency('glm', required: false)
gl = dependency('glew')
curl = dependency('libcurl')
zlib = dependency('zlib')
sdl2 = dependency('SDL2', required: false)
sdl2image = dependency('SDL2_image', required: false)
thread_dep = dependency('threads')

fftw_req = false
if (links == [])
  fftw_req = true
endif

native = (links == [])
req = native ? true : false

fftw = dependency('fftw3f', required: fftw_req)
json = dependency('nlohmann_json')
gemmi = dependency('gemmi', modules: ['gemmi::gemmi_cpp'], required: req)
deps = [glm, json]
name = 'index.html'

  deps += [gl, sdl2, sdl2image, curl, thread_dep, gemmi, fftw, zlib]
  name = 'rope.gui'
endif

data_dir = get_option('datadir') / 'vagabond'
shaders = data_dir / 'assets' / 'shaders'
images = data_dir / 'assets' / 'images'
geometry = data_dir / 'assets' / 'geometry'
examples = data_dir / 'assets' / 'examples'
dir_base = get_option('prefix')
combined = join_paths(dir_base, data_dir)

install_data(
install_dir : examples)

install_data(
'assets/geometry/standard_geometry.cif',
'assets/geometry/proline.json',
install_dir : geometry)

install_data(
'assets/shaders_450/axes.vsh',
'assets/shaders_450/axes.fsh',
'assets/shaders_450/color_only.fsh',
'assets/shaders_450/quad.vsh',
'assets/shaders_450/quad.fsh',
'assets/shaders_450/box.vsh',
'assets/shaders_450/box.fsh',
'assets/shaders_450/indexed_box.fsh',
'assets/shaders_450/scrollbox.fsh',
'assets/shaders_450/shade.fsh',
'assets/shaders_450/floating_box.vsh',
'assets/shaders_450/with_matrix.vsh',
'assets/shaders_450/density.vsh',
'assets/shaders_450/density.fsh',
'assets/shaders_450/multiply.fsh',
'assets/shaders_450/pencil_shading.fsh',
'assets/shaders_450/progress.fsh',
'assets/shaders_450/point.vsh',
'assets/shaders_450/point.fsh',
'assets/shaders_450/simple_point.fsh',
'assets/shaders_450/simple_point.vsh',
'assets/shaders_450/lighting.fsh',
'assets/shaders_450/reflections.fsh',
install_dir : shaders)

install_data(
'assets/images/arrow.png',
'assets/images/axes.png',
'assets/images/axis.png',
'assets/images/bee.png',
'assets/images/big_button.png',
'assets/images/black.png',
'assets/images/blank.png',
'assets/images/blob.png',
'assets/images/bond.png',
'assets/images/box.png',
'assets/images/cartoon.png',
'assets/images/colour_scheme.png',
'assets/images/confspace.png',
'assets/images/cross.png',
'assets/images/data_model.png',
'assets/images/density.png',
'assets/images/eye.png',
'assets/images/dot.png',
'assets/images/gel.png',
'assets/images/gel_top.png',
'assets/images/files.png',
'assets/images/happy_face.png',
'assets/images/lemon.png',
'assets/images/little_line.png',
'assets/images/line.png',
'assets/images/map.png',
'assets/images/metadata.png',
'assets/images/misc_data.png',
'assets/images/model.png',
'assets/images/ok_face.png',
'assets/images/orange.png',
'assets/images/palette.png',
'assets/images/paper.jpg',
'assets/images/peak.png',
'assets/images/pencil_shading.png',
'assets/images/phenylalanine.png',
'assets/images/points.png',
'assets/images/present_bond.png',
'assets/images/protein.png',
'assets/images/protractor.png',
'assets/images/reflections.png',
'assets/images/refresh.png',
'assets/images/rope.png',
'assets/images/ruler.png',
'assets/images/sad_face.png',
'assets/images/search.png',
'assets/images/sequence.png',
'assets/images/some_atoms.png',
'assets/images/tick.png',
'assets/images/tickbox_empty.png',
'assets/images/tickbox_filled.png',
'assets/images/transparency.png',
'assets/images/triangle.png',
'assets/images/tools.png',
'assets/images/torsion.png',
'assets/images/tube_1.5ml.png',
'assets/images/unit_cell.png',
'assets/images/us28.png',
'assets/images/strong_bond.png',
'assets/images/weak_bond.png',
'assets/images/unassigned_bond.png',
install_dir : images)

subdir('config')
#add_data = find_program('get_config.sh')

#out_path = 'config.h'
#gen_src = custom_target('config-output',
#                        input : ['config.h.in'],
#                        output : [out_path],
#                        command : [add_data, '@INPUT@', '@OUTPUT@',
#						combined],
#                       build_by_default: true,
#                        build_always_stale: false)

compile_args += ['-DNDEBUG', '-DBOOST_UBLAS_NDEBUG']

subdir('vagabond/utils')
subdir('vagabond/c4x')
subdir('vagabond/core')
subdir('vagabond/cmd')

subdir('vagabond/gui/elements')
subdir('vagabond/gui')

extrafiles = []

if (links != [])
extrafiles += 'vagabond/gui/print_file.cpp'
endif

if (links == []) # not emscripten
  executable(name, 'vagabond/gui/main.cpp',
  link_with : [gui, core],
  cpp_args: ['-std=c++11'],
  link_args: [links],
  dependencies : deps, 
  install: true)

  executable('rope', 'vagabond/cmd/main.cpp',
    link_with : [core, cli],
    cpp_args: ['-std=c++11'],
    link_args: [links],
    dependencies : [fftw, json, gemmi, thread_dep, curl],
    install: true)

  executable('rope.gui.test',
  ['vagabond/gui/tests/main.cpp',
   'vagabond/gui/tests/CheckList.cpp',
   'vagabond/gui/tests/CheckList.h'],
  link_with : [gui, core],
  cpp_args: ['-std=c++11'],
  link_args: [links],
  dependencies : deps, 
  install: true)
else # emscripten
  start_cpp = 'vagabond/gui/main.cpp'
  executable(name, extrafiles, start_cpp,
  link_with: [core, gui],
  cpp_args: ['-I/usr/local/include', '-std=c++11', compile_args],
  link_args: [links],
  dependencies : deps, install: true)
endif

if (links == [])
  subdir('vagabond/untangle')
  subdir('vagabond/c4x/tests')
  subdir('vagabond/utils/tests')
  subdir('vagabond/core/tests')
  subdir('vagabond/gui/tests')
  subdir('vagabond/gui/elements/tests')
endif

