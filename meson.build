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
  name = 'vagabond.gui'
endif

data_dir = get_option('datadir') / 'vagabond'
shaders = data_dir / 'assets' / 'shaders'
images = data_dir / 'assets' / 'images'
geometry = data_dir / 'assets' / 'geometry'
examples = data_dir / 'assets' / 'examples'
dir_base = get_option('prefix')
combined = join_paths(dir_base, data_dir)


install_data(
'assets/examples/2YBH.cif',
'assets/examples/6QHU.cif',
install_dir : examples)

install_data(
'assets/geometry/standard_geometry.cif',
'assets/geometry/D-ASP.cif',
'assets/geometry/ACY.cif',
'assets/geometry/ATP.cif',
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
'assets/geometry/TCI.cif',
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
                        input : ['vagabond/commit.h.pre'],
                        output : ['commit.h'],
                        command : [hash_script, '@INPUT@', '@OUTPUT@', 
						combined],
                        build_by_default: true,
                        build_always_stale: true)

guifiles = [
'vagabond/gui/AskYesNo.cpp',
'vagabond/gui/BadChoice.cpp',
'vagabond/gui/Box.cpp',
'vagabond/gui/Button.cpp',
'vagabond/gui/Choice.cpp',
'vagabond/gui/ChoiceGroup.cpp',
'vagabond/gui/Draggable.cpp',
'vagabond/gui/Display.cpp',
'vagabond/gui/FileManager.cpp',
'vagabond/gui/FileView.cpp',
'vagabond/gui/GuiAtom.cpp',
'vagabond/gui/GuiBond.cpp',
'vagabond/gui/HasRenderables.cpp',
'vagabond/gui/Icosahedron.cpp',
'vagabond/gui/Image.cpp',
'vagabond/gui/Library.cpp',
'vagabond/gui/MainMenu.cpp',
'vagabond/gui/Modal.cpp',
'vagabond/gui/Slider.cpp',
'vagabond/gui/Renderable.cpp',
'vagabond/gui/Scene.cpp',
'vagabond/gui/SnowGL.cpp',
'vagabond/gui/Text.cpp',
'vagabond/gui/TextManager.cpp',
'vagabond/gui/Window.cpp',
]

vagafiles = [
'vagabond/core/AlignmentTool.cpp',
'vagabond/core/Atom.cpp',
'vagabond/core/AtomGroup.cpp',
'vagabond/core/AtomGraph.cpp',
'vagabond/core/BondAngle.cpp',
'vagabond/core/BondCalculator.cpp',
'vagabond/core/BondSequence.cpp',
'vagabond/core/BondSequenceHandler.cpp',
'vagabond/core/BondLength.cpp',
'vagabond/core/BondTorsion.cpp',
'vagabond/core/Chirality.cpp',
'vagabond/core/CifFile.cpp',
'vagabond/core/ConcertedBasis.cpp',
'vagabond/core/GeometryTable.cpp',
'vagabond/core/HasBondstraints.cpp',
'vagabond/core/Knotter.cpp',
'vagabond/core/matrix_functions.cpp',
'vagabond/core/PositionRefinery.cpp',
'vagabond/core/RefList.cpp',
'vagabond/core/SimpleBasis.cpp',
'vagabond/core/SimplexEngine.cpp',
'vagabond/core/Superpose.cpp',
'vagabond/core/ThreadCalculatesBondSequence.cpp',
'vagabond/core/ThreadExtractsBondPositions.cpp',
'vagabond/core/ThreadSubmitsJobs.cpp',
'vagabond/core/ThreadMiniJobForSequence.cpp',
'vagabond/core/TorsionBasis.cpp',
'vagabond/utils/FileReader.cpp',
'vagabond/utils/svd/svdcmp.cpp',
'vagabond/utils/svd/PCA.cpp',
'vagabond/utils/svd/matrix.cpp',
]

install_headers([
'vagabond/utils/FileReader.h',
'vagabond/utils/glm_import.h',
],
subdir : 'vagabond/utils')

install_headers([
'vagabond/core/AlignmentTool.h',
'vagabond/core/Atom.h',
'vagabond/core/AtomGroup.h',
'vagabond/core/BondAngle.h',
'vagabond/core/BondCalculator.h',
'vagabond/core/BondLength.h',
'vagabond/core/BondSequence.h',
'vagabond/core/BondSequenceHandler.h',
'vagabond/core/BondTorsion.h',
'vagabond/core/Bondstraint.h',
'vagabond/core/Chirality.h',
'vagabond/core/CifFile.h',
'vagabond/core/ConcertedBasis.h',
'vagabond/core/GeometryTable.h',
'vagabond/core/Handler.h',
'vagabond/core/HasBondstraints.h',
'vagabond/core/Knotter.h',
'vagabond/core/PositionRefinery.h',
'vagabond/core/RefList.h',
'vagabond/core/Semaphore.h',
'vagabond/core/SimpleBasis.h',
'vagabond/core/SimplexEngine.h',
'vagabond/core/Superpose.h',
'vagabond/core/ThreadCalculatesBondSequence.h',
'vagabond/core/ThreadExtractsBondPositions.h',
'vagabond/core/ThreadMiniJobForSequence.h',
'vagabond/core/ThreadSubmitsJobs.h',
'vagabond/core/ThreadWorker.h',
'vagabond/core/TorsionBasis.h',
'vagabond/core/matrix_functions.h',
],
subdir : 'vagabond/core')


if (links != [])
vagafiles += 'vagabond/gui/print_file.cpp'
endif

vag = []

if (links == []) # not emscripten
  vag = static_library('vag', vagafiles, 
  cpp_args: ['-I/usr/local/include', '-std=c++11'],
  link_args: ['-L/usr/local/lib'],
  dependencies : deps, install: true)

  gui = static_library('vaggui', guifiles,
  cpp_args: ['-I/usr/local/include', '-std=c++11'],
  link_args: ['-L/usr/local/lib', links],
  dependencies : deps, link_with : vag, install: true)

  executable(name, 'vagabond/gui/main.cpp',
  cpp_args: ['-I/usr/local/include', '-std=c++11'],
  link_args: ['-L/usr/local/lib', links],
  dependencies : deps, link_with : gui, install: true)
else
  executable(name, vagafiles, guifiles,
  'vagabond/gui/main.cpp',
  cpp_args: ['-I/usr/local/include', compile_args],
  link_args: ['-L/usr/local/lib', links],
  dependencies : deps, install: true)
endif

if (links == [])
  subdir('vagabond/core/tests')
  subdir('vagabond/utils/tests')
  subdir('vagabond/gui/tests')
endif
