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
'src/BadChoice.cpp',
'src/Box.cpp',
'src/Button.cpp',
'src/Choice.cpp',
'src/ChoiceGroup.cpp',
'src/Draggable.cpp',
'src/FileReader.cpp',
'src/HasRenderables.cpp',
'src/Image.cpp',
'src/Library.cpp',
'src/MainMenu.cpp',
'src/Modal.cpp',
'src/Renderable.cpp',
'src/Scene.cpp',
'src/Slider.cpp',
'src/SnowGL.cpp',
'src/Text.cpp',
'src/TextManager.cpp',
'src/Window.cpp',
]

if (links == [])
  vag = static_library('libvag', vagafiles, 
  cpp_args: ['-I/usr/local/include'],
  link_args: ['-L/usr/local/lib'],
  dependencies : deps, install: true)

  executable(name, 'src/main.cpp',
  cpp_args: ['-I/usr/local/include'],
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
'empty_renderable_envelope_radius_is_zero',
]

foreach entry : test_list
  t = executable(entry, 'tests' / entry + '.cpp', link_with : vag)
  test(entry, t)
endforeach

