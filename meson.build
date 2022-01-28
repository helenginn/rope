project('vagabond', 'cpp')

links = meson.get_cross_property('links', [])
compile_args = meson.get_cross_property('compile_args', [])
glm = dependency('glm', required: false)
deps = [glm]
gl = dependency('GL', method : 'system', required: false)
sdl2 = dependency('SDL2', required: false)
sdl2image = dependency('SDL2_image', required: false)
name = 'index.html'

if (links == [])
  deps += [gl, sdl2, sdl2image]
  name = 'vagabond'
endif

data_dir = get_option('datadir') / 'vagabond'
shaders = data_dir / 'assets' / 'shaders'
images = data_dir / 'assets' / 'images'
geometry = data_dir / 'assets' / 'geometry'
dir_base = get_option('prefix')
combined = join_paths(dir_base, data_dir)


install_data(
'assets/geometry/standard_geometry.cif',
'assets/geometry/D-ASP.cif',
'assets/geometry/ACY.cif',
'assets/geometry/ASP.cif',
'assets/geometry/CHX.cif',
'assets/geometry/PO4.cif',
'assets/geometry/CMO.cif',
'assets/geometry/H2S.cif',
'assets/geometry/HEX.cif',
'assets/geometry/IPA.cif',
'assets/geometry/GLY.cif',
'assets/geometry/L2P.cif',
'assets/geometry/L86.cif',
'assets/geometry/LSD.cif',
'assets/geometry/TYR.cif',
'assets/geometry/NH3.cif',
install_dir : geometry)

install_data(
'assets/shaders_450/quad.vsh',
'assets/shaders_450/quad.fsh',
'assets/shaders_450/box.vsh',
'assets/shaders_450/box.fsh',
'assets/shaders_450/shade.fsh',
'assets/shaders_450/with_matrix.vsh',
'assets/shaders_450/lighting.fsh',
install_dir : shaders)

install_data(
'assets/images/arrow.png',
'assets/images/box.png',
'assets/images/dot.png',
'assets/images/gel.png',
'assets/images/gel_top.png',
'assets/images/happy_face.png',
'assets/images/line.png',
'assets/images/ok_face.png',
'assets/images/paper.jpg',
'assets/images/peak.png',
'assets/images/sad_face.png',
'assets/images/tick.png',
'assets/images/tube_1.5ml.png',
install_dir : images)

hash_script = find_program('get_hash.sh')

gen_src = custom_target('commit-output',
                        input : ['src/commit.h.pre'],
                        output : ['commit.h'],
                        command : [hash_script, '@INPUT@', '@OUTPUT@', 
						combined],
                        build_by_default: true,
                        build_always_stale: true)

vagafiles = [
'src/AskYesNo.cpp',
'src/Atom.cpp',
'src/AtomGroup.cpp',
'src/BadChoice.cpp',
'src/BondAngle.cpp',

'src/BondCalculator.cpp',
'src/BondSequence.cpp',
'src/BondSequenceHandler.cpp',

'src/BondLength.cpp',
'src/BondTorsion.cpp',
'src/Box.cpp',
'src/Button.cpp',
'src/Chirality.cpp',
'src/Choice.cpp',
'src/ChoiceGroup.cpp',
'src/Cif2Geometry.cpp',
'src/Draggable.cpp',
'src/Display.cpp',
'src/FileManager.cpp',
'src/FileReader.cpp',
'src/FileView.cpp',
'src/GeometryTable.cpp',
'src/GuiAtom.cpp',
'src/GuiBond.cpp',
'src/HasRenderables.cpp',
'src/HasBondstraints.cpp',
'src/Knotter.cpp',
'src/Icosahedron.cpp',
'src/Image.cpp',
'src/Library.cpp',
'src/MainMenu.cpp',
'src/matrix_functions.cpp',
'src/Modal.cpp',
'src/Renderable.cpp',
'src/Scene.cpp',
'src/Slider.cpp',
'src/Superpose.cpp',
'src/SnowGL.cpp',
'src/svd/svdcmp.cpp',
'src/svd/PCA.cpp',
'src/svd/matrix.cpp',
'src/Text.cpp',
'src/TextManager.cpp',
'src/ThreadCalculatesBondSequence.cpp',
'src/ThreadExtractsBondPositions.cpp',
'src/ThreadSubmitsJobs.cpp',
'src/ThreadMiniJobForSequence.cpp',
'src/Window.cpp',
]

if (links != [])
vagafiles += 'src/print_file.cpp'
endif

vag = []

if (links == []) # not emscripten
  vag = static_library('libvag', vagafiles, 
  cpp_args: ['-I/usr/local/include', '-std=c++11'],
  link_args: ['-L/usr/local/lib'],
  dependencies : deps, install: true)

  executable(name, 'src/main.cpp',
  cpp_args: ['-I/usr/local/include', '-std=c++11'],
  link_args: ['-L/usr/local/lib', links],
  dependencies : deps, link_with : vag, install: true)
else
  executable(name, vagafiles,
  'src/main.cpp',
  cpp_args: ['-I/usr/local/include', compile_args],
  link_args: ['-L/usr/local/lib', links],
  dependencies : deps, install: true)
endif

fs = import('fs')
test_list = fs.read(meson.current_source_dir() / 'tests/test_list').strip().split('\n')

if (links == [])
foreach entry : test_list
  t = executable(entry, 'tests' / entry + '.cpp', link_with : vag, 
	  cpp_args : ['-std=c++11'])
  test(entry, t)
endforeach
endif
