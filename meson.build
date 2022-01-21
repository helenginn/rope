project('vagabond', 'cpp')

links = meson.get_cross_property('links', [])
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
dir_base = get_option('prefix')
combined = join_paths(dir_base, data_dir)

install_data(
'assets/shaders_450/quad.vsh',
'assets/shaders_450/quad.fsh',
'assets/shaders_450/box.vsh',
'assets/shaders_450/box.fsh',
'assets/shaders_450/shade.fsh',
'assets/shaders_450/with_matrix.vsh',
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
'src/BadChoice.cpp',
'src/Box.cpp',
'src/Button.cpp',
'src/Choice.cpp',
'src/ChoiceGroup.cpp',
'src/Draggable.cpp',
'src/Display.cpp',
'src/FileReader.cpp',
'src/GuiAtom.cpp',
'src/HasRenderables.cpp',
'src/Icosahedron.cpp',
'src/Image.cpp',
'src/Library.cpp',
'src/MainMenu.cpp',
'src/matrix_functions.cpp',
'src/Modal.cpp',
'src/Renderable.cpp',
'src/Scene.cpp',
'src/Slider.cpp',
'src/SnowGL.cpp',
'src/Text.cpp',
'src/TextManager.cpp',
'src/Window.cpp',
]

vag = []

if (links == [])
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
  cpp_args: ['-I/usr/local/include'],
  link_args: ['-L/usr/local/lib', links],
  dependencies : deps, install: true)
endif

test_list = [
'base_filename_shortens_extension', 
'base_filename_without_path_shortens_extension',
'split_by_comma_leaves_dangling_strings',
'split_by_comma_starts_with_dangling_string',

'button_without_sender_does_nothing',
'button_highlights_when_mouse_over',
'inert_button_does_nothing_when_mouse_over',

'renderable_envelope_radius_is_most_maximal',
'renderable_returns_vertex_count_and_size',
'renderable_does_not_add_same_object_twice',
'empty_renderable_envelope_radius_is_zero',

'right_hand_matrix_has_positive_determinant',

'box_does_not_use_projection',
'atom_can_use_projection',
'atom_increments_vertex_array_by_number',
'atom_vertices_per_atom_above_zero',
'atom_does_not_accept_anumeric_positions',
'atom_centroid_is_at_added_position',
'atom_derived_should_immediately_match_initial_placement',
'atom_element_throws_exception_if_null',
'atom_element_throws_exception_if_number',
'atom_element_converts_to_upper_case',
'atom_name_converts_to_upper_case',
'atom_number_cannot_be_negative',
'atom_initialised_and_pushed_to_gui_is_renderable',
'atom_initialised_with_nitrogen_is_blue',
'atom_initialised_with_oxygen_is_red',
'atom_initialised_with_sulphur_is_yellow',
]

should_fail_list = [
]

if (links == [])
foreach entry : test_list
  t = executable(entry, 'tests' / entry + '.cpp', link_with : vag)
  test(entry, t)
endforeach

foreach entry : should_fail_list
  t = executable(entry, 'tests' / entry + '.cpp', link_with : vag)
  test(entry, t, should_fail : true)
endforeach
endif
