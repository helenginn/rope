project('vagabond', 'cpp', 'c')

links = meson.get_cross_property('links', [])
compile_args = meson.get_cross_property('compile_args', [])
glm = dependency('glm', required: false)
gl = dependency('GL', method : 'system', required: false)
sdl2 = dependency('SDL2', required: false)
sdl2image = dependency('SDL2_image', required: false)

fftw_static = true
if (links == [])
  fftw_static = false
endif

fftw = dependency('fftw3f', static: fftw_static)
deps = [glm, fftw]
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
'assets/examples/4cvd.cif',
'assets/examples/4cvd.pdb',
'assets/examples/6qhu-sf.cif',
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
'assets/shaders_450/density.vsh',
'assets/shaders_450/density.fsh',
'assets/shaders_450/lighting.fsh',
'assets/shaders_450/reflections.fsh',
install_dir : shaders)

install_data(
'assets/images/arrow.png',
'assets/images/box.png',
'assets/images/cartoon.png',
'assets/images/data_model.png',
'assets/images/density.png',
'assets/images/dot.png',
'assets/images/gel.png',
'assets/images/gel_top.png',
'assets/images/files.png',
'assets/images/happy_face.png',
'assets/images/lemon.png',
'assets/images/line.png',
'assets/images/model.png',
'assets/images/ok_face.png',
'assets/images/orange.png',
'assets/images/paper.jpg',
'assets/images/peak.png',
'assets/images/protein.png',
'assets/images/protractor.png',
'assets/images/reflections.png',
'assets/images/sad_face.png',
'assets/images/some_atoms.png',
'assets/images/tick.png',
'assets/images/tube_1.5ml.png',
'assets/images/unit_cell.png',
install_dir : images)

subdir('ccp4')
deps += dep_ccp4

hash_script = find_program('get_hash.sh')

out_path = 'commit.h'
gen_src = custom_target('commit-output',
                        input : ['vagabond/commit.h.pre'],
                        output : [out_path],
                        command : [hash_script, '@INPUT@', '@OUTPUT@', 
						combined],
                        build_by_default: true,
                        build_always_stale: true)

subdir('vagabond/gui/elements')

guifiles = [
'vagabond/gui/AddModel.cpp',
'vagabond/gui/DatasetMenu.cpp',
'vagabond/gui/Display.cpp',
'vagabond/gui/EntityMenu.cpp',
'vagabond/gui/FileLine.cpp',
'vagabond/gui/FileView.cpp',
'vagabond/gui/GuiAtom.cpp',
'vagabond/gui/GuiBond.cpp',
'vagabond/gui/GuiDensity.cpp',
'vagabond/gui/GuiRefls.cpp',
'vagabond/gui/MainMenu.cpp',
'vagabond/gui/ModelMenu.cpp',
'vagabond/gui/VagWindow.cpp',
'vagabond/cmd/CmdWorker.cpp',
'vagabond/cmd/Dictator.cpp',
]

vagafiles = [
'vagabond/core/AlignmentTool.cpp',
'vagabond/core/Atom.cpp',
'vagabond/core/AtomContent.cpp',
'vagabond/core/AtomGroup.cpp',
'vagabond/core/AtomGraph.cpp',
'vagabond/core/AtomSegment.cpp',
'vagabond/core/AtomMap.cpp',
'vagabond/core/BondAngle.cpp',
'vagabond/core/BondCalculator.cpp',
'vagabond/core/BondSequence.cpp',
'vagabond/core/BondSequenceHandler.cpp',
'vagabond/core/BondLength.cpp',
'vagabond/core/BondTorsion.cpp',
'vagabond/core/Chain.cpp',
'vagabond/core/Chirality.cpp',
'vagabond/core/CifFile.cpp',
'vagabond/core/ConcertedBasis.cpp',
'vagabond/core/Diffraction.cpp',
'vagabond/core/ElementLibrary.cpp',
'vagabond/core/ElementSegment.cpp',
'vagabond/core/Environment.cpp',
'vagabond/core/File.cpp',
'vagabond/core/FileManager.cpp',
'vagabond/core/GeometryTable.cpp',
'vagabond/core/Grapher.cpp',
'vagabond/core/HasBondstraints.cpp',
'vagabond/core/Knotter.cpp',
'vagabond/core/MapTransferHandler.cpp',
'vagabond/core/MapSumHandler.cpp',
'vagabond/core/matrix_functions.cpp',
'vagabond/core/Model.cpp',
'vagabond/core/ModelManager.cpp',
'vagabond/core/PdbFile.cpp',
'vagabond/core/PointStore.cpp',
'vagabond/core/PointStoreHandler.cpp',
'vagabond/core/PositionRefinery.cpp',
'vagabond/core/RefList.cpp',
'vagabond/core/Residue.cpp',
'vagabond/core/Sampler.cpp',
'vagabond/core/Sequence.cpp',
'vagabond/core/SimpleBasis.cpp',
'vagabond/core/SimplexEngine.cpp',
'vagabond/core/Superpose.cpp',
'vagabond/core/ThreadCalculatesBondSequence.cpp',
'vagabond/core/ThreadExtractsBondPositions.cpp',
'vagabond/core/ThreadSubmitsJobs.cpp',
'vagabond/core/ThreadMapTransfer.cpp',
'vagabond/core/ThreadMapSummer.cpp',
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
'vagabond/core/AtomSegment.h',
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
  dependencies : deps, link_with : [vag, guielements], install: true)

  executable(name, 'vagabond/gui/main.cpp',
  cpp_args: ['-I/usr/local/include', '-std=c++11'],
  link_args: ['-L/usr/local/lib', links],
  dependencies : deps, link_with : gui, install: true)
else # emscripten
  executable(name, vagafiles, guielementfiles, guifiles,
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
